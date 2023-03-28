#include "rendering.h"

#include "camera.h"
#include "shader.h"
#include "complex_model.h"
#include "user_interface.h"
#include "neon_engine.h"
#include "cylinder.h"
#include "game_object.h"
#include "opengl_utils.h"
#include "transform3d.h"
#include "quad.h"

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
    phong_shader = nullptr;
    camera_viewport = new Camera((glm::vec3(0.0f, 0.0f, 3.0f)));
    near_camera_viewport = 0.1f;
    far_camera_viewport = 100.0f;
    key_selected_object = "";
    key_generator = new KeyGenerator(1000000);
    transform3d = new Transform3D();
    outline_color = glm::vec3(255.0f/255.0f, 195.0f/255.0f, 7.0f/255.0f);
    screen_quad = nullptr;
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

void Rendering::initialize_game_objects() {
    GameObject* backpack1 = new GameObject("backpack1", "backpack", glm::vec3(0.0f, 0.0f, -5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(102.0f/255.0f, 0.0f/255.0f, 204.0f/255.0f), false, false, true);
    game_objects[backpack1->name] = backpack1;

    GameObject* backpack2 = new GameObject("backpack2", "backpack", glm::vec3(3.0f, 2.0f, -6.0f), glm::vec3(60.0f, 0.0f, 0.0f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f));
    game_objects[backpack2->name] = backpack2;

    GameObject* backpack3 = new GameObject("backpack3", "backpack", glm::vec3(-3.0f, 2.0f, -6.0f), glm::vec3(10.0f, 30.0f, 45.0f), glm::vec3(0.8f, 0.8f, 0.8f), glm::vec3(0.0f, 1.0f, 0.0f));
    game_objects[backpack3->name] = backpack3;

    GameObject* cylinder1 = new GameObject("cylinder1", "cylinder", glm::vec3(3.0f, 7.0f, -6.0f), glm::vec3(45.0f, 60.0f, 20.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 0.0f), false, false, true);
    game_objects[cylinder1->name] = cylinder1;

    GameObject* cone1 = new GameObject("cone1", "cone", glm::vec3(-3.0f, 7.0f, -6.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), false, false, true);
    game_objects[cone1->name] = cone1;

    GameObject* cylinder2 = new GameObject("cylinder2", "cylinder", glm::vec3(-3.0f, -7.0f, -6.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), false, false, true);
    game_objects[cylinder2->name] = cylinder2;



    GameObject* point_light1 = new PointLight("point_light1", "cylinder", glm::vec3(3.0f, 3.0f, -3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), false, false, true,
        glm::vec3(0.05f, 0.05f, 0.05f), glm::vec3(0.8f, 0.8f, 0.8f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, 0.045f, 0.0075f);
    game_objects[point_light1->name] = point_light1;
    point_lights[point_light1->name] = (PointLight*)point_light1;

    GameObject* directional_light1 = new DirectionalLight("directional_light1", "cylinder", glm::vec3(-5.0f, 5.0f, -3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), false, false, true,
        glm::vec3(0.05f, 0.05f, 0.05f), glm::vec3(0.4f, 0.4f, 0.4f), glm::vec3(0.8f, 0.8f, 0.8f), glm::vec3(3.0f, -4.0f, -3.0f));
    game_objects[directional_light1->name] = directional_light1;
    directional_lights[directional_light1->name] = (DirectionalLight*)directional_light1;

    GameObject* spot_light1 = new SpotLight("spot_light1", "cylinder", glm::vec3(-5.0f, -5.0f, -3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), false, false, true,
        glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(5.0f, 5.0f, -3.0f),
        glm::cos(glm::radians(12.5f)), glm::cos(glm::radians(15.0f)), 1.0f, 0.09f, 0.032f);
    game_objects[spot_light1->name] = spot_light1;
    spot_lights[spot_light1->name] = (SpotLight*)spot_light1;
}

void Rendering::set_opengl_state() {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Rendering::set_viewport_shaders() {
    // build and compile shaders
    phong_shader = new Shader("shaders/vertices_3d_model.vert", "shaders/phong_lighting.frag");
    outline_shader = new Shader("shaders/vertices_quad.vert", "shaders/edge_outlining.frag");
}

void Rendering::set_viewport_models() {
    // Screen Quad
    screen_quad = new Quad();

    // Backpack
    Model* backpack = new ComplexModel("backpack", "models/backpack/backpack.obj");
    loaded_models[backpack->name] = backpack;

    // Cylinder
    Model* cylinder = new Cylinder("cylinder", 1.0f, 1.0f, 1.0f, 36, 1, true, 3);
    loaded_models[cylinder->name] = cylinder;

    // Cone
    Model* cone = new Cylinder("cone", 1.0f, 0.0f, 1.0f, 36, 1, true, 3);
    loaded_models[cone->name] = cone;
}

void Rendering::create_and_set_viewport_framebuffer() {
    int texture_viewport_width = user_interface->texture_viewport_width;
    int texture_viewport_height = user_interface->texture_viewport_height;
    create_and_set_framebuffer(&framebuffer, &textureColorbuffer, &texture_selected_color_buffer, &rboDepthStencil, texture_viewport_width, texture_viewport_height);
}

void Rendering::render_viewport() {
    int texture_viewport_width = user_interface->texture_viewport_width;
    int texture_viewport_height = user_interface->texture_viewport_height;
    glViewport(0, 0, texture_viewport_width, texture_viewport_height);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);


    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    unsigned int attachments1[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments1);


    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // view/projection transformations
    projection = glm::perspective(glm::radians(camera_viewport->Zoom), (float)texture_viewport_width / (float)texture_viewport_height, near_camera_viewport, far_camera_viewport);
    view = camera_viewport->GetViewMatrix();

    phong_shader->use();

    phong_shader->setVec3("viewPos", camera_viewport->Position);

    phong_shader->setInt("num_point_lights", point_lights.size());
    phong_shader->setInt("num_directional_lights", directional_lights.size());
    phong_shader->setInt("num_spot_lights", spot_lights.size());

    int idx_point_light = 0;
    for (auto it = point_lights.begin(); it != point_lights.end(); it++, idx_point_light++) {
        phong_shader->setVec3("pointLights[" + std::to_string(idx_point_light) + "].position", it->second->position);
        phong_shader->setVec3("pointLights[" + std::to_string(idx_point_light) + "].ambient", it->second->ambient);
        phong_shader->setVec3("pointLights[" + std::to_string(idx_point_light) + "].diffuse", it->second->diffuse);
        phong_shader->setVec3("pointLights[" + std::to_string(idx_point_light) + "].specular", it->second->specular);
        phong_shader->setFloat("pointLights[" + std::to_string(idx_point_light) + "].constant", it->second->constant);
        phong_shader->setFloat("pointLights[" + std::to_string(idx_point_light) + "].linear", it->second->linear);
        phong_shader->setFloat("pointLights[" + std::to_string(idx_point_light) + "].quadratic", it->second->quadratic);
    }

    int idx_directional_light = 0;
    for (auto it = directional_lights.begin(); it != directional_lights.end(); it++, idx_directional_light++) {
        phong_shader->setVec3("directionalLights[" + std::to_string(idx_directional_light) + "].direction", it->second->direction);
        phong_shader->setVec3("directionalLights[" + std::to_string(idx_directional_light) + "].ambient", it->second->ambient);
        phong_shader->setVec3("directionalLights[" + std::to_string(idx_directional_light) + "].diffuse", it->second->diffuse);
        phong_shader->setVec3("directionalLights[" + std::to_string(idx_directional_light) + "].specular", it->second->specular);
    }

    int idx_spot_light = 0;
    for (auto it = spot_lights.begin(); it != spot_lights.end(); it++, idx_spot_light++) {
        phong_shader->setVec3("spotLights[" + std::to_string(idx_spot_light) + "].position", it->second->position);
        phong_shader->setVec3("spotLights[" + std::to_string(idx_spot_light) + "].direction", it->second->direction);
        phong_shader->setVec3("spotLights[" + std::to_string(idx_spot_light) + "].ambient", it->second->ambient);
        phong_shader->setVec3("spotLights[" + std::to_string(idx_spot_light) + "].diffuse", it->second->diffuse);
        phong_shader->setVec3("spotLights[" + std::to_string(idx_spot_light) + "].specular", it->second->specular);
        phong_shader->setFloat("spotLights[" + std::to_string(idx_spot_light) + "].constant", it->second->constant);
        phong_shader->setFloat("spotLights[" + std::to_string(idx_spot_light) + "].linear", it->second->linear);
        phong_shader->setFloat("spotLights[" + std::to_string(idx_spot_light) + "].quadratic", it->second->quadratic);
        phong_shader->setFloat("spotLights[" + std::to_string(idx_spot_light) + "].inner_cut_off", it->second->inner_cut_off);
        phong_shader->setFloat("spotLights[" + std::to_string(idx_spot_light) + "].outer_cut_off", it->second->outer_cut_off);
    }

    // render all the game objects
    for (auto it = game_objects.begin(); it != game_objects.end(); it++) {
        GameObject* game_object = it->second;
        game_object->draw(phong_shader, this, false);
    }
    if (key_selected_object != "") {
        transform3d->update_model_matrices(this, game_objects[key_selected_object]);
        transform3d->draw(phong_shader, this);
    }


    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    unsigned int attachments2[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, attachments2);


    outline_shader->use();
    outline_shader->setVec2("pixel_size", glm::vec2(1.0f / user_interface->texture_viewport_width, 1.0f / user_interface->texture_viewport_height));
    outline_shader->setVec3("outline_color", outline_color);
    glDisable(GL_DEPTH_TEST);
    screen_quad->draw(outline_shader, texture_selected_color_buffer);
    glEnable(GL_DEPTH_TEST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

std::string Rendering::check_mouse_over_models() {
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
        std::string key_intersected_object = "";
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
        if (key_intersected_object != "") {
            return key_intersected_object;
        }
        else {
            return "";
        }
    }
    return "";
}

void Rendering::clean() {
    delete phong_shader;
    delete outline_shader;
    for (auto it = loaded_models.begin(); it != loaded_models.end(); it++) {
        delete it->second;
    }
    for (auto it = game_objects.begin(); it != game_objects.end(); it++) {
        delete it->second;
    }
    delete transform3d;
    delete camera_viewport;
    delete key_generator;
    delete screen_quad;
}

void Rendering::clean_viewport_framebuffer() {
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteTextures(1, &textureColorbuffer);
    glDeleteTextures(1, &texture_selected_color_buffer);
    glDeleteRenderbuffers(1, &rboDepthStencil);
}