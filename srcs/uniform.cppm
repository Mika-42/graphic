/**
 * @file uniform.cppm
 * @brief C++20 module implementation for `uniform`.
 */
module;

#include <vector>
#include <string>
#include "glad.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

export module mka.graphic.opengl.uniform;

namespace mka::graphic::gl {

    /// @brief Primary trait specialized for each supported uniform type.
    // =========================
    // Base trait
    // =========================
    template<typename T>
    struct Uniform;

    // =========================
    // Scalars
    // =========================
    template<>
    struct Uniform<float> {
        static void set(GLuint p, GLint l, float v) {
            glProgramUniform1f(p, l, v);
        }
    };

    template<>
    struct Uniform<int> {
        static void set(GLuint p, GLint l, int v) {
            glProgramUniform1i(p, l, v);
        }
    };

    template<>
    struct Uniform<unsigned int> {
        static void set(GLuint p, GLint l, unsigned int v) {
            glProgramUniform1ui(p, l, v);
        }
    };

    template<>
    struct Uniform<bool> {
        static void set(GLuint p, GLint l, bool v) {
            glProgramUniform1i(p, l, static_cast<int>(v));
        }
    };

    // =========================
    // Vectors (float)
    // =========================
    template<>
    struct Uniform<glm::vec2> {
        static void set(GLuint p, GLint l, const glm::vec2& v) {
            glProgramUniform2fv(p, l, 1, glm::value_ptr(v));
        }
    };

    template<>
    struct Uniform<glm::vec3> {
        static void set(GLuint p, GLint l, const glm::vec3& v) {
            glProgramUniform3fv(p, l, 1, glm::value_ptr(v));
        }
    };

    template<>
    struct Uniform<glm::vec4> {
        static void set(GLuint p, GLint l, const glm::vec4& v) {
            glProgramUniform4fv(p, l, 1, glm::value_ptr(v));
        }
    };

    // =========================
    // Vectors (int)
    // =========================
    template<>
    struct Uniform<glm::ivec2> {
        static void set(GLuint p, GLint l, const glm::ivec2& v) {
            glProgramUniform2iv(p, l, 1, glm::value_ptr(v));
        }
    };

    template<>
    struct Uniform<glm::ivec3> {
        static void set(GLuint p, GLint l, const glm::ivec3& v) {
            glProgramUniform3iv(p, l, 1, glm::value_ptr(v));
        }
    };

    template<>
    struct Uniform<glm::ivec4> {
        static void set(GLuint p, GLint l, const glm::ivec4& v) {
            glProgramUniform4iv(p, l, 1, glm::value_ptr(v));
        }
    };

    // =========================
    // Vectors (uint)
    // =========================
    template<>
    struct Uniform<glm::uvec2> {
        static void set(GLuint p, GLint l, const glm::uvec2& v) {
            glProgramUniform2uiv(p, l, 1, glm::value_ptr(v));
        }
    };

    template<>
    struct Uniform<glm::uvec3> {
        static void set(GLuint p, GLint l, const glm::uvec3& v) {
            glProgramUniform3uiv(p, l, 1, glm::value_ptr(v));
        }
    };

    template<>
    struct Uniform<glm::uvec4> {
        static void set(GLuint p, GLint l, const glm::uvec4& v) {
            glProgramUniform4uiv(p, l, 1, glm::value_ptr(v));
        }
    };

    // =========================
    // Matrices (float)
    // =========================
    template<>
    struct Uniform<glm::mat2> {
        static void set(GLuint p, GLint l, const glm::mat2& v) {
            glProgramUniformMatrix2fv(p, l, 1, GL_FALSE, glm::value_ptr(v));
        }
    };

    template<>
    struct Uniform<glm::mat3> {
        static void set(GLuint p, GLint l, const glm::mat3& v) {
            glProgramUniformMatrix3fv(p, l, 1, GL_FALSE, glm::value_ptr(v));
        }
    };

    template<>
    struct Uniform<glm::mat4> {
        static void set(GLuint p, GLint l, const glm::mat4& v) {
            glProgramUniformMatrix4fv(p, l, 1, GL_FALSE, glm::value_ptr(v));
        }
    };

    // =========================
    // Arrays (std::vector)
    // =========================
    template<>
    struct Uniform<std::vector<float>> {
        static void set(GLuint p, GLint l, const std::vector<float>& v) {
            if (!v.empty())
                glProgramUniform1fv(p, l, static_cast<GLsizei>(v.size()), v.data());
        }
    };

    template<>
    struct Uniform<std::vector<glm::vec3>> {
        static void set(GLuint p, GLint l, const std::vector<glm::vec3>& v) {
            if (!v.empty())
                glProgramUniform3fv(p, l, static_cast<GLsizei>(v.size()), glm::value_ptr(v[0]));
        }
    };

    template<>
    struct Uniform<std::vector<glm::vec4>> {
        static void set(GLuint p, GLint l, const std::vector<glm::vec4>& v) {
            if (!v.empty())
                glProgramUniform4fv(p, l, static_cast<GLsizei>(v.size()), glm::value_ptr(v[0]));
        }
    };

    // =========================
    // Generic entry point
    // =========================
    /**
     * @brief Generic entry point used by `Shader::set`.
     *
     * Compilation fails if no `Uniform<T>` specialization exists, making
     * unsupported uniform types explicit at compile-time.
     */
    export template<typename T>
    void glUniform(GLuint program, GLint location, const T& value) {
        Uniform<T>::set(program, location, value);
    }
}
