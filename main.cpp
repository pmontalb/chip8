
#include "IgnoreWarning.h"

#ifdef __clang__
	#define __IGNORE_MAHI_GUI_WARNINGS__                                                                               \
		__IGNORE_WARNING__("-Wsign-conversion")                                                                        \
		__IGNORE_WARNING__("-Wfloat-equal")                                                                            \
		__IGNORE_WARNING__("-Wold-style-cast")                                                                         \
		__IGNORE_WARNING__("-Wdouble-promotion")                                                                       \
		__IGNORE_WARNING__("-Wnewline-eof")                                                                            \
		__IGNORE_WARNING__("-Wdocumentation-unknown-command")                                                          \
		__IGNORE_WARNING__("-Wdocumentation")                                                                          \
		__IGNORE_WARNING__("-Wgnu-anonymous-struct")                                                                   \
		__IGNORE_WARNING__("-Wnested-anon-types")                                                                      \
		__IGNORE_WARNING__("-Wunused-parameter")                                                                       \
		__IGNORE_WARNING__("-Wmissing-braces")                                                                         \
		__IGNORE_WARNING__("-Wnon-virtual-dtor")                                                                       \
		__IGNORE_WARNING__("-Wundef")                                                                                  \
		__IGNORE_WARNING__("-Wimplicit-float-conversion")                                                              \
		__IGNORE_WARNING__("-Wimplicit-int-float-conversion")                                                          \
		__IGNORE_WARNING__("-Wzero-as-null-pointer-constant")                                                          \
		__IGNORE_WARNING__("-Wdeprecated-copy-dtor")                                                                   \
		__IGNORE_WARNING__("-Wextra-semi")                                                                             \
		__IGNORE_WARNING__("-Wundefined-func-template")                                                                \
		__IGNORE_WARNING__("-Wshadow-field-in-constructor")
#else
	#define __IGNORE_MAHI_GUI_WARNINGS__                                                                               \
		__IGNORE_WARNING__("-Wshadow")                                                                                 \
		__IGNORE_WARNING__("-Wfloat-equal")                                                                            \
		__IGNORE_WARNING__("-Wold-style-cast")                                                                         \
		__IGNORE_WARNING__("-Wpedantic")                                                                               \
		__IGNORE_WARNING__("-Wunused-parameter")                                                                       \
		__IGNORE_WARNING__("-Weffc++")                                                                                 \
		__IGNORE_WARNING__("-Wnon-virtual-dtor")                                                                       \
		__IGNORE_WARNING__("-Wconversion")                                                                             \
		__IGNORE_WARNING__("-Wuseless-cast")                                                                           \
		__IGNORE_WARNING__("-Wextra-semi")                                                                             \
		__IGNORE_WARNING__("-Wsuggest-override")                                                                       \
		__IGNORE_WARNING__("-Wctor-dtor-privacy")                                                                      \
		__IGNORE_WARNING__("-Wuseless-cast")                                                                           \
		__IGNORE_WARNING__("-Wdeprecated")                                                                             \
		__IGNORE_WARNING__("-Wredundant-tags")                                                                            \
		__IGNORE_WARNING__("-Wzero-as-null-pointer-constant")                                                             \
		__IGNORE_WARNING__("-Wsign-promo")                                                                         \
		__IGNORE_WARNING__("-Wdouble-promotion")
#endif

__START_IGNORING_WARNINGS__
__IGNORE_MAHI_GUI_WARNINGS__
#include <Mahi/Gui.hpp>
#include <Mahi/Gui/Native.hpp>
#include <imgui_internal.h>
__STOP_IGNORING_WARNINGS__

#define LOGGER_NAME "ringBuffer"
#include "Emulator/Logging.h"

#include "Emulator/Chip8.h"
#include "Emulator/RingBuffer.h"

#include <mutex>
#include <thread>

static const std::unordered_map<spdlog::level::level_enum, mahi::gui::Color> logColors = {
	{ spdlog::level::trace, mahi::gui::Greens::LimeGreen }, { spdlog::level::debug, mahi::gui::Greens::LightGreen },
	{ spdlog::level::info, mahi::gui::Whites::White },		{ spdlog::level::warn, mahi::gui::Yellows::Yellow },
	{ spdlog::level::err, mahi::gui::Oranges::OrangeRed },	{ spdlog::level::critical, mahi::gui::Reds::Crimson },
};

static auto RegisterRingBufferSink()
{
	auto rbSink = std::make_shared<utils::RingBufferSinkSt>();
	std::vector<spdlog::sink_ptr> sinks;
	sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_st>());
	sinks.push_back(rbSink);
	spdlog::register_logger(std::make_shared<spdlog::logger>(LOGGER_NAME, sinks.begin(), sinks.end()));

	spdlog::get(LOGGER_NAME)->set_level(spdlog::level::trace);

	return rbSink;
}

// keyboard mapping
namespace detail
{
	template<typename ImGuiOp>
	static inline std::bitset<emu::Keys::END> GetKeyPressedOrReleasedWorker(ImGuiOp&& op, bool toggle)
	{
		std::bitset<emu::Keys::END> ret;
		if (op(GLFW_KEY_1))
			ret[emu::Keys::One] = toggle;
		if (op(GLFW_KEY_2))
			ret[emu::Keys::Two] = toggle;
		if (op(GLFW_KEY_3))
			ret[emu::Keys::Three] = toggle;
		if (op(GLFW_KEY_4))
			ret[emu::Keys::C] = toggle;

		if (op(GLFW_KEY_Q))
			ret[emu::Keys::Four] = toggle;
		if (op(GLFW_KEY_W))
			ret[emu::Keys::Five] = toggle;
		if (op(GLFW_KEY_E))
			ret[emu::Keys::Six] = toggle;
		if (op(GLFW_KEY_R))
			ret[emu::Keys::D] = toggle;

		if (op(GLFW_KEY_A))
			ret[emu::Keys::Seven] = toggle;
		if (op(GLFW_KEY_S))
			ret[emu::Keys::Eight] = toggle;
		if (op(GLFW_KEY_D))
			ret[emu::Keys::Nine] = toggle;
		if (op(GLFW_KEY_F))
			ret[emu::Keys::E] = toggle;

		if (op(GLFW_KEY_Z))
			ret[emu::Keys::A] = toggle;
		if (op(GLFW_KEY_X))
			ret[emu::Keys::Zero] = toggle;
		if (op(GLFW_KEY_C))
			ret[emu::Keys::B] = toggle;
		if (op(GLFW_KEY_V))
			ret[emu::Keys::F] = toggle;

		return ret;
	}
}	 // namespace detail
static inline auto GetKeysPressed()
{
	return detail::GetKeyPressedOrReleasedWorker([](int x) { return ImGui::IsKeyPressed(x); }, true);
}
static inline auto GetKeysReleased()
{
	return detail::GetKeyPressedOrReleasedWorker([](int x) { return ImGui::IsKeyReleased(x); }, true);
}

namespace ImGui
{
	static inline void BeginGroupPanel(const char* name, const ImVec2& size = ImVec2(-1.0f, -1.0f))
	{
		ImGui::BeginGroup();

		//		auto cursorPos = ImGui::GetCursorScreenPos();
		auto itemSpacing = ImGui::GetStyle().ItemSpacing;
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

		auto frameHeight = ImGui::GetFrameHeight();
		ImGui::BeginGroup();

		ImVec2 effectiveSize = size;
		if (size.x < 0.0f)
			effectiveSize.x = ImGui::GetContentRegionAvailWidth();
		else
			effectiveSize.x = size.x;
		ImGui::Dummy(ImVec2(effectiveSize.x, 0.0f));

		ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
		ImGui::SameLine(0.0f, 0.0f);
		ImGui::BeginGroup();
		ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
		ImGui::SameLine(0.0f, 0.0f);
		ImGui::TextUnformatted(name);
		ImGui::SameLine(0.0f, 0.0f);
		ImGui::Dummy(ImVec2(0.0, frameHeight + itemSpacing.y));
		ImGui::BeginGroup();

		ImGui::PopStyleVar(2);

		ImGui::GetCurrentWindow()->ContentRegionRect.Max.x -= frameHeight * 0.5f;
		ImGui::GetCurrentWindow()->Size.x -= frameHeight;

		ImGui::PushItemWidth(effectiveSize.x - frameHeight);
	}

	static inline void EndGroupPanel()
	{
		ImGui::PopItemWidth();

		auto itemSpacing = ImGui::GetStyle().ItemSpacing;

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

		auto frameHeight = ImGui::GetFrameHeight();

		ImGui::EndGroup();

		// ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(0, 255,
		// 0, 64), 4.0f);

		ImGui::EndGroup();

		ImGui::SameLine(0.0f, 0.0f);
		ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
		ImGui::Dummy(ImVec2(0.0, frameHeight - frameHeight * 0.5f - itemSpacing.y));

		ImGui::EndGroup();

		auto itemMin = ImGui::GetItemRectMin();
		auto itemMax = ImGui::GetItemRectMax();
		// ImGui::GetWindowDrawList()->AddRectFilled(itemMin, itemMax, IM_COL32(255, 0, 0, 64), 4.0f);

		ImVec2 halfFrame = ImVec2(frameHeight * 0.25f * 0.5f, frameHeight * 0.5f);
		ImGui::GetWindowDrawList()->AddRect({ itemMin.x + halfFrame.x, itemMin.y + halfFrame.y },
											{ itemMax.x - halfFrame.x, itemMax.y },
											ImColor(ImGui::GetStyleColorVec4(ImGuiCol_Border)), halfFrame.x);

		ImGui::PopStyleVar(2);

		ImGui::GetCurrentWindow()->ContentRegionRect.Max.x += frameHeight * 0.5f;
		ImGui::GetCurrentWindow()->Size.x += frameHeight;

		ImGui::Dummy(ImVec2(0.0f, 0.0f));

		ImGui::EndGroup();
	}
}	 // namespace ImGui

__START_IGNORING_WARNINGS__
__IGNORE_WARNING__("-Wnon-virtual-dtor")
__IGNORE_WARNING__("-Wold-style-cast")
class ChipEightEmulator: public mahi::gui::Application
{
public:
	explicit ChipEightEmulator(const mahi::gui::Application::Config& config,
							   std::shared_ptr<utils::RingBufferSinkSt> sink)
		: Application(config), _sink(std::move(sink))
	{
		spdlog::set_level(logLevel);
	}
	virtual ~ChipEightEmulator() = default;

private:
	// Override update (called once per frame)
	void update() override { emulatorExample(); }

	void setUpDebuggingWindow()
	{
		ImGui::SetNextWindowSize(ImVec2(700, 400), ImGuiCond_Once);
		ImGui::Begin("Debug", &open);
		if (ImGui::BeginCombo("Debug Level", to_string_view(logLevel).data()))
		{
			for (auto level : { spdlog::level::off, spdlog::level::critical, spdlog::level::err, spdlog::level::warn,
								spdlog::level::info, spdlog::level::debug, spdlog::level::trace })
			{
				if (ImGui::Selectable(to_string_view(level).data(), level == logLevel))
				{
					logLevel = level;
					spdlog::set_level(logLevel);
				}
			}

			ImGui::EndCombo();
		}
		ImGui::Separator();

		if (ImGui::Button("Clear"))
			_sink->GetRingBuffer().Clear();
		ImGui::SameLine();
		filter.Draw("Filter", -50);
		ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
		for (size_t i = 0; i < _sink->GetRingBuffer().Size(); ++i)
		{
			const auto& rbIter = _sink->GetRingBuffer()[i];
			if (filter.PassFilter(rbIter.second.c_str()))
			{
				ImGui::PushStyleColor(ImGuiCol_Text, logColors.at(rbIter.first));
				ImGui::TextUnformatted(rbIter.second.c_str());
				ImGui::PopStyleColor();
			}
		}
		if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
			ImGui::SetScrollHereY(1.0f);
		ImGui::EndChild();
		ImGui::End();
	}

	void setUpDisassemblerView()
	{
		ImGui::SetNextWindowSize(ImVec2(0, 400), ImGuiCond_Once);
		ImGui::Begin("Cpu View", &open);

		const auto& cpu = _emulator.GetCpu();
		const auto& ram = _emulator.GetRam();
		ImGui::Text("PC=0x%X", cpu.GetProgramCounter());
		ImGui::SameLine();
		ImGui::Text("I=0x%X", cpu.GetIndexRegister());
//		ImGui::SameLine();
		ImGui::Text("Current Instruction=0x%02X%02X", ram.GetAt(cpu.GetProgramCounter()), ram.GetAt(cpu.GetProgramCounter() + 1));
		ImGui::SameLine();
		ImGui::Text("Last Instruction=0x%04X", _emulator.GetLastExecutedInstruction());
		ImGui::Separator();

		ImGui::BeginGroup();
		ImGui::SameLine();
		ImGui::BeginGroupPanel("Stack", ImVec2(0.0f, 0.0f));
		for (size_t i = 0; i < cpu.GetStack().size(); ++i)
		{
			if (i == cpu.GetStackPointer())
				ImGui::TextColored(mahi::gui::Colors::Orange, "0x%0X", cpu.GetStack()[i]);
			else
				ImGui::Text("0x%0X", cpu.GetStack()[i]);
		}
		ImGui::EndGroupPanel();

		ImGui::SameLine();

		ImGui::BeginGroupPanel("Registers", ImVec2(0.0f, 0.0f));
		for (size_t i = 0; i < cpu.GetRegisters().size(); ++i)
		{
			ImGui::Text("V[%zX]: 0x%X", i, cpu.GetRegisters()[i]);
		}
		ImGui::EndGroupPanel();

		ImGui::SameLine();

		ImGui::BeginGroupPanel("RAM", ImVec2(0.0f, 0.0f));
		constexpr size_t width = 16;
		for (size_t i = 0; i < ram.GetSize() / 2; ++i)
		{
			for (size_t j = 0; j < width; ++j)
			{
				if (i >= ram.GetSize() / 2)
					break;

				if (i < ram.GetInstructionStartAddress())
					ImGui::TextColored(mahi::gui::Colors::Yellow, "0x%02X%02X", ram.GetAt(2 * i), ram.GetAt(2 * i + 1));
				else
				{
					if (i == cpu.GetProgramCounter())
						ImGui::TextColored(mahi::gui::Colors::Orange, "0x%02X%02X", ram.GetAt(2 * i),
										   ram.GetAt(2 * i + 1));
					else if (i == cpu.GetIndexRegister())
						ImGui::TextColored(mahi::gui::Colors::Green, "0x%02X%02X", ram.GetAt(2 * i),
										   ram.GetAt(2 * i + 1));
					else
						ImGui::Text("0x%02X%02X", ram.GetAt(2 * i), ram.GetAt(2 * i + 1));
				}
				ImGui::SameLine();
				++i;
			}
			ImGui::NewLine();
		}
		ImGui::EndGroupPanel();
		ImGui::EndGroup();

		ImGui::End();
	}

	void setUpMenuBar()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open"))
				{
					std::string out;
					if (mahi::gui::open_dialog(out, { { "Chip8 Roms", ".ch8" } }) ==
						mahi::gui::DialogResult::DialogOkay)
					{
						_emulator.LoadRom(out);
						_running = true;
					}
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}

	void drawEmulatorScreen()
	{
		auto* drawList = ImGui::GetWindowDrawList();
		auto p = ImGui::GetCursorScreenPos();
		drawList->AddRect(
			p,
			{ p.x + 2.0f * xPadding + static_cast<float>(_emulator.GetDisplay().GetWidth()) * pixelSize,
			  p.y + 2.0f * yPadding + static_cast<float>(_emulator.GetDisplay().GetHeight()) * pixelSize },
			frameColor);
		p.x += xPadding;
		p.y += yPadding;

		if (_emulator.GetDisplay().HasChanged())
		{
			for (size_t row = 0; row < _emulator.GetDisplay().GetWidth(); ++row)
			{
				auto pixelStart = p;
				pixelStart.x = p.x + static_cast<float>(row) * pixelSize;

				for (size_t col = 0; col < _emulator.GetDisplay().GetHeight(); ++col)
				{
					pixelStart.y = p.y + static_cast<float>(col) * pixelSize;
					ImVec2 pixelEnd = { pixelStart.x + pixelSize, pixelStart.y + pixelSize };
					const size_t coord = row + col * _emulator.GetDisplay().GetWidth();
					if (_emulator.GetDisplay().GetAt(coord))
						drawList->AddRectFilled(pixelStart, pixelEnd, pixelColor);
				}
			}
		}
	}

	void runEmulator()
	{
		if (ImGui::Button("Play"))
			_running = true;
		ImGui::SameLine();
		if (ImGui::Button("Stop"))
			_running = false;
		ImGui::SameLine();
		const bool stepping = ImGui::Button("Step");
		ImGui::SameLine();
		const bool rewinding = ImGui::Button("Rewind");
		if (stepping || rewinding)
			_running = false;
		if (rewinding)
		{
			_emulator.Rewind();
		}
		else if (_running || stepping)
		{
			auto pressedKeys = GetKeysPressed();
			auto releasedKeys = GetKeysReleased();
			for (size_t k = emu::Keys::START; k < emu::Keys::END; ++k)
			{
				if (pressedKeys[k])
					_emulator.GetKeypad().Press(static_cast<emu::Keys::Enum>(k), true);
				else if (releasedKeys[k])
					_emulator.GetKeypad().Press(static_cast<emu::Keys::Enum>(k), false);
			}

			bool success = _emulator.Cycle();
			if (stopOnError && !success)
			{
				_running = false;
			}
		}
	}

	void emulatorExample()
	{
		ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
		ImGui::Begin("Display", &open);

		setUpDebuggingWindow();
		setUpDisassemblerView();

		setUpMenuBar();

		ImGui::Text("%.2f FPS", static_cast<double>(ImGui::GetIO().Framerate));

		runEmulator();

		drawEmulatorScreen();

		if (!open)
			quit();

		ImGui::End();
	}


private:
	std::shared_ptr<utils::RingBufferSinkSt> _sink {};
	bool open = true;
	emu::Chip8 _emulator {};
	ImGuiTextFilter filter {};

	ImU32 pixelColor { IM_COL32(255, 0, 0, 255) };
	ImU32 frameColor { IM_COL32(255, 255, 255, 255) };
	float pixelSize = 4.0f;
	float xPadding = 50.0f;
	float yPadding = 50.0f;
	spdlog::level::level_enum logLevel = spdlog::level::warn;
	bool stopOnError = true;

	bool _running = false;
};

int main(int /*argc*/, char** /*argv*/)
{
	auto sink = RegisterRingBufferSink();
	spdlog::set_pattern("[%H:%M:%S.%F][%l][%!][ %s:%# ] %v");

	mahi::gui::Application::Config config;
	config.width = 1500;
	config.height = 900;
	config.fullscreen = false;
	config.msaa = 0;
	config.nvg_aa = false;
	config.monitor = 1;
	config.title = "Chip8 Emulator";
	config.vsync = false;
	config.decorated = true;
	//		config.background = mahi::gui::Colors::Auto;
	ChipEightEmulator app(config, sink);
	app.run();
	return 0;
}
