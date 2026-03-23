module;

#include "glad.h"
#include <string>

export module mka.graphic.opengl.shader;
export import mka.graphic.opengl.uniform;

export namespace mka::graphic::gl {

	enum class ShaderType : GLenum {
		Vertex = GL_VERTEX_SHADER,
		Fragment = GL_FRAGMENT_SHADER,
		Geometry = GL_GEOMETRY_SHADER
	};

    class Shader {
    public:

        Shader() {
			program = glCreateProgram();
		}
		
		~Shader() {
            if (program != 0) {
                glDeleteProgram(program);
            }
        }

        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;
        Shader(Shader&&) = delete;
        Shader& operator=(Shader&&) = delete;
        
		void use() const {
			glUseProgram(program);
		}

        void set(const std::string &uniform_variable, const auto value) const {
            const GLint location = glGetUniformLocation(program, uniform_variable.c_str());
            mka::graphic::gl::glUniform(program, location, value);
        }

        std::string addScript(const std::string &shaderCode, ShaderType type) const {
			const GLuint shaderID = glCreateShader(static_cast<GLenum>(type));
			const char* source = shaderCode.c_str();

			glShaderSource(shaderID, 1, &source, nullptr);
			glCompileShader(shaderID);
			
			GLint success = 0;
			glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
	
			std::string log = "";
			
			if (!success) {
				GLint logLength = 0;
				glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &logLength);

				if(logLength > 0) {
					log.resize(logLength);
					GLsizei written = 0;
					glGetShaderInfoLog(shaderID, logLength, &written, log.data());
					log.resize(written);
				}
			}

			glAttachShader(program, shaderID);
			glDeleteShader(shaderID);
			
			return log;
		}
		
		std::string link() const {
            glLinkProgram(program);

            GLint success = 0;
            glGetProgramiv(program, GL_LINK_STATUS, &success);

            std::string log = "";
            if (!success) {
                GLint logLength = 0;
                glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

                if (logLength > 0) {
                    log.resize(logLength);
                    GLsizei written = 0;
                    glGetProgramInfoLog(program, logLength, &written, log.data());
                    log.resize(written);
                }
            }

            return log;
        }
    private:
        GLuint program = 0;
    };
} // mka::graphic
