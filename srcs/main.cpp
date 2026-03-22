
#include "glad.h"
#include <print>
#include <memory>
import mka.graphic.window;

class MyApp : public mka::graphic::Window {
	public:	

		MyApp(std::unique_ptr<mka::graphic::Context> ctx) :
			mka::graphic::Window(800, 200, "Hello Window", std::move(ctx)) {
	
	}

	void render() {
		glClearColor(1.0f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);	
	}
};

int main() {

	auto ctx = mka::graphic::createContext(
			mka::graphic::API::OpenGL, 
			mka::graphic::Loader::Glad
	);
	MyApp w(std::move(ctx));
			
	w.run();

	return 0;
}
