#include "neon_engine.h"

#include "user_interface.h"
#include "input.h"
#include "camera.h"
#include "viewport.h"

// Includes to load and draw a model
#include "shader.h"
#include "model.h"
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <mutex>
#include <iostream>

NeonEngine* NeonEngine::instance = nullptr;
std::mutex NeonEngine::neon_engine_mutex;

NeonEngine::NeonEngine() {
    window = nullptr;
    user_interface = nullptr;
    input = nullptr;
    glfw_major_version = 3;
    glfw_minor_version = 3;
    glsl_version = "#version 330";
    window_title = "Neon Engine";
    window_width = 1920;
    window_height = 1080;
    clear_color = ImVec4(1.0f, 0.1f, 0.1f, 1.0f);

    camera_viewport = Camera((glm::vec3(0.0f, 0.0f, 3.0f)));
    viewport_width = 800;
    viewport_height = 600;
    deltaTime = 0.0f;
    lastFrame = 0.0f;
}

NeonEngine::~NeonEngine() {

}

void NeonEngine::initialize_all_components() {
    user_interface = UserInterface::get_instance();
    input = Input::get_instance();

    user_interface->initialize();
    input->initialize();
}

NeonEngine* NeonEngine::get_instance()
{
    std::lock_guard<std::mutex> lock(neon_engine_mutex);
    if (instance == nullptr) {
        instance = new NeonEngine();
    }
    return instance;
}

int NeonEngine::setup_glfw() {
    glfwSetErrorCallback(UserInterface::glfw_error_callback);
    if (!glfwInit())
        return 1;

    // GL 3.0 + GLSL 130
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, glfw_major_version);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, glfw_minor_version);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Create window with graphics context
    window = glfwCreateWindow(window_width, window_height, window_title, NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, Input::framebuffer_size_callback);
    glfwSetCursorPosCallback(window, Input::mouse_callback);
    glfwSetScrollCallback(window, Input::scroll_callback);

    // tell GLFW to capture our mouse
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSwapInterval(1); // Enable vsync
    return 0;
}

int NeonEngine::setup_glad() {
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    return 0;
}

void NeonEngine::clean_gflw() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

int NeonEngine::run() {
    initialize_all_components();
    if (setup_glfw() != 0) {
        return -1;
    }

    if (setup_glad() != 0) {
        return -1;
    }
    
    user_interface->setup_imgui();


    // Our state
    ImGuiIO& io = ImGui::GetIO();


    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    Shader ourShader("src/model_loading.vert", "src/model_loading.frag");

    // load models
    // -----------
    Model ourModel("models/backpack/backpack.obj");

    /*
    const char* vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "}\0";
    const char* fragmentShaderSource = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "   FragColor = vec4(0.1f, 0.1f, 1.0f, 1.0f);\n"
        "}\n\0";

    float vertex_data[] = {
        -0.5f, -0.5f, 0.0f, // left  
         0.5f, -0.5f, 0.0f, // right 
         0.0f,  0.5f, 0.0f  // top   
    };

    int size_vertex_data = sizeof(vertex_data);

    unsigned int shaderProgram = compile_shaders(vertexShaderSource, fragmentShaderSource);
    unsigned int VAO = create_and_set_vao(vertex_data, size_vertex_data);*/
    unsigned int framebuffer, textureColorbuffer;
    render_to_framebuffer(&framebuffer, &textureColorbuffer);

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        input->processInput();

        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        /*
        glViewport(0, 0, 800, 600);

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

        glClearColor(1.0f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);*/

        glViewport(0, 0, viewport_width, viewport_height);

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

        glClearColor(1.0f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now

        ourShader.use();

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera_viewport.Zoom), (float)viewport_width / (float)viewport_height, 0.1f, 100.0f);
        glm::mat4 view = camera_viewport.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        // render the loaded model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", model);
        ourModel.Draw(ourShader);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        user_interface->render_ui();

        ImGui::Begin("Aux viewport");
        ImVec2 pos1 = ImGui::GetCursorScreenPos();
        ImVec2 pos2(pos1.x + 800, pos1.y + 600);
        //ImVec2 pos2(pos1.x + window_width / 2.0f, pos1.y + window_height / 2);
        ImGui::GetWindowDrawList()->AddImage((void*)textureColorbuffer, pos1, pos2, ImVec2(0, 1), ImVec2(1, 0));
        ImGui::Text("This is some useful text.");
        ImGui::End();



        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
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

        glfwSwapBuffers(window);
    }

    // Cleanup
    user_interface->clean_imgui();

    clean_gflw();


    return 0;
}