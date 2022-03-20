
// my_app.cpp
#include <Mahi/Gui.hpp>
#include <Mahi/Util.hpp>

#include "Emulator/Chip8.h"

// keyboard mapping
static inline emu::Keys::Enum GetKeyPressed()
{
	if (ImGui::IsKeyPressed(GLFW_KEY_1))
		return emu::Keys::One;
	if (ImGui::IsKeyPressed(GLFW_KEY_2))
		return emu::Keys::Two;
	if (ImGui::IsKeyPressed(GLFW_KEY_3))
		return emu::Keys::Three;
	if (ImGui::IsKeyPressed(GLFW_KEY_4))
		return emu::Keys::C;

	if (ImGui::IsKeyPressed(GLFW_KEY_Q))
		return emu::Keys::Four;
	if (ImGui::IsKeyPressed(GLFW_KEY_W))
		return emu::Keys::Five;
	if (ImGui::IsKeyPressed(GLFW_KEY_E))
		return emu::Keys::Six;
	if (ImGui::IsKeyPressed(GLFW_KEY_R))
		return emu::Keys::D;

	if (ImGui::IsKeyPressed(GLFW_KEY_A))
		return emu::Keys::Seven;
	if (ImGui::IsKeyPressed(GLFW_KEY_S))
		return emu::Keys::Eight;
	if (ImGui::IsKeyPressed(GLFW_KEY_D))
		return emu::Keys::Nine;
	if (ImGui::IsKeyPressed(GLFW_KEY_F))
		return emu::Keys::E;

	if (ImGui::IsKeyPressed(GLFW_KEY_Z))
		return emu::Keys::A;
	if (ImGui::IsKeyPressed(GLFW_KEY_X))
		return emu::Keys::Zero;
	if (ImGui::IsKeyPressed(GLFW_KEY_C))
		return emu::Keys::B;
	if (ImGui::IsKeyPressed(GLFW_KEY_V))
		return emu::Keys::F;

	return emu::Keys::END;
}

// Inherit from Application
class ChipEightEmulator : public mahi::gui::Application {
public:
	// 640x480 px window
	ChipEightEmulator() : Application(640,480,"ChipEightEmulator") { }

	// Override update (called once per frame)
	void update() override {
		// App logic and/or ImGui code goes here
		ImGui::Begin("Example");
		if (ImGui::Button("Press Me!"))
			mahi::util::print("Hello, World!");
		if (ImGui::IsKeyPressed(GLFW_KEY_A))
			mahi::util::print("A is pressed");
		ImGui::End();
	}
};

int main(int /*argc*/, char** /*argv*/) {
	ChipEightEmulator app;
	app.run();
	return 0;
}
