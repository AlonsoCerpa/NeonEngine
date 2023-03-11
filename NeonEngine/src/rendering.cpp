#include "rendering.h"

#include "camera.h"
#include "shader.h"
#include "model.h"
#include "user_interface.h"
#include "neon_engine.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

Rendering* Rendering::instance = nullptr;
std::mutex Rendering::rendering_mutex;

Rendering::Rendering() {
    ourShader = nullptr;
    camera_viewport = new Camera((glm::vec3(0.0f, 0.0f, 3.0f)));
    near_camera_viewport = 0.1f;
    far_camera_viewport = 100.0f;
    idx_selected_object = -1;
}

Rendering::~Rendering() {

}

Rendering* Rendering::get_instance()
{
    std::lock_guard<std::mutex> lock(rendering_mutex);
    if (instance == nullptr) {
        instance = new Rendering();
    }
    return instance;
}

void Rendering::initialize() {
    user_interface = UserInterface::get_instance();
    neon_engine = NeonEngine::get_instance();
}

int Rendering::check_mouse_over_models() {
    int texture_viewport_width = user_interface->texture_viewport_width;
    int texture_viewport_height = user_interface->texture_viewport_height;
    view_projection = projection * view;
    view_projection_inv = glm::inverse(view_projection);
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 mouse_pos = io.MousePos;
    ImVec2 window_pos = ImGui::GetWindowPos();
    ImVec2 mouse_pos_in_window = ImVec2(mouse_pos.x - window_pos.x, mouse_pos.y - window_pos.y);
    ImVec2& viewport_texture_pos = user_interface->viewport_texture_pos;
    ImVec2 mouse_pos_in_viewport_texture = ImVec2(mouse_pos_in_window.x - viewport_texture_pos.x, mouse_pos_in_window.y - viewport_texture_pos.y);
    if (mouse_pos_in_viewport_texture.x >= 0 && mouse_pos_in_viewport_texture.y >= 0 &&
        mouse_pos_in_viewport_texture.x <= texture_viewport_width && mouse_pos_in_viewport_texture.y <= texture_viewport_height) {
        float x_viewport = mouse_pos_in_viewport_texture.x * (2.0f / texture_viewport_width) - 1;
        float y_viewport = -mouse_pos_in_viewport_texture.y * (2.0f / texture_viewport_height) + 1;
        glm::vec4 mouse_viewport(x_viewport, y_viewport, -1.0f, 1.0f); // in clip space with near plane depth
        glm::vec4 mouse_world = view_projection_inv * mouse_viewport;
        mouse_world /= mouse_world.w;
        glm::vec3 ray_dir = glm::normalize(glm::vec3(mouse_world) - camera_viewport->Position);

        float t;
        int idx_intersected_object = -1;
        float min_t = std::numeric_limits<float>::max();
        for (int i = 0; i < game_objects.size(); i++) {
            GameObject& game_object = game_objects[i];
            glm::vec3 ray_dir_model = game_object.model_inv * glm::vec4(ray_dir, 0.0f);
            glm::vec3 ray_origin_model = game_object.model_inv * glm::vec4(camera_viewport->Position, 1.0f);
            if (loaded_models[game_object.idx_loaded_models]->intersected_ray(ray_origin_model, ray_dir_model, t)) {
                if (t < min_t) {
                    min_t = t;
                    idx_intersected_object = i;
                }
            }
        }
        if (idx_intersected_object != -1) {
            return idx_intersected_object;
        }
        else {
            return -1;
        }
    }
    return -1;
}

void Rendering::initialize_game_objects() {
    GameObject backpack1;
    backpack1.idx_loaded_models = 0;
    backpack1.position = glm::vec3(0.0f, 0.0f, -5.0f);
    backpack1.scale = glm::vec3(1.0f, 1.0f, 1.0f);
    backpack1.axis_rotation = glm::vec3(1.0f, 0.0f, 0.0f);;
    backpack1.angle_rotation_degrees = 0.0f;
    backpack1.model = glm::mat4(1.0f);
    backpack1.model = glm::translate(backpack1.model, backpack1.position);
    backpack1.model = glm::rotate(backpack1.model, glm::radians(backpack1.angle_rotation_degrees), backpack1.axis_rotation);
    backpack1.model = glm::scale(backpack1.model, backpack1.scale);
    backpack1.model_inv = glm::inverse(backpack1.model);
    backpack1.is_selected = false;
    game_objects.push_back(backpack1);

    GameObject backpack2;
    backpack2.idx_loaded_models = 0;
    backpack2.position = glm::vec3(6.0f, 4.0f, -15.0f);
    backpack2.scale = glm::vec3(0.5f, 0.5f, 0.5f);
    backpack2.axis_rotation = glm::vec3(1.0f, 0.0f, 0.0f);;
    backpack2.angle_rotation_degrees = 60.0f;
    backpack2.model = glm::mat4(1.0f);
    backpack2.model = glm::translate(backpack2.model, backpack2.position);
    backpack2.model = glm::rotate(backpack2.model, glm::radians(backpack2.angle_rotation_degrees), backpack2.axis_rotation);
    backpack2.model = glm::scale(backpack2.model, backpack2.scale);
    backpack2.model_inv = glm::inverse(backpack2.model);
    backpack2.is_selected = false;
    game_objects.push_back(backpack2);
}

void Rendering::set_opengl_state() {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
}

void Rendering::render_viewport() {
    int texture_viewport_width = user_interface->texture_viewport_width;
    int texture_viewport_height = user_interface->texture_viewport_height;
    glViewport(0, 0, texture_viewport_width, texture_viewport_height);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glClearColor(1.0f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // view/projection transformations
    projection = glm::perspective(glm::radians(camera_viewport->Zoom), (float)texture_viewport_width / (float)texture_viewport_height, near_camera_viewport, far_camera_viewport);
    view = camera_viewport->GetViewMatrix();
    ourShader->setMat4("projection", projection);
    ourShader->setMat4("view", view);
    // render all the game objects
    for (int i = 0; i < game_objects.size(); i++) {
        GameObject& game_object = game_objects[i];
        ourShader->setMat4("model", game_object.model);
        loaded_models[game_object.idx_loaded_models]->Draw(*ourShader, game_object.is_selected);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Rendering::set_viewport_shaders() {
    // build and compile shaders
    ourShader = new Shader("shaders/render_model.vert", "shaders/render_model.frag");
}

void Rendering::set_viewport_models() {
    // load models
    loaded_models.push_back(new Model("models/backpack/backpack.obj"));
}

void Rendering::create_and_set_viewport_framebuffer() {
    int texture_viewport_width = user_interface->texture_viewport_width;
    int texture_viewport_height = user_interface->texture_viewport_height;
    create_and_set_framebuffer(&framebuffer, &textureColorbuffer, &rboDepthStencil, texture_viewport_width, texture_viewport_height);
}

void Rendering::clean() {
    delete ourShader;
    for (int i = 0; i < loaded_models.size(); i++) {
        delete loaded_models[i];
    }
    delete camera_viewport;
}

void Rendering::clean_viewport_framebuffer() {
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteTextures(1, &textureColorbuffer);
    glDeleteRenderbuffers(1, &rboDepthStencil);
}

unsigned int compile_shaders(const char* vertexShaderSource, const char* fragmentShaderSource) {
    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

unsigned int create_and_set_vao(float* vertex_data, int size_vertex_data) {
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, size_vertex_data, vertex_data, GL_STATIC_DRAW);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(float)*9, vertex_data, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);

    return VAO;
}

void create_and_set_framebuffer(unsigned int* framebuffer, unsigned int* textureColorbuffer, unsigned int* rboDepthStencil, int width, int height) {
    int color_texture_width = width;
    int color_texture_height = height;
    int depth_stencil_rbo_width = width;
    int depth_stencil_rbo_height = height;

    // Create and bind Framebuffer
    glGenFramebuffers(1, framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, *framebuffer);

    // Create and set Color Texture
    glGenTextures(1, textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, *textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, color_texture_width, color_texture_height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Attach the Texture to the currently bound Framebuffer object
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *textureColorbuffer, 0);

    // Create and set Depth and Stencil Renderbuffer
    glGenRenderbuffers(1, rboDepthStencil);
    glBindRenderbuffer(GL_RENDERBUFFER, *rboDepthStencil);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, depth_stencil_rbo_width, depth_stencil_rbo_height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // Attach the Renderbuffer to the currently bound Gramebuffer object
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, *rboDepthStencil);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

    // Bind to the default Framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}