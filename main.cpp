
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
		__IGNORE_WARNING__("-Wredundant-tags")                                                                         \
		__IGNORE_WARNING__("-Wzero-as-null-pointer-constant")                                                          \
		__IGNORE_WARNING__("-Wsign-promo")                                                                             \
		__IGNORE_WARNING__("-Wdouble-promotion")
#endif

__START_IGNORING_WARNINGS__
__IGNORE_MAHI_GUI_WARNINGS__
#include <Mahi/Gui.hpp>
#include <Mahi/Gui/Native.hpp>
#include <Mahi/Util/Timing/Frequency.hpp>
#include <imgui_internal.h>
__STOP_IGNORING_WARNINGS__

#define LOGGER_NAME "ringBuffer"
#include "Emulator/Logging.h"

#include "Emulator/Chip8.h"
#include "Emulator/RingBuffer.h"

#include <mutex>
#include <thread>

#include "Emulator/Stopwatch.h"

enum class SaveStateOperation
{
	None,
	FileNotFound,
	Load,
	Save,
};

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
	return detail::GetKeyPressedOrReleasedWorker(
		[](int x) { return ImGui::IsKeyPressed(x) || ImGui::GetIO().KeysDownDuration[x] > 0.0f; }, true);
}
static inline auto GetKeysReleased()
{
	return detail::GetKeyPressedOrReleasedWorker([](int x) { return ImGui::IsKeyReleased(x); }, true);
}

static inline ImU32 Convert(const mahi::gui::Color& color)
{
	__START_IGNORING_WARNINGS__
	__IGNORE_WARNING__("-Wold-style-cast")
	return IM_COL32(color.r * 255, color.g * 255, color.b * 255, color.a * 255);
	__STOP_IGNORING_WARNINGS__
}

namespace ImGui
{
	// https://github.com/ocornut/imgui/issues/1496
	static ImVector<ImRect> s_GroupPanelLabelStack;

	static void BeginGroupPanel(const char* name, const ImVec2& size)
	{
		ImGui::BeginGroup();

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
		auto labelMin = ImGui::GetItemRectMin();
		auto labelMax = ImGui::GetItemRectMax();
		ImGui::SameLine(0.0f, 0.0f);
		ImGui::Dummy(ImVec2(0.0, frameHeight + itemSpacing.y));
		ImGui::BeginGroup();

		//ImGui::GetWindowDrawList()->AddRect(labelMin, labelMax, IM_COL32(255, 0, 255, 255));

		ImGui::PopStyleVar(2);

#if IMGUI_VERSION_NUM >= 17301
		ImGui::GetCurrentWindow()->ContentRegionRect.Max.x -= frameHeight * 0.5f;
		ImGui::GetCurrentWindow()->WorkRect.Max.x          -= frameHeight * 0.5f;
		ImGui::GetCurrentWindow()->InnerRect.Max.x         -= frameHeight * 0.5f;
#else
		ImGui::GetCurrentWindow()->ContentsRegionRect.Max.x -= frameHeight * 0.5f;
#endif
		ImGui::GetCurrentWindow()->Size.x                   -= frameHeight;

		auto itemWidth = ImGui::CalcItemWidth();
		ImGui::PushItemWidth(ImMax(0.0f, itemWidth - frameHeight));

		s_GroupPanelLabelStack.push_back(ImRect(labelMin, labelMax));
	}

	static void EndGroupPanel()
	{
		ImGui::PopItemWidth();

		auto itemSpacing = ImGui::GetStyle().ItemSpacing;

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

		auto frameHeight = ImGui::GetFrameHeight();

		ImGui::EndGroup();

		//ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(0, 255, 0, 64), 4.0f);

		ImGui::EndGroup();

		ImGui::SameLine(0.0f, 0.0f);
		ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
		ImGui::Dummy(ImVec2(0.0, frameHeight - frameHeight * 0.5f - itemSpacing.y));

		ImGui::EndGroup();

		auto itemMin = ImGui::GetItemRectMin();
		auto itemMax = ImGui::GetItemRectMax();
		//ImGui::GetWindowDrawList()->AddRectFilled(itemMin, itemMax, IM_COL32(255, 0, 0, 64), 4.0f);

		auto labelRect = s_GroupPanelLabelStack.back();
		s_GroupPanelLabelStack.pop_back();

		ImVec2 halfFrame = ImVec2(frameHeight * 0.25f * 0.5f, frameHeight * 0.5f);
		ImRect frameRect = ImRect({itemMin.x + halfFrame.x, itemMin.y + halfFrame.y}, {itemMax.x - halfFrame.x, itemMax.y});
		labelRect.Min.x -= itemSpacing.x;
		labelRect.Max.x += itemSpacing.x;
		for (int i = 0; i < 4; ++i)
		{
			switch (i)
			{
				// left half-plane
				case 0: ImGui::PushClipRect(ImVec2(-FLT_MAX, -FLT_MAX), ImVec2(labelRect.Min.x, FLT_MAX), true); break;
				// right half-plane
				case 1: ImGui::PushClipRect(ImVec2(labelRect.Max.x, -FLT_MAX), ImVec2(FLT_MAX, FLT_MAX), true); break;
				// top
				case 2: ImGui::PushClipRect(ImVec2(labelRect.Min.x, -FLT_MAX), ImVec2(labelRect.Max.x, labelRect.Min.y), true); break;
				// bottom
				case 3: ImGui::PushClipRect(ImVec2(labelRect.Min.x, labelRect.Max.y), ImVec2(labelRect.Max.x, FLT_MAX), true); break;
				default:
					assert(false);
					break;
			}

			ImGui::GetWindowDrawList()->AddRect(
				frameRect.Min, frameRect.Max,
				ImColor(ImGui::GetStyleColorVec4(ImGuiCol_Border)),
				halfFrame.x);

			ImGui::PopClipRect();
		}

		ImGui::PopStyleVar(2);

#if IMGUI_VERSION_NUM >= 17301
		ImGui::GetCurrentWindow()->ContentRegionRect.Max.x += frameHeight * 0.5f;
		ImGui::GetCurrentWindow()->WorkRect.Max.x          += frameHeight * 0.5f;
		ImGui::GetCurrentWindow()->InnerRect.Max.x         += frameHeight * 0.5f;
#else
		ImGui::GetCurrentWindow()->ContentsRegionRect.Max.x += frameHeight * 0.5f;
#endif
		ImGui::GetCurrentWindow()->Size.x                   += frameHeight;

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
		setFpsLimit();
	}
	virtual ~ChipEightEmulator() = default;

private:
	// Override update (called once per frame)
	void update() override 
	{ 
		setupMainWindow();
		runEmulator();
		drawEmulatorScreen();
		
		setUpDebuggingWindow();
		setUpDisassemblerView();
		setUpMenuBar();

		if (!_open)
			quit();

		ImGui::End(); 
	}

	void setFpsLimit()
	{
		if (_capFps)
			set_frame_limit(mahi::util::hertz(static_cast<long long int>(_fps)));
		else
			set_frame_limit(mahi::util::hertz(0));
	}

	void setUpDebuggingWindow()
	{
		if (!_viewLogging)
			return;

		ImGui::SetNextWindowSize({ 656, 369 }, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos({ 14, 502 }, ImGuiCond_FirstUseEver);
		ImGui::Begin("Logging", &_viewLogging);
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
		_filter.Draw("Filter", -50);
		ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
		for (size_t i = 0; i < _sink->GetRingBuffer().Size(); ++i)
		{
			const auto& rbIter = _sink->GetRingBuffer()[i];
			if (_filter.PassFilter(rbIter.second.c_str()))
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
		if (!_viewDisassembler)
			return;

		ImGui::SetNextWindowSize({ 963, 832 }, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos({ 676, 38 }, ImGuiCond_FirstUseEver);
		ImGui::Begin("Disassembler", &_viewDisassembler);

		const auto& cpu = _emulator.GetCpu();
		const auto& ram = _emulator.GetRam();
		ImGui::Text("PC=0x%04X", cpu.GetProgramCounter());
		ImGui::SameLine();
		ImGui::Text("I=0x%04X", cpu.GetIndexRegister());
		//		ImGui::SameLine();
		ImGui::Text("Current Instruction=0x%02X%02X", ram.GetAt(cpu.GetProgramCounter()),
					ram.GetAt(cpu.GetProgramCounter() + 1));
		//		ImGui::SameLine();
		ImGui::Text("Last Instruction=%s(0x%04X)", emu::ToString(_emulator.GetLastExecutedInstructionCode()).data(),
					_emulator.GetLastExecutedInstruction());
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

		ImGui::BeginGroupPanel("Keypad", ImVec2(0.0f, 0.0f));

		//		const auto pressedKeys = GetKeysPressed();
		//		const auto releasedKeys = GetKeysReleased();
		for (size_t k = emu::Keys::START; k < emu::Keys::END; ++k)
		{
			ImGui::Text("Key[%s]", emu::ToString(static_cast<emu::Keys::Enum>(k)).data());
			//			ImGui::SameLine();
			//			bool p = pressedKeys[k];
			//			ImGui::Checkbox("", &p);
			//
			ImGui::SameLine();

			bool isKeyPressed = _emulator.GetKeypad().IsPressed(static_cast<emu::Keys::Enum>(k));
			ImGui::Checkbox("", &isKeyPressed);
			//			if (pressedKeys[k])
			//				_emulator.GetKeypad().Press(static_cast<emu::Keys::Enum>(k), true);
			//			else if (releasedKeys[k])
			//				_emulator.GetKeypad().Press(static_cast<emu::Keys::Enum>(k), false);
		}

		ImGui::EndGroupPanel();

		ImGui::SameLine();

		ImGui::BeginGroupPanel("RAM", ImVec2(0.0f, 0.0f));
		constexpr size_t width = 16;
		for (size_t i = 0; i < ram.GetSize() - 1;)
		{
			ImGui::Text("0x%03zX: ", i);
			ImGui::SameLine();
			for (size_t j = 0; j < width; ++j)
			{
				if (i >= ram.GetSize() - 1)
					break;

				if (i < ram.GetInstructionStartAddress())
					ImGui::TextColored(mahi::gui::Colors::Yellow, "%02X%02X", ram.GetAt(i), ram.GetAt(i + 1));
				else
				{
					if (i == cpu.GetProgramCounter())
						ImGui::TextColored(mahi::gui::Colors::Orange, "%02X%02X", ram.GetAt(i), ram.GetAt(i + 1));
					else if (i == cpu.GetIndexRegister())
						ImGui::TextColored(mahi::gui::Colors::Green, "%02X%02X", ram.GetAt(i), ram.GetAt(i + 1));
					else
						ImGui::Text("%02X%02X", ram.GetAt(i), ram.GetAt(i + 1));
				}
				ImGui::SameLine();
				i += 1;
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
					if (mahi::gui::open_dialog(_selectedRom, { { "Chip8 Roms", "ch8" } }) ==
						mahi::gui::DialogResult::DialogOkay)
					{
						_emulator.LoadRom(_selectedRom);
						_running = true;
					}
				}

				if (ImGui::BeginMenu("Load State"))
				{
					for (size_t i = 0; i < 9; ++i)
					{
						if (ImGui::MenuItem(std::to_string(i).c_str()))
						{
							std::filesystem::path saveState = _selectedRom + ".sav." + std::to_string(i);

							_showSaveStateOperation = true;
							_saveStateTimer = std::chrono::system_clock::now();
							_selectedSaveState = i;

							if (!std::filesystem::exists(saveState))
							{
								_saveStateOperation = SaveStateOperation::FileNotFound;
							}
							else
							{
								std::ifstream saveStateFile(saveState.string(), std::ios::binary | std::ios::ate);

								const auto fileSize = static_cast<size_t>(saveStateFile.tellg());

								std::vector<emu::Byte> byteArray(fileSize);
								saveStateFile.seekg(saveStateFile.beg);
								saveStateFile.read(reinterpret_cast<char*>(byteArray.data()),
												   static_cast<long>(fileSize));
								_emulator.Deserialize(byteArray);

								_saveStateOperation = SaveStateOperation::Load;
							}
						}
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Save State"))
				{
					for (size_t i = 0; i < 9; ++i)
					{
						if (ImGui::MenuItem(std::to_string(i).c_str()))
						{
							std::filesystem::path saveState = _selectedRom + ".sav." + std::to_string(i);

							std::vector<emu::Byte> byteArray;
							_emulator.Serialize(byteArray);
							std::ofstream saveStateFile(saveState.string(), std::ios::binary);
							saveStateFile.write(reinterpret_cast<const char*>(&byteArray[0]),
												static_cast<long>(byteArray.size() * sizeof(emu::Byte)));

							_showSaveStateOperation = true;
							_saveStateTimer = std::chrono::system_clock::now();
							_selectedSaveState = i;
							_saveStateOperation = SaveStateOperation::Save;
						}
					}

					ImGui::EndMenu();
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Restart"))
				{
					_emulator.LoadRom(_selectedRom);
					_running = true;
				}
				ImGui::MenuItem("Cap FPS", nullptr, &_capFps);

				const auto changed = ImGui::SliderFloat("FPS Limit", &_fps, 0.0f, 400.0f, "%.1f Hz");
				if (std::abs(_fps - 400.0f) < 1e-7f)
					_capFps = false;
				if (changed)
					_capFps = true;

				if (_capFps)
					set_frame_limit(mahi::util::hertz(static_cast<long long int>(_fps)));
				else
					set_frame_limit(mahi::util::hertz(0));
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View"))
			{
				ImGui::MenuItem("View FPS", nullptr, &_viewFps);
				ImGui::MenuItem("View Cycles", nullptr, &_viewCycles);
				ImGui::MenuItem("View Performance", nullptr, &_viewPerformance);
				ImGui::SliderFloat("Pixel Size", &_pixelSize, 0.1f, 20.0f, "%.1f");
				ImGui::SliderFloat("X-Padding", &_xPadding, 0.f, 100.0f, "%.1f");
				ImGui::SliderFloat("Y-Padding", &_yPadding, 0.f, 100.0f, "%.1f");

				ImGui::ColorEdit3("Pixel Color", _pixelColor.rgba);
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Debug"))
			{
				ImGui::MenuItem("View Disassembler", nullptr, &_viewDisassembler);
				ImGui::MenuItem("View Logging", nullptr, &_viewLogging);

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		if (_showSaveStateOperation)
		{
			auto color = mahi::gui::Colors::Red;
			auto timeDiff = std::chrono::system_clock::now() - _saveStateTimer;
			constexpr auto maxDurationSeconds = 3.0f;
			color.a = std::max(0.0f, (1.0f - (std::chrono::duration<float>(timeDiff).count() / maxDurationSeconds)));
			if (std::abs(color.a) < 1e-7f)
			{
				_showSaveStateOperation = false;
			}
			else
			{
				auto* drawList = ImGui::GetWindowDrawList();
				auto p = ImGui::GetCursorScreenPos();
				p.x += ImGui::GetWindowWidth() / 3.0f;

				std::string str;
				switch (_saveStateOperation)
				{
					case SaveStateOperation::FileNotFound:
						str = "Save state " + std::to_string(_selectedSaveState) + " not found";
						break;
					case SaveStateOperation::Load:
						str = "Loaded state " + std::to_string(_selectedSaveState);
						break;
					case SaveStateOperation::Save:
						str = "Saved state " + std::to_string(_selectedSaveState);
						break;
					default:
						break;
				}
				drawList->AddText(p, Convert(color), str.c_str());
			}
		}
	}

	void setupMainWindow()
	{
		const auto xDim = 2.0f * _xPadding + static_cast<float>(_emulator.GetDisplay().GetWidth()) * _pixelSize;
		const auto yDim = 2.0f * _yPadding + static_cast<float>(_emulator.GetDisplay().GetHeight()) * _pixelSize;
		ImGui::SetNextWindowSize(ImVec2(20 + xDim, 120 + yDim), ImGuiCond_Always);
		ImGui::SetNextWindowPos({ 82, 31 }, ImGuiCond_FirstUseEver);
		ImGui::Begin("Display", &_open, ImGuiWindowFlags_NoResize);

		if (_viewFps)
			ImGui::Text("%.2f FPS", static_cast<double>(ImGui::GetIO().Framerate));
		//		ImGui::SameLine();

		if (_viewCycles)
			ImGui::Text("%lu Cycles", _cycles);

		if (_viewPerformance)
		{
			if (_cyclesPerSeconds > 1e9)
				ImGui::Text("%.2f GHz", _cyclesPerSeconds * 1e-9);
			else if (_cyclesPerSeconds > 1e6)
				ImGui::Text("%.2f MHz", _cyclesPerSeconds * 1e-6);
			else if (_cyclesPerSeconds > 1e3)
				ImGui::Text("%.2f KHz", _cyclesPerSeconds * 1e-3);
			else
				ImGui::Text("%.2f Hz", _cyclesPerSeconds);
		}

		if (ImGui::Button("Play"))
			_running = true;
		ImGui::SameLine();
		if (ImGui::Button("Stop"))
			_running = false;
		ImGui::SameLine();
		_stepping = ImGui::Button("Step");
		ImGui::SameLine();
		_rewinding = ImGui::Button("Rewind");
		if (_stepping || _rewinding)
			_running = false;
	}

	void runEmulator()
	{
		if (_rewinding)
		{
			if (_cycles > 0)
				--_cycles;
			_emulator.Rewind();
		}
		else if (_running || _stepping)
		{
			const auto pressedKeys = GetKeysPressed();
			const auto releasedKeys = GetKeysReleased();
			for (size_t k = emu::Keys::START; k < emu::Keys::END; ++k)
			{
				if (pressedKeys[k])
					_emulator.GetKeypad().Press(static_cast<emu::Keys::Enum>(k), true);
				else if (releasedKeys[k])
					_emulator.GetKeypad().Press(static_cast<emu::Keys::Enum>(k), false);
			}

			utils::StopWatch sw;
			bool success = _emulator.Cycle();
			sw.Stop();
			_cyclesPerSeconds = 1.0 / sw.GetSeconds();

			++_cycles;
			if (_stopOnError && !success)
			{
				_running = false;
			}
		}
	}

	void drawEmulatorScreen()
	{
		auto* drawList = ImGui::GetWindowDrawList();

		auto p = ImGui::GetCursorScreenPos();
		drawList->AddRect(
			p,
			{ p.x + 2.0f * _xPadding + static_cast<float>(_emulator.GetDisplay().GetWidth()) * _pixelSize,
			  p.y + 2.0f * _yPadding + static_cast<float>(_emulator.GetDisplay().GetHeight()) * _pixelSize },
			Convert(_frameColor));
		p.x += _xPadding;
		p.y += _yPadding;

		for (size_t row = 0; row < _emulator.GetDisplay().GetWidth(); ++row)
		{
			auto pixelStart = p;
			pixelStart.x = p.x + static_cast<float>(row) * _pixelSize;

			for (size_t col = 0; col < _emulator.GetDisplay().GetHeight(); ++col)
			{
				pixelStart.y = p.y + static_cast<float>(col) * _pixelSize;
				ImVec2 pixelEnd = { pixelStart.x + _pixelSize, pixelStart.y + _pixelSize };
				const size_t coord = row + col * _emulator.GetDisplay().GetWidth();
				if (_emulator.GetDisplay().GetAt(coord))
					drawList->AddRectFilled(pixelStart, pixelEnd, Convert(_pixelColor));
			}
		}
	}

private:
	emu::Chip8 _emulator {};
	std::string _selectedRom {};

	spdlog::level::level_enum logLevel = spdlog::level::warn;
	std::shared_ptr<utils::RingBufferSinkSt> _sink {};
	ImGuiTextFilter _filter {};

	bool _stopOnError = true;

	bool _open = true;
	mahi::gui::Color _pixelColor { mahi::gui::Colors::Red };
	mahi::gui::Color _frameColor {{{ 1.0f, 1.0f, 1.0f, 0.1f }}};
	float _pixelSize = 7.2f;
	float _xPadding = 50.0f;
	float _yPadding = 50.0f;

	bool _showSaveStateOperation = false;
	size_t _selectedSaveState = 0;
	std::chrono::system_clock::time_point _saveStateTimer {};
	SaveStateOperation _saveStateOperation = SaveStateOperation::None;

	bool _running = false;
	bool _rewinding = false;
	bool _stepping = false;
	bool _capFps = true;
	float _fps = 60.0f;
	std::uint64_t _cycles = 0;
	double _cyclesPerSeconds = 0.0;
	bool _viewFps = true;
	bool _viewCycles = true;
	bool _viewPerformance = true;
	bool _viewDisassembler = false;
	bool _viewLogging = false;
};

int main(int /*argc*/, char** /*argv*/)
{
	auto sink = RegisterRingBufferSink();
	spdlog::set_pattern("[%H:%M:%S.%F][%l][%!][ %s:%# ] %v");

	mahi::gui::Application::Config config;
	config.width = 1650;
	config.height = 900;
	config.fullscreen = false;
	config.msaa = 0;
	config.nvg_aa = false;
	config.monitor = 1;
	config.title = "Chip8 Emulator";
	config.vsync = false;
	config.decorated = true;
	ChipEightEmulator app(config, sink);
	app.run();
	return 0;
}
