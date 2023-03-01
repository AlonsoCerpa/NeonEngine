#include "rendering.h"

#include "camera.h"
#include "shader.h"
#include "model.h"
#include "user_interface.h"

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
    ourModel = nullptr;
    camera_viewport = new Camera((glm::vec3(0.0f, 0.0f, 3.0f)));
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
}

void Rendering::render_viewport() {
    int viewport_width = user_interface->viewport_width;
    int viewport_height = user_interface->viewport_height;
    glViewport(0, 0, viewport_width, viewport_height);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glClearColor(1.0f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now

    ourShader->use();

    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(camera_viewport->Zoom), (float)viewport_width / (float)viewport_height, 0.1f, 100.0f);
    glm::mat4 view = camera_viewport->GetViewMatrix();
    ourShader->setMat4("projection", projection);
    ourShader->setMat4("view", view);

    // render the loaded model
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
    ourShader->setMat4("model", model);
    ourModel->Draw(*ourShader);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Rendering::set_viewport_shaders() {
    // build and compile shaders
    ourShader = new Shader("src/model_loading.vert", "src/model_loading.frag");
}

void Rendering::set_viewport_models() {
    // load models
    ourModel = new Model("models/backpack/backpack.obj");
}

void Rendering::create_and_set_viewport_framebuffer() {
    int viewport_width = user_interface->viewport_width;
    int viewport_height = user_interface->viewport_height;
    create_and_set_framebuffer(&framebuffer, &textureColorbuffer, &rboDepthStencil, viewport_width, viewport_height);
}

void Rendering::clean() {
    delete ourShader;
    delete ourModel;
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