
// my_app.cpp
#include <Mahi/Gui.hpp>
#include <Mahi/Util.hpp>

// Inherit from Application
class MyApp : public mahi::gui::Application {
public:
	// 640x480 px window
	MyApp() : Application(640,480,"My App") { }
	// Override update (called once per frame)
	void update() override {
		// App logic and/or ImGui code goes here
		ImGui::Begin("Example");
		if (ImGui::Button("Press Me!"))
			mahi::util::print("Hello, World!");
		ImGui::End();
	}
};

int main(int /*argc*/, char** /*argv*/) {
	MyApp app;
	app.run();
	return 0;
}
