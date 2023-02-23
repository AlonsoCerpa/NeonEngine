#include "neon_engine.h"

#include "user_interface.h"
#include "camera.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <mutex>

NeonEngine* NeonEngine::instance = nullptr;
std::mutex NeonEngine::neon_engine_mutex;

NeonEngine::NeonEngine() {
    camera_viewport = Camera((glm::vec3(0.0f, 0.0f, 3.0f)));
}

NeonEngine::~NeonEngine() {

}

void NeonEngine::initialize() {
    user_interface = UserInterface::get_instance();
    user_interface->initialize();
}

NeonEngine* NeonEngine::get_instance()
{
    std::lock_guard<std::mutex> lock(neon_engine_mutex);
    if (instance == nullptr) {
        instance = new NeonEngine();
    }
    return instance;
}



int NeonEngine::run() {
    initialize();
    if (user_interface->setup_glfw() != 0) {
        return -1;
    }

    if (user_interface->setup_glad() != 0) {
        return -1;
    }
    
    user_interface->setup_imgui();


    // Our state
    ImVec4 clear_color = user_interface->clear_color;
    ImGuiIO& io = ImGui::GetIO();

    while (!glfwWindowShouldClose(user_interface->window))
    {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();



        user_interface->render_ui();



        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(user_interface->window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(user_interface->window);
    }

    // Cleanup
    user_interface->clean_imgui();

    user_interface->clean_gflw();


    return 0;
}