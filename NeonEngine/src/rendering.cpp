#include "rendering.h"

#include "camera.h"
#include "shader.h"
#include "complex_model.h"
#include "user_interface.h"
#include "neon_engine.h"
#include "cylinder.h"
#include "game_object.h"
#include "opengl_utils.h"

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
    key_selected_object = -1;
    key_generator = new KeyGenerator(1000000);
    highlight_color = glm::vec3(255.0f/255.0f, 195.0f/255.0f, 7.0f/255.0f);
}

Rendering::~Rendering() {
    // Delete all the data in the "clean" member function
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
        int key_intersected_object = -1;
        float min_t = std::numeric_limits<float>::max();
        for (auto it = game_objects.begin(); it != game_objects.end(); it++) {
            GameObject* game_object = it->second;
            if (game_object->intersected_ray(this, ray_dir, camera_viewport->Position, t)) {
                if (t < min_t) {
                    min_t = t;
                    key_intersected_object = it->first;
                }
            }
        }
        if (key_intersected_object != -1) {
            return key_intersected_object;
        }
        else {
            return -1;
        }
    }
    return -1;
}

void Rendering::initialize_game_objects() {
    GameObject* backpack1 = new GameObject(0, glm::vec3(0.0f, 0.0f, -5.0f));
    int backpack1_key = key_generator->generate_key();
    game_objects[backpack1_key] = backpack1;

    GameObject* backpack2 = new GameObject(0, glm::vec3(6.0f, 4.0f, -15.0f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(60.0f, 0.0f, 0.0f));
    int backpack2_key = key_generator->generate_key();
    game_objects[backpack2_key] = backpack2;

    GameObject* cylinder1 = new GameObject(1, glm::vec3(3.0f, 7.0f, -6.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(45.0f, 60.0f, 20.0f), glm::vec3(1.0f, 1.0f, 0.0f));
    int cylinder1_key = key_generator->generate_key();
    game_objects[cylinder1_key] = cylinder1;

    GameObject* cone1 = new GameObject(2, glm::vec3(-3.0f, 7.0f, -6.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    int cone1_key = key_generator->generate_key();
    game_objects[cone1_key] = cone1;

    GameObject* cylinder2 = new GameObject(1, glm::vec3(-3.0f, -7.0f, -6.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    int cylinder2_key = key_generator->generate_key();
    game_objects[cylinder2_key] = cylinder2;



    GameObject* coord_axes_3D = new GameObject(-1, glm::vec3(0.0f, -3.0f, -3.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    int coord_axes_3D_key = key_generator->generate_key();
    game_objects[coord_axes_3D_key] = coord_axes_3D;

    GameObject* x_arrow_body = new GameObject(1, glm::vec3(0.0f, 0.0f, 1.5f), glm::vec3(0.1f, 0.1f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    x_arrow_body->set_model_matrices_type1(coord_axes_3D);
    coord_axes_3D->children_game_objects.push_back(x_arrow_body);
    
    GameObject* y_arrow_body = new GameObject(1, glm::vec3(0.0f, 1.5f, 0.0f), glm::vec3(0.1f, 0.1f, 3.0f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    y_arrow_body->set_model_matrices_type1(coord_axes_3D);
    coord_axes_3D->children_game_objects.push_back(y_arrow_body);

    GameObject* z_arrow_body = new GameObject(1, glm::vec3(1.5f, 0.0f, 0.0f), glm::vec3(0.1f, 0.1f, 3.0f), glm::vec3(0.0f, 90.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    z_arrow_body->set_model_matrices_type1(coord_axes_3D);
    coord_axes_3D->children_game_objects.push_back(z_arrow_body);

    GameObject* x_arrow_head = new GameObject(2, glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.3f, 0.3f, 0.6f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    x_arrow_head->set_model_matrices_type1(coord_axes_3D);
    coord_axes_3D->children_game_objects.push_back(x_arrow_head);

    GameObject* y_arrow_head = new GameObject(2, glm::vec3(0.0f, 3.0f, 0.0f), glm::vec3(0.3f, 0.3f, 0.6f), glm::vec3(-90.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    y_arrow_head->set_model_matrices_type1(coord_axes_3D);
    coord_axes_3D->children_game_objects.push_back(y_arrow_head);

    GameObject* z_arrow_head = new GameObject(2, glm::vec3(3.0f, 0.0f, 0.0f), glm::vec3(0.3f, 0.3f, 0.6f), glm::vec3(0.0f, 90.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    z_arrow_head->set_model_matrices_type1(coord_axes_3D);
    coord_axes_3D->children_game_objects.push_back(z_arrow_head);



    GameObject* point_light1 = new PointLight(1, glm::vec3(1.0f, 1.0f, -3.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), false,
        glm::vec3(0.05f, 0.05f, 0.05f), glm::vec3(0.8f, 0.8f, 0.8f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, 0.045f, 0.0075f);
    int point_light1_key = key_generator->generate_key();
    game_objects[point_light1_key] = point_light1;
    point_lights[point_light1_key] = (PointLight*)point_light1;


    GameObject* directional_light1 = new DirectionalLight(1, glm::vec3(-5.0f, 5.0f, -3.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), false,
        glm::vec3(0.05f, 0.05f, 0.05f), glm::vec3(0.4f, 0.4f, 0.4f), glm::vec3(0.8f, 0.8f, 0.8f), glm::vec3(3.0f, -4.0f, -3.0f));
    int directional_light1_key = key_generator->generate_key();
    game_objects[directional_light1_key] = directional_light1;
    directional_lights[directional_light1_key] = (DirectionalLight*)directional_light1;


    GameObject* spot_light1 = new SpotLight(1, glm::vec3(-5.0f, -5.0f, -3.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), false,
        glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(5.0f, 5.0f, -3.0f),
        glm::cos(glm::radians(12.5f)), glm::cos(glm::radians(15.0f)), 1.0f, 0.09f, 0.032f);
    int spot_light1_key = key_generator->generate_key();
    game_objects[spot_light1_key] = spot_light1;
    spot_lights[spot_light1_key] = (SpotLight*)spot_light1;
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

    ourShader->setVec3("viewPos", camera_viewport->Position);

    ourShader->setInt("num_point_lights", point_lights.size());
    ourShader->setInt("num_directional_lights", directional_lights.size());
    ourShader->setInt("num_spot_lights", spot_lights.size());

    int idx_point_light = 0;
    for (auto it = point_lights.begin(); it != point_lights.end(); it++, idx_point_light++) {
        ourShader->setVec3("pointLights[" + std::to_string(idx_point_light) + "].position", it->second->position);
        ourShader->setVec3("pointLights[" + std::to_string(idx_point_light) + "].ambient", it->second->ambient);
        ourShader->setVec3("pointLights[" + std::to_string(idx_point_light) + "].diffuse", it->second->diffuse);
        ourShader->setVec3("pointLights[" + std::to_string(idx_point_light) + "].specular", it->second->specular);
        ourShader->setFloat("pointLights[" + std::to_string(idx_point_light) + "].constant", it->second->constant);
        ourShader->setFloat("pointLights[" + std::to_string(idx_point_light) + "].linear", it->second->linear);
        ourShader->setFloat("pointLights[" + std::to_string(idx_point_light) + "].quadratic", it->second->quadratic);
    }

    int idx_directional_light = 0;
    for (auto it = directional_lights.begin(); it != directional_lights.end(); it++, idx_directional_light++) {
        ourShader->setVec3("directionalLights[" + std::to_string(idx_directional_light) + "].direction", it->second->direction);
        ourShader->setVec3("directionalLights[" + std::to_string(idx_directional_light) + "].ambient", it->second->ambient);
        ourShader->setVec3("directionalLights[" + std::to_string(idx_directional_light) + "].diffuse", it->second->diffuse);
        ourShader->setVec3("directionalLights[" + std::to_string(idx_directional_light) + "].specular", it->second->specular);
    }

    int idx_spot_light = 0;
    for (auto it = spot_lights.begin(); it != spot_lights.end(); it++, idx_spot_light++) {
        ourShader->setVec3("spotLights[" + std::to_string(idx_spot_light) + "].position", it->second->position);
        ourShader->setVec3("spotLights[" + std::to_string(idx_spot_light) + "].direction", it->second->direction);
        ourShader->setVec3("spotLights[" + std::to_string(idx_spot_light) + "].ambient", it->second->ambient);
        ourShader->setVec3("spotLights[" + std::to_string(idx_spot_light) + "].diffuse", it->second->diffuse);
        ourShader->setVec3("spotLights[" + std::to_string(idx_spot_light) + "].specular", it->second->specular);
        ourShader->setFloat("spotLights[" + std::to_string(idx_spot_light) + "].constant", it->second->constant);
        ourShader->setFloat("spotLights[" + std::to_string(idx_spot_light) + "].linear", it->second->linear);
        ourShader->setFloat("spotLights[" + std::to_string(idx_spot_light) + "].quadratic", it->second->quadratic);
        ourShader->setFloat("spotLights[" + std::to_string(idx_spot_light) + "].inner_cut_off", it->second->inner_cut_off);
        ourShader->setFloat("spotLights[" + std::to_string(idx_spot_light) + "].outer_cut_off", it->second->outer_cut_off);
    }

    // render all the game objects
    for (auto it = game_objects.begin(); it != game_objects.end(); it++) {
        GameObject* game_object = it->second;
        game_object->draw(ourShader, this);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Rendering::set_viewport_shaders() {
    // build and compile shaders
    ourShader = new Shader("shaders/render_model.vert", "shaders/render_model.frag");
}

void Rendering::set_viewport_models() {
    // load complex models
    loaded_models.push_back(new ComplexModel("models/backpack/backpack.obj"));

    // load basic shapes
    //loaded_models.push_back(new Cylinder(0.5f, 1.0f, 30, 1));
    //loaded_models.push_back(new Cone(0.5f, 1.0f, 30, 1));

    // Cylinder
    loaded_models.push_back(new Cylinder(1.0f, 1.0f, 1.0f, 36, 1, true, 3));

    // Cone
    loaded_models.push_back(new Cylinder(1.0f, 0.0f, 1.0f, 36, 1, true, 3));
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
    for (int i = 0; i < game_objects.size(); i++) {
        delete game_objects[i];
    }
    delete camera_viewport;
    delete key_generator;
}

void Rendering::clean_viewport_framebuffer() {
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteTextures(1, &textureColorbuffer);
    glDeleteRenderbuffers(1, &rboDepthStencil);
}