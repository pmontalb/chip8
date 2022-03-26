
#include "IgnoreWarning.h"

#ifdef __clang__
#define __IGNORE_MAHI_GUI_WARNINGS__ \
	__IGNORE_WARNING__("-Wsign-conversion") \
	__IGNORE_WARNING__("-Wfloat-equal")  \
	__IGNORE_WARNING__("-Wold-style-cast") \
	__IGNORE_WARNING__("-Wdouble-promotion") \
	__IGNORE_WARNING__("-Wnewline-eof")    \
	__IGNORE_WARNING__("-Wdocumentation-unknown-command") \
	__IGNORE_WARNING__("-Wdocumentation")\
	__IGNORE_WARNING__("-Wgnu-anonymous-struct")    \
	__IGNORE_WARNING__("-Wnested-anon-types")       \
	__IGNORE_WARNING__("-Wunused-parameter")\
	__IGNORE_WARNING__("-Wmissing-braces")  \
	__IGNORE_WARNING__("-Wnon-virtual-dtor")\
	__IGNORE_WARNING__("-Wundef")        \
	__IGNORE_WARNING__("-Wimplicit-float-conversion") \
	__IGNORE_WARNING__("-Wimplicit-int-float-conversion")          \
	__IGNORE_WARNING__("-Wzero-as-null-pointer-constant") \
	__IGNORE_WARNING__("-Wdeprecated-copy-dtor")    \
	__IGNORE_WARNING__("-Wextra-semi")   \
	__IGNORE_WARNING__("-Wundefined-func-template") \
	__IGNORE_WARNING__("-Wshadow-field-in-constructor")
#else
#define __IGNORE_MAHI_GUI_WARNINGS__ \
	__IGNORE_WARNING__("-Wshadow")       \
	__IGNORE_WARNING__("-Wfloat-equal")  \
	__IGNORE_WARNING__("-Wold-style-cast") \
	__IGNORE_WARNING__("-Wpedantic")     \
	__IGNORE_WARNING__("-Wunused-parameter") \
	__IGNORE_WARNING__("-Weffc++")       \
	__IGNORE_WARNING__("-Wnon-virtual-dtor") \
	__IGNORE_WARNING__("-Wconversion")   \
	__IGNORE_WARNING__("-Wuseless-cast") \
	__IGNORE_WARNING__("-Wextra-semi")   \
	__IGNORE_WARNING__("-Wsuggest-override") \
	__IGNORE_WARNING__("-Wctor-dtor-privacy")\
	__IGNORE_WARNING__("-Wuseless-cast") \
	__IGNORE_WARNING__("-Wdeprecated")  \
    __IGNORE_WARNING__("-Wredundant-tags")\
	__IGNORE_WARNING__("-Wdouble-promotion")
#endif

__START_IGNORING_WARNINGS__
__IGNORE_MAHI_GUI_WARNINGS__
#include <Mahi/Gui.hpp>
__STOP_IGNORING_WARNINGS__

#define LOGGER_NAME "ringBuffer"
#include "Emulator/Logging.h"

#include "Emulator/Chip8.h"
#include "Emulator/RingBuffer.h"

#include <mutex>
#include <thread>

static const std::unordered_map<spdlog::level::level_enum, mahi::gui::Color> logColors = {
	{ spdlog::level::trace, mahi::gui::Greens::LimeGreen },	{ spdlog::level::debug, mahi::gui::Greens::LightGreen },
	{ spdlog::level::info, mahi::gui::Whites::White }, { spdlog::level::warn, mahi::gui::Yellows::Yellow },
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

static void ShowExampleMenuFile()
{
	ImGui::MenuItem("(demo menu)", nullptr, false, false);
	if (ImGui::MenuItem("New"))
	{
	}
	if (ImGui::MenuItem("Open", "Ctrl+O"))
	{
	}
	if (ImGui::BeginMenu("Open Recent"))
	{
		ImGui::MenuItem("fish_hat.c");
		ImGui::MenuItem("fish_hat.inl");
		ImGui::MenuItem("fish_hat.h");
		if (ImGui::BeginMenu("More.."))
		{
			ImGui::MenuItem("Hello");
			ImGui::MenuItem("Sailor");
			if (ImGui::BeginMenu("Recurse.."))
			{
				ShowExampleMenuFile();
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenu();
	}
	if (ImGui::MenuItem("Save", "Ctrl+S"))
	{
	}
	if (ImGui::MenuItem("Save As.."))
	{
	}

	ImGui::Separator();
	if (ImGui::BeginMenu("Options"))
	{
		static bool enabled = true;
		ImGui::MenuItem("Enabled", "", &enabled);
		ImGui::BeginChild("child", ImVec2(0, 60), true);
		for (int i = 0; i < 10; i++)
			ImGui::Text("Scrolling Text %d", i);
		ImGui::EndChild();
		static float f = 0.5f;
		static int n = 0;
		ImGui::SliderFloat("Value", &f, 0.0f, 1.0f);
		ImGui::InputFloat("Input", &f, 0.1f);
		ImGui::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Colors"))
	{
		float sz = ImGui::GetTextLineHeight();
		for (int i = 0; i < ImGuiCol_COUNT; i++)
		{
			const char* name = ImGui::GetStyleColorName(i);
			ImVec2 p = ImGui::GetCursorScreenPos();
			ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + sz, p.y + sz), ImGui::GetColorU32(i));
			ImGui::Dummy(ImVec2(sz, sz));
			ImGui::SameLine();
			ImGui::MenuItem(name);
		}
		ImGui::EndMenu();
	}

	// Here we demonstrate appending again to the "Options" menu (which we already created above)
	// Of course in this demo it is a little bit silly that this function calls BeginMenu("Options") twice.
	// In a real code-base using it would make senses to use this feature from very different code locations.
	if (ImGui::BeginMenu("Options"))	// <-- Append!
	{
		static bool b = true;
		ImGui::Checkbox("SomeOption", &b);
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Disabled", false))	// Disabled
	{
		IM_ASSERT(0);
	}
	if (ImGui::MenuItem("Checked", nullptr, true))
	{
	}
	if (ImGui::MenuItem("Quit", "Alt+F4"))
	{
	}
}

// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.md)
static void HelpMarker(const char* desc)
{
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

__START_IGNORING_WARNINGS__
__IGNORE_WARNING__("-Wnon-virtual-dtor")
class ChipEightEmulator: public mahi::gui::Application
{
public:
	explicit ChipEightEmulator(const mahi::gui::Application::Config& config, std::shared_ptr<utils::RingBufferSinkSt> sink)
		: Application(config), _sink(sink)
	{
	}
	virtual ~ChipEightEmulator() = default;

private:
	// Override update (called once per frame)
	void update() override { logExample(); }

	void logExample()
	{
		ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
		ImGui::Begin("MAHI Log", &open);
		for (int i = 0; i < 6; ++i)
		{
			if (ImGui::Button((std::to_string(i) + " - Try").c_str()))
			{
				if (i == 0)
					LOG_CRITICAL("Here's a critical line={}", i);
				else if (i == 1)
					LOG_ERROR("Here's an error line={:x}", i);
				else if (i == 2)
					LOG_WARN("Here's a warn line={:3f}", 2.0 * i);
				else if (i == 3)
					LOG_INFO("Here's an info line={}", 4.0 * i);
				else if (i == 4)
					LOG_DEBUG("Here's a debug line={}", "yup");
				else if (i == 5)
					LOG_TRACE("Here's a trace line={}-{}", "!!!", 2.7182);
			}

			if (i != 6)
				ImGui::SameLine();
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

		if (!open)
			quit();
	}

	void example()
	{
		// We specify a default position/size in case there's no data in the .ini file.
		// We only do it to make the demo applications a little more welcoming, but typically this isn't required.
		ImGuiViewport* main_viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(ImVec2(main_viewport->GetWorkPos().x + 650, main_viewport->GetWorkPos().y + 20),
								ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);

		// Demonstrate the various window flags. Typically you would just use the default!
		static bool no_titlebar = false;
		static bool no_scrollbar = false;
		static bool no_menu = false;
		static bool no_move = false;
		static bool no_resize = false;
		static bool no_collapse = false;
		static bool no_close = false;
		static bool no_nav = false;
		static bool no_background = false;
		static bool no_bring_to_front = false;
		static bool no_docking = false;

		ImGuiWindowFlags window_flags = 0;
		if (no_titlebar)
			window_flags |= ImGuiWindowFlags_NoTitleBar;
		if (no_scrollbar)
			window_flags |= ImGuiWindowFlags_NoScrollbar;
		if (!no_menu)
			window_flags |= ImGuiWindowFlags_MenuBar;
		if (no_move)
			window_flags |= ImGuiWindowFlags_NoMove;
		if (no_resize)
			window_flags |= ImGuiWindowFlags_NoResize;
		if (no_collapse)
			window_flags |= ImGuiWindowFlags_NoCollapse;
		if (no_nav)
			window_flags |= ImGuiWindowFlags_NoNav;
		if (no_background)
			window_flags |= ImGuiWindowFlags_NoBackground;
		if (no_bring_to_front)
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
		if (no_docking)
			window_flags |= ImGuiWindowFlags_NoDocking;

		// Main body of the Demo window starts here.
		if (!ImGui::Begin("Dear ImGui Demo", &open, window_flags))
		{
			// Early out if the window is collapsed, as an optimization.
			ImGui::End();
			return;
		}

		// Most "big" widgets share a common width settings by default. See 'Demo->Layout->Widgets Width' for details.

		// e.g. Use 2/3 of the space for widgets and 1/3 for labels (right align)
		// ImGui::PushItemWidth(-ImGui::GetWindowWidth() * 0.35f);

		// e.g. Leave a fixed amount of width for labels (by passing a negative value), the rest goes to widgets.
		ImGui::PushItemWidth(ImGui::GetFontSize() * -12);

		// Examples Apps (accessible from the "Examples" menu)
		static bool show_app_main_menu_bar = false;
		static bool show_app_dockspace = false;
		static bool show_app_documents = false;

		static bool show_app_console = false;
		static bool show_app_log = false;
		static bool show_app_layout = false;
		static bool show_app_property_editor = false;
		static bool show_app_long_text = false;
		static bool show_app_auto_resize = false;
		static bool show_app_constrained_resize = false;
		static bool show_app_simple_overlay = false;
		static bool show_app_window_titles = false;
		static bool show_app_custom_rendering = false;

		// Dear ImGui Apps (accessible from the "Tools" menu)
		static bool show_app_metrics = false;
		static bool show_app_style_editor = false;
		static bool show_app_about = false;

		// e.g. Leave a fixed amount of width for labels (by passing a negative value), the rest goes to widgets.
		ImGui::PushItemWidth(ImGui::GetFontSize() * -12);

		// Menu Bar
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Menu"))
			{
				ShowExampleMenuFile();
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Examples"))
			{
				ImGui::MenuItem("Main menu bar", nullptr, &show_app_main_menu_bar);
				ImGui::MenuItem("Console", nullptr, &show_app_console);
				ImGui::MenuItem("Log", nullptr, &show_app_log);
				ImGui::MenuItem("Simple layout", nullptr, &show_app_layout);
				ImGui::MenuItem("Property editor", nullptr, &show_app_property_editor);
				ImGui::MenuItem("Long text display", nullptr, &show_app_long_text);
				ImGui::MenuItem("Auto-resizing window", nullptr, &show_app_auto_resize);
				ImGui::MenuItem("Constrained-resizing window", nullptr, &show_app_constrained_resize);
				ImGui::MenuItem("Simple overlay", nullptr, &show_app_simple_overlay);
				ImGui::MenuItem("Manipulating window titles", nullptr, &show_app_window_titles);
				ImGui::MenuItem("Custom rendering", nullptr, &show_app_custom_rendering);
				ImGui::MenuItem("Dockspace", nullptr, &show_app_dockspace);
				ImGui::MenuItem("Documents", nullptr, &show_app_documents);
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Tools"))
			{
				ImGui::MenuItem("Metrics/Debugger", nullptr, &show_app_metrics);
				ImGui::MenuItem("Style Editor", nullptr, &show_app_style_editor);
				ImGui::MenuItem("About Dear ImGui", nullptr, &show_app_about);
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		ImGui::Text("dear imgui says hello. (%s)", IMGUI_VERSION);
		ImGui::Spacing();

		if (ImGui::CollapsingHeader("Help"))
		{
			ImGui::Text("ABOUT THIS DEMO:");
			ImGui::BulletText("Sections below are demonstrating many aspects of the library.");
			ImGui::BulletText("The \"Examples\" menu above leads to more demo contents.");
			ImGui::BulletText("The \"Tools\" menu above gives access to: About Box, Style Editor,\n"
							  "and Metrics/Debugger (general purpose Dear ImGui debugging tool).");
			ImGui::Separator();

			ImGui::Text("PROGRAMMER GUIDE:");
			ImGui::BulletText("See the ShowDemoWindow() code in imgui_demo.cpp. <- you are here!");
			ImGui::BulletText("See comments in imgui.cpp.");
			ImGui::BulletText("See example applications in the examples/ folder.");
			ImGui::BulletText("Read the FAQ at http://www.dearimgui.org/faq/");
			ImGui::BulletText("Set 'io.ConfigFlags |= NavEnableKeyboard' for keyboard controls.");
			ImGui::BulletText("Set 'io.ConfigFlags |= NavEnableGamepad' for gamepad controls.");
			ImGui::Separator();

			ImGui::Text("USER GUIDE:");
			ImGui::ShowUserGuide();
		}

		if (ImGui::CollapsingHeader("Configuration"))
		{
			ImGuiIO& io = ImGui::GetIO();

			if (ImGui::TreeNode("Configuration##2"))
			{
				ImGui::CheckboxFlags("io.ConfigFlags: NavEnableKeyboard", &io.ConfigFlags,
									 ImGuiConfigFlags_NavEnableKeyboard);
				ImGui::SameLine();
				HelpMarker("Enable keyboard controls.");
				ImGui::CheckboxFlags("io.ConfigFlags: NavEnableGamepad", &io.ConfigFlags,
									 ImGuiConfigFlags_NavEnableGamepad);
				ImGui::SameLine();
				HelpMarker(
					"Enable gamepad controls. Require backend to set io.BackendFlags |= ImGuiBackendFlags_HasGamepad.\n\nRead instructions in imgui.cpp for details.");
				ImGui::CheckboxFlags("io.ConfigFlags: NavEnableSetMousePos", &io.ConfigFlags,
									 ImGuiConfigFlags_NavEnableSetMousePos);
				ImGui::SameLine();
				HelpMarker(
					"Instruct navigation to move the mouse cursor. See comment for ImGuiConfigFlags_NavEnableSetMousePos.");
				ImGui::CheckboxFlags("io.ConfigFlags: NoMouse", &io.ConfigFlags, ImGuiConfigFlags_NoMouse);
				if (io.ConfigFlags & ImGuiConfigFlags_NoMouse)
				{
					// The "NoMouse" option can get us stuck with a disabled mouse! Let's provide an alternative way to
					// fix it:
					if (fmodf(static_cast<float>(ImGui::GetTime()), 0.40f) < 0.20f)
					{
						ImGui::SameLine();
						ImGui::Text("<<PRESS SPACE TO DISABLE>>");
					}
					if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Space)))
						io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
				}
				ImGui::CheckboxFlags("io.ConfigFlags: NoMouseCursorChange", &io.ConfigFlags,
									 ImGuiConfigFlags_NoMouseCursorChange);
				ImGui::SameLine();
				HelpMarker("Instruct backend to not alter mouse cursor shape and visibility.");

				ImGui::CheckboxFlags("io.ConfigFlags: DockingEnable", &io.ConfigFlags, ImGuiConfigFlags_DockingEnable);
				ImGui::SameLine();
				HelpMarker(io.ConfigDockingWithShift ? "[beta] Use SHIFT to dock window into each others."
													 : "[beta] Drag from title bar to dock windows into each others.");
				if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
				{
					ImGui::Indent();
					ImGui::Checkbox("io.ConfigDockingNoSplit", &io.ConfigDockingNoSplit);
					ImGui::SameLine();
					HelpMarker(
						"Simplified docking mode: disable window splitting, so docking is limited to merging multiple windows together into tab-bars.");
					ImGui::Checkbox("io.ConfigDockingWithShift", &io.ConfigDockingWithShift);
					ImGui::SameLine();
					HelpMarker(
						"Enable docking when holding Shift only (allows to drop in wider space, reduce visual noise)");
					ImGui::Checkbox("io.ConfigDockingAlwaysTabBar", &io.ConfigDockingAlwaysTabBar);
					ImGui::SameLine();
					HelpMarker("Create a docking node and tab-bar on single floating windows.");
					ImGui::Checkbox("io.ConfigDockingTransparentPayload", &io.ConfigDockingTransparentPayload);
					ImGui::SameLine();
					HelpMarker(
						"Make window or viewport transparent when docking and only display docking boxes on the target viewport. Useful if rendering of multiple viewport cannot be synced. Best used with ConfigViewportsNoAutoMerge.");
					ImGui::Unindent();
				}

				ImGui::CheckboxFlags("io.ConfigFlags: ViewportsEnable", &io.ConfigFlags,
									 ImGuiConfigFlags_ViewportsEnable);
				ImGui::SameLine();
				HelpMarker("[beta] Enable beta multi-viewports support. See ImGuiPlatformIO for details.");
				if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
				{
					ImGui::Indent();
					ImGui::Checkbox("io.ConfigViewportsNoAutoMerge", &io.ConfigViewportsNoAutoMerge);
					ImGui::SameLine();
					HelpMarker(
						"Set to make all floating imgui windows always create their own viewport. Otherwise, they are merged into the main host viewports when overlapping it.");
					ImGui::Checkbox("io.ConfigViewportsNoTaskBarIcon", &io.ConfigViewportsNoTaskBarIcon);
					ImGui::SameLine();
					HelpMarker(
						"Toggling this at runtime is normally unsupported (most platform backends won't refresh the task bar icon state right away).");
					ImGui::Checkbox("io.ConfigViewportsNoDecoration", &io.ConfigViewportsNoDecoration);
					ImGui::SameLine();
					HelpMarker(
						"Toggling this at runtime is normally unsupported (most platform backends won't refresh the decoration right away).");
					ImGui::Checkbox("io.ConfigViewportsNoDefaultParent", &io.ConfigViewportsNoDefaultParent);
					ImGui::SameLine();
					HelpMarker(
						"Toggling this at runtime is normally unsupported (most platform backends won't refresh the parenting right away).");
					ImGui::Unindent();
				}

				ImGui::Checkbox("io.ConfigInputTextCursorBlink", &io.ConfigInputTextCursorBlink);
				ImGui::SameLine();
				HelpMarker("Set to false to disable blinking cursor, for users who consider it distracting");
				ImGui::Checkbox("io.ConfigWindowsResizeFromEdges", &io.ConfigWindowsResizeFromEdges);
				ImGui::SameLine();
				HelpMarker(
					"Enable resizing of windows from their edges and from the lower-left corner.\nThis requires (io.BackendFlags & ImGuiBackendFlags_HasMouseCursors) because it needs mouse cursor feedback.");
				ImGui::Checkbox("io.ConfigWindowsMoveFromTitleBarOnly", &io.ConfigWindowsMoveFromTitleBarOnly);
				ImGui::Checkbox("io.MouseDrawCursor", &io.MouseDrawCursor);
				ImGui::SameLine();
				HelpMarker(
					"Instruct Dear ImGui to render a mouse cursor itself. Note that a mouse cursor rendered via your application GPU rendering path will feel more laggy than hardware cursor, but will be more in sync with your other visuals.\n\nSome desktop applications may use both kinds of cursors (e.g. enable software cursor only when resizing/dragging something).");
				ImGui::Text("Also see Style->Rendering for rendering options.");
				ImGui::TreePop();
				ImGui::Separator();
			}

			if (ImGui::TreeNode("Backend Flags"))
			{
				HelpMarker("Those flags are set by the backends (imgui_impl_xxx files) to specify their capabilities.\n"
						   "Here we expose then as read-only fields to avoid breaking interactions with your backend.");

				// Make a local copy to avoid modifying actual backend flags.
				ImGuiBackendFlags backend_flags = io.BackendFlags;
				ImGui::CheckboxFlags("io.BackendFlags: HasGamepad", &backend_flags, ImGuiBackendFlags_HasGamepad);
				ImGui::CheckboxFlags("io.BackendFlags: HasMouseCursors", &backend_flags,
									 ImGuiBackendFlags_HasMouseCursors);
				ImGui::CheckboxFlags("io.BackendFlags: HasSetMousePos", &backend_flags,
									 ImGuiBackendFlags_HasSetMousePos);
				ImGui::CheckboxFlags("io.BackendFlags: PlatformHasViewports", &backend_flags,
									 ImGuiBackendFlags_PlatformHasViewports);
				ImGui::CheckboxFlags("io.BackendFlags: HasMouseHoveredViewport", &backend_flags,
									 ImGuiBackendFlags_HasMouseHoveredViewport);
				ImGui::CheckboxFlags("io.BackendFlags: RendererHasVtxOffset", &backend_flags,
									 ImGuiBackendFlags_RendererHasVtxOffset);
				ImGui::CheckboxFlags("io.BackendFlags: RendererHasViewports", &backend_flags,
									 ImGuiBackendFlags_RendererHasViewports);
				ImGui::TreePop();
				ImGui::Separator();
			}

			if (ImGui::TreeNode("Style"))
			{
				HelpMarker(
					"The same contents can be accessed in 'Tools->Style Editor' or by calling the ShowStyleEditor() function.");
				ImGui::ShowStyleEditor();
				ImGui::TreePop();
				ImGui::Separator();
			}

			if (ImGui::TreeNode("Capture/Logging"))
			{
				HelpMarker(
					"The logging API redirects all text output so you can easily capture the content of "
					"a window or a block. Tree nodes can be automatically expanded.\n"
					"Try opening any of the contents below in this window and then click one of the \"Log To\" button.");
				ImGui::LogButtons();

				HelpMarker("You can also call ImGui::LogText() to output directly to the log without a visual output.");
				if (ImGui::Button("Copy \"Hello, world!\" to clipboard"))
				{
					ImGui::LogToClipboard();
					ImGui::LogText("Hello, world!");
					ImGui::LogFinish();
				}
				ImGui::TreePop();
			}
		}

		if (ImGui::CollapsingHeader("Window options"))
		{
			if (ImGui::BeginTable("split", 3))
			{
				ImGui::TableNextColumn();
				ImGui::Checkbox("No titlebar", &no_titlebar);
				ImGui::TableNextColumn();
				ImGui::Checkbox("No scrollbar", &no_scrollbar);
				ImGui::TableNextColumn();
				ImGui::Checkbox("No menu", &no_menu);
				ImGui::TableNextColumn();
				ImGui::Checkbox("No move", &no_move);
				ImGui::TableNextColumn();
				ImGui::Checkbox("No resize", &no_resize);
				ImGui::TableNextColumn();
				ImGui::Checkbox("No collapse", &no_collapse);
				ImGui::TableNextColumn();
				ImGui::Checkbox("No close", &no_close);
				ImGui::TableNextColumn();
				ImGui::Checkbox("No nav", &no_nav);
				ImGui::TableNextColumn();
				ImGui::Checkbox("No background", &no_background);
				ImGui::TableNextColumn();
				ImGui::Checkbox("No bring to front", &no_bring_to_front);
				ImGui::TableNextColumn();
				ImGui::Checkbox("No docking", &no_docking);
				ImGui::EndTable();
			}
		}

		// End of ShowDemoWindow()
		ImGui::PopItemWidth();
		ImGui::End();
	}

private:
	std::shared_ptr<utils::RingBufferSinkSt> _sink{};
	bool open = true;
	emu::Chip8 _emulator{};
	ImGuiTextFilter filter{};
};
__STOP_IGNORING_WARNINGS__

int main(int /*argc*/, char** /*argv*/)
{
	mahi::gui::Application::Config config;
	config.fullscreen = false;
	config.msaa = 0;
	config.nvg_aa = false;
	config.monitor = 1;
	config.title = "Chip8 Emulator";
	config.background = mahi::gui::Colors::Auto;
	ChipEightEmulator app(config, RegisterRingBufferSink());
	app.run();
	return 0;
}
