#include "user_interface.h"

#include "neon_engine.h"
#include "input.h"
#include "rendering.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <mutex>
#include <iostream>

UserInterface* UserInterface::instance = nullptr;
std::mutex UserInterface::user_interface_mutex;

void UserInterface::glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

UserInterface::UserInterface() {
    neon_engine = nullptr;
    input = nullptr;
    rendering = nullptr;
    window_viewport_width = -1;
    window_viewport_height = -1;
    texture_viewport_width = -1;
    texture_viewport_height = -1;
    texture_viewport_reduce_width_px = 10;
    texture_viewport_reduce_height_px = 30;
    first_time_viewport_fbo = true;
}

UserInterface::~UserInterface() {

}

UserInterface* UserInterface::get_instance()
{
    std::lock_guard<std::mutex> lock(user_interface_mutex);
    if (instance == nullptr) {
        instance = new UserInterface();
    }
    return instance;
}

void UserInterface::initialize() {
    neon_engine = NeonEngine::get_instance();
    input = Input::get_instance();
    rendering = Rendering::get_instance();
}

void UserInterface::set_ui_style() {
    ImGuiIO& io = ImGui::GetIO();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    io.Fonts->AddFontFromFileTTF("fonts/Ubuntu-Regular.ttf", 20.0f);


    ImVec4 dark_purple      = ImVec4(70.0f / 255.0f * 0.65f, 36.0f / 255.0f * 0.65f, 90.0f / 255.0f * 0.65f, 1.0f);
    ImVec4 very_dark_purple = (glm::vec4)dark_purple * 0.2f; very_dark_purple.w = 1.0f;
    ImVec4 purple           = (glm::vec4)dark_purple * 2.0f; purple.w = 1.0f;
    ImVec4 light_purple     = (glm::vec4)dark_purple * 3.0f; light_purple.w = 1.0f;
    ImVec4 white            = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    ImVec4 dark_white       = (glm::vec4)white * 0.8f; dark_white.w = 1.0f;
    ImVec4 light_cyan       = ImVec4(16.0f / 255.0f * 0.8f, 186.0f / 255.0f * 0.8f, 233.0f / 255.0f * 0.8f, 1.0f);
    ImVec4 cyan             = (glm::vec4)light_cyan * 0.8f; cyan.w = 1.0f;
    ImVec4 dark_cyan        = (glm::vec4)light_cyan * 0.5f; dark_cyan.w = 0.5f;
    ImVec4 light_yellow     = ImVec4(200.0f / 255.0f, 165.0f / 255.0f, 30.0f / 255.0f, 1.0f);
    ImVec4 yellow           = (glm::vec4)light_yellow * 0.8f; yellow.w = 1.0f;
    ImVec4 dark_yellow      = (glm::vec4)light_yellow * 0.6f; dark_yellow.w = 1.0f;
    ImVec4 red              = ImVec4(233.0f / 255.0f, 16.0f / 255.0f, 96.0f / 255.0f, 1.0f);

    ImVec4* colors = ImGui::GetStyle().Colors;

    colors[ImGuiCol_WindowBg] = very_dark_purple;

    colors[ImGuiCol_TitleBg] = yellow;
    colors[ImGuiCol_TitleBgActive] = light_yellow;
    colors[ImGuiCol_TitleBgCollapsed] = dark_yellow;

    colors[ImGuiCol_Header] = dark_purple;
    colors[ImGuiCol_HeaderHovered] = purple;
    colors[ImGuiCol_HeaderActive] = light_purple;

    colors[ImGuiCol_Button] = dark_purple;
    colors[ImGuiCol_ButtonHovered] = purple;
    colors[ImGuiCol_ButtonActive] = light_purple;

    colors[ImGuiCol_FrameBg] = dark_purple;
    colors[ImGuiCol_FrameBgHovered] = purple;
    colors[ImGuiCol_FrameBgActive] = light_purple;

    colors[ImGuiCol_CheckMark] = light_cyan;

    colors[ImGuiCol_SliderGrab] = cyan;
    colors[ImGuiCol_SliderGrabActive] = light_cyan;

    colors[ImGuiCol_Tab] = dark_cyan;
    colors[ImGuiCol_TabHovered] = cyan;
    colors[ImGuiCol_TabActive] = light_cyan;
    colors[ImGuiCol_TabUnfocused] = dark_cyan;
    colors[ImGuiCol_TabUnfocusedActive] = light_cyan;
}

void UserInterface::setup_imgui() {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    set_ui_style();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(neon_engine->window, true);
    ImGui_ImplOpenGL3_Init(neon_engine->glsl_version);
}

void UserInterface::render_ui() {
    static bool opt_fullscreen = true;
    static bool opt_padding = false;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen)
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }
    else
    {
        dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
    }

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
    // and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    if (!opt_padding)
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace Demo", nullptr, window_flags);
    if (!opt_padding)
        ImGui::PopStyleVar();

    if (opt_fullscreen)
        ImGui::PopStyleVar(2);

    // Submit the DockSpace
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Options"))
        {
            // Disabling fullscreen would allow the window to be moved to the front of other windows,
            // which we can't undo at the moment without finer window depth/z control.
            ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
            ImGui::MenuItem("Padding", NULL, &opt_padding);
            ImGui::Separator();

            if (ImGui::MenuItem("Flag: NoSplit", "", (dockspace_flags & ImGuiDockNodeFlags_NoSplit) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoSplit; }
            if (ImGui::MenuItem("Flag: NoResize", "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoResize; }
            if (ImGui::MenuItem("Flag: NoDockingInCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingInCentralNode) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoDockingInCentralNode; }
            if (ImGui::MenuItem("Flag: AutoHideTabBar", "", (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar; }
            if (ImGui::MenuItem("Flag: PassthruCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0, opt_fullscreen)) { dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode; }
            ImGui::Separator();
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    ImGui::Begin("Viewport");
    int new_window_viewport_width = ImGui::GetWindowWidth();
    int new_window_viewport_height = ImGui::GetWindowHeight();
    if (new_window_viewport_width != window_viewport_width || new_window_viewport_height != window_viewport_height) {
        if (!first_time_viewport_fbo) {
            rendering->clean_viewport_framebuffer();
        }
        else {
            first_time_viewport_fbo = false;
        }
        window_viewport_width = new_window_viewport_width;
        window_viewport_height = new_window_viewport_height;
        texture_viewport_width = new_window_viewport_width - texture_viewport_reduce_width_px;
        texture_viewport_height = new_window_viewport_height - texture_viewport_reduce_height_px;
        rendering->create_and_set_viewport_framebuffer();
    }
    rendering->render_viewport();

    ImVec2 pos1 = ImGui::GetCursorScreenPos();
    ImVec2 pos2(pos1.x + texture_viewport_width, pos1.y + texture_viewport_height);

    viewport_texture_pos = ImGui::GetCursorPos();
    ImGui::GetWindowDrawList()->AddImage((void*)rendering->textureColorbuffer, pos1, pos2, ImVec2(0, 1), ImVec2(1, 0));
    //ImGui::GetWindowDrawList()->AddImage((void*)rendering->texture_selected_color_buffer, pos1, pos2, ImVec2(0, 1), ImVec2(1, 0));

    input->process_viewport_input();

    ImGui::End();

    ImGui::Begin("Details");
    ImGui::Text("This is some useful text.");
    ImGui::Button("Add");
    ImGui::End();

    ImGui::Begin("Outlier");
    ImGui::Text("This is some useful text.");
    ImGui::Button("Add");
    ImGui::End();

    ImGui::Begin("Content Browser");
    ImGui::Text("This is some useful text.");
    ImGui::Button("Add");
    ImGui::End();

    ImGui::End();
}

void UserInterface::clean_imgui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}