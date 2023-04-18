#include "rendering.h"

#include "camera.h"
#include "shader.h"
#include "model.h"
#include "user_interface.h"
#include "neon_engine.h"
#include "cylinder.h"
#include "game_object.h"
#include "opengl_utils.h"
#include "transform3d.h"
#include "quad.h"
#include "cube.h"
#include "sphere.h"
#include "disk_border.h"
#include "cubemap.h"

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
    selection_shader = nullptr;
    outline_shader = nullptr;
    camera_viewport = new Camera((glm::vec3(0.0f, 0.0f, 3.0f)));
    near_camera_viewport = 0.1f;
    far_camera_viewport = 500.0f;
    last_selected_object = nullptr;
    last_selected_object_transform3d = nullptr;
    outline_color = glm::vec3(255.0f/255.0f, 195.0f/255.0f, 7.0f/255.0f);
    screen_quad = nullptr;
    cubemap = nullptr;
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
    transform3d = new Transform3D();
}

void Rendering::set_opengl_state() {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Rendering::create_and_set_viewport_framebuffer() {
    int texture_viewport_width = user_interface->texture_viewport_width;
    int texture_viewport_height = user_interface->texture_viewport_height;
    create_and_set_framebuffer(&framebuffer, &textureColorbuffer, &texture_id_colors, &texture_id_colors_transform3d, &texture_selected_color_buffer, &rboDepthStencil, texture_viewport_width, texture_viewport_height);
}

void Rendering::set_viewport_shaders() {
    // build and compile shaders
    phong_shader = new Shader("shaders/vertices_3d_model.vert", "shaders/phong_lighting.frag");
    selection_shader = new Shader("shaders/vertices_3d_model.vert", "shaders/paint_selected.frag");
    outline_shader = new Shader("shaders/vertices_quad.vert", "shaders/edge_outlining.frag");
    skybox_shader = new Shader("shaders/skybox.vert", "shaders/skybox.frag");
}

void Rendering::set_viewport_models() {
    // Cubemap
    std::vector<std::string> skybox_textures_ocean_with_sky = {
        "skyboxes/ocean_with_sky/right.jpg",
        "skyboxes/ocean_with_sky/left.jpg",
        "skyboxes/ocean_with_sky/top.jpg",
        "skyboxes/ocean_with_sky/bottom.jpg",
        "skyboxes/ocean_with_sky/front.jpg",
        "skyboxes/ocean_with_sky/back.jpg" };
    std::vector<std::string> skybox_textures_nebula = {
        "skyboxes/nebula/right.png",
        "skyboxes/nebula/left.png",
        "skyboxes/nebula/top.png",
        "skyboxes/nebula/bottom.png",
        "skyboxes/nebula/front.png",
        "skyboxes/nebula/back.png" };
    std::vector<std::string> skybox_textures_red_space = {
        "skyboxes/red_space/right.png",
        "skyboxes/red_space/left.png",
        "skyboxes/red_space/top.png",
        "skyboxes/red_space/bottom.png",
        "skyboxes/red_space/front.png",
        "skyboxes/red_space/back.png" };

    skyboxes_loaded.push_back("ocean_with_sky");
    skyboxes_loaded.push_back("nebula");
    skyboxes_loaded.push_back("red_space");

    cubemap = new Cubemap(skyboxes_loaded[0], skybox_textures_ocean_with_sky);
    cubemap->add_cubemap_texture(skyboxes_loaded[1], skybox_textures_nebula);
    cubemap->add_cubemap_texture(skyboxes_loaded[2], skybox_textures_red_space);

    // Screen Quad
    screen_quad = new Quad();

    // Outer Space Environment
    BaseModel* outer_space_environment = new Model("outer_space_environment", "models/outer_space_environment/outer_space_environment.gltf", false, false);
    loaded_models[outer_space_environment->name] = outer_space_environment;

    // Space Station 1
    BaseModel* space_station1 = new Model("space_station1", "models/space_station1/space_station1.gltf", false, false);
    loaded_models[space_station1->name] = space_station1;

    // Space Station 2
    BaseModel* space_station2 = new Model("space_station2", "models/space_station2/space_station2.gltf", false, false);
    loaded_models[space_station2->name] = space_station2;

    // Vampire
    BaseModel* vampire = new Model("vampire", "models/vampire/vampire.gltf", false, false);
    loaded_models[vampire->name] = vampire;

    // Knight
    BaseModel* knight = new Model("knight", "models/knight/knight.gltf", false, false);
    loaded_models[knight->name] = knight;

    // Mutant
    BaseModel* mutant = new Model("mutant", "models/mutant/mutant.gltf", false, false);
    loaded_models[mutant->name] = mutant;

    // Android
    BaseModel* android = new Model("android", "models/android/android.gltf", false, false);
    loaded_models[android->name] = android;

    // Cylinder
    BaseModel* cylinder = new Cylinder("cylinder", 1.0f, 1.0f, 1.0f, 36, 1, true, 3);
    loaded_models[cylinder->name] = cylinder;

    // Cone
    BaseModel* cone = new Cylinder("cone", 1.0f, 0.0f, 1.0f, 36, 1, true, 3);
    loaded_models[cone->name] = cone;

    // Cube
    BaseModel* cube = new Cube("cube");
    loaded_models[cube->name] = cube;

    // Sphere
    BaseModel* sphere = new Sphere("sphere");
    loaded_models[sphere->name] = sphere;

    // Disk border
    BaseModel* disk_border = new DiskBorder("disk_border", 2.0f * M_PI);
    loaded_models[disk_border->name] = disk_border;

    // Quarter of a disk border
    BaseModel* quarter_disk_border = new DiskBorder("quarter_disk_border", M_PI / 2.0f, 1.0f, 1.3f);
    loaded_models[quarter_disk_border->name] = quarter_disk_border;
}

void Rendering::initialize_game_objects() {
    GameObject* outer_space_environment1 = new GameObject("outer_space_environment1", "outer_space_environment", glm::vec3(0.0f, -15.0f, 0.0f), glm::angleAxis(glm::radians(-90.0f), glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f))), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    game_objects[outer_space_environment1->name] = outer_space_environment1;
    id_color_to_game_object[outer_space_environment1->id_color] = outer_space_environment1;

    GameObject* space_station1_1 = new GameObject("space_station1_1", "space_station1", glm::vec3(-7.0f, 20.0f, -2.0f), glm::angleAxis(glm::radians(0.0f), glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f))), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    game_objects[space_station1_1->name] = space_station1_1;
    id_color_to_game_object[space_station1_1->id_color] = space_station1_1;

    GameObject* space_station2_1 = new GameObject("space_station2_1", "space_station2", glm::vec3(20.0f, 20.0f, -15.0f), glm::angleAxis(glm::radians(60.0f), glm::normalize(glm::vec3(1.0f, 0.3f, 0.8f))), glm::vec3(10.0f, 10.0f, 10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    game_objects[space_station2_1->name] = space_station2_1;
    id_color_to_game_object[space_station2_1->id_color] = space_station2_1;



    GameObject* vampire1 = new GameObject("vampire1", "vampire", glm::vec3(-7.0f, 0.0f, -2.0f), glm::angleAxis(glm::radians(0.0f), glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f))), glm::vec3(0.03f, 0.03f, 0.03f), glm::vec3(0.0f, 1.0f, 0.0f), 0);
    game_objects[vampire1->name] = vampire1;
    id_color_to_game_object[vampire1->id_color] = vampire1;

    GameObject* knight1 = new GameObject("knight1", "knight", glm::vec3(-2.0f, 0.0f, -2.0f), glm::angleAxis(glm::radians(0.0f), glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f))), glm::vec3(0.03f, 0.03f, 0.03f), glm::vec3(0.0f, 1.0f, 0.0f), 10);
    game_objects[knight1->name] = knight1;
    id_color_to_game_object[knight1->id_color] = knight1;

    GameObject* mutant1 = new GameObject("mutant1", "mutant", glm::vec3(6.0f, 0.0f, -2.0f), glm::angleAxis(glm::radians(0.0f), glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f))), glm::vec3(0.03f, 0.03f, 0.03f), glm::vec3(0.0f, 1.0f, 0.0f), 12);
    game_objects[mutant1->name] = mutant1;
    id_color_to_game_object[mutant1->id_color] = mutant1;

    GameObject* android1 = new GameObject("android1", "android", glm::vec3(2.0f, 0.0f, -2.0f), glm::angleAxis(glm::radians(0.0f), glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f))), glm::vec3(0.03f, 0.03f, 0.03f), glm::vec3(0.0f, 1.0f, 0.0f), 0);
    game_objects[android1->name] = android1;
    id_color_to_game_object[android1->id_color] = android1;



    GameObject* cylinder1 = new GameObject("cylinder1", "cylinder", glm::vec3(3.0f, 7.0f, -6.0f), glm::angleAxis(glm::radians(45.0f), glm::normalize(glm::vec3(0.8f, 0.6f, 1.0f))), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 0.0f), -1, false, false, true);
    game_objects[cylinder1->name] = cylinder1;
    id_color_to_game_object[cylinder1->id_color] = cylinder1;

    GameObject* cone1 = new GameObject("cone1", "cone", glm::vec3(-3.0f, 7.0f, -6.0f), glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), -1, false, false, true);
    game_objects[cone1->name] = cone1;
    id_color_to_game_object[cone1->id_color] = cone1;

    GameObject* cylinder2 = new GameObject("cylinder2", "cylinder", glm::vec3(-3.0f, -7.0f, -6.0f), glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), -1, false, false, true);
    game_objects[cylinder2->name] = cylinder2;
    id_color_to_game_object[cylinder2->id_color] = cylinder2;
    
    GameObject* cube1 = new GameObject("cube1", "cube", glm::vec3(2.5f, -1.5f, -4.0f), glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.8f, 0.2f, 0.4f), -1, false, false, true);
    game_objects[cube1->name] = cube1;
    id_color_to_game_object[cube1->id_color] = cube1;

    GameObject* disk_border1 = new GameObject("disk_border1", "disk_border", glm::vec3(3.5f, 1.5f, -4.5f), glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), -1, false, false, true);
    game_objects[disk_border1->name] = disk_border1;
    id_color_to_game_object[disk_border1->id_color] = disk_border1;
    

    
    GameObject* point_light1 = new PointLight("point_light1", "sphere", glm::vec3(3.0f, 3.0f, -3.0f), glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), -1, false, false, true,
        glm::vec3(0.05f, 0.05f, 0.00f), glm::vec3(0.8f, 0.8f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f), 1.0f, 0.045f, 0.0075f);
    game_objects[point_light1->name] = point_light1;
    point_lights[point_light1->name] = (PointLight*)point_light1;
    id_color_to_game_object[point_light1->id_color] = point_light1;
    
    GameObject* directional_light1 = new DirectionalLight("directional_light1", "sphere", glm::vec3(-5.0f, 5.0f, -3.0f), glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), -1, false, false, true,
        glm::vec3(0.3f, 0.3f, 0.3f), glm::vec3(0.7f, 0.7f, 0.7f), glm::vec3(0.9f, 0.9f, 0.9f), glm::vec3(3.0f, -4.0f, -3.0f));
    game_objects[directional_light1->name] = directional_light1;
    directional_lights[directional_light1->name] = (DirectionalLight*)directional_light1;
    id_color_to_game_object[directional_light1->id_color] = directional_light1;

    GameObject* spot_light1 = new SpotLight("spot_light1", "sphere", glm::vec3(-5.0f, -5.0f, -3.0f), glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 0.0f), -1, false, false, true,
        glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(5.0f, 5.0f, -3.0f),
        glm::cos(glm::radians(12.5f)), glm::cos(glm::radians(15.0f)), 1.0f, 0.09f, 0.032f);
    game_objects[spot_light1->name] = spot_light1;
    spot_lights[spot_light1->name] = (SpotLight*)spot_light1;
    id_color_to_game_object[spot_light1->id_color] = spot_light1;
}

void Rendering::set_time_before_rendering_loop() {
    this->time_before_rendering = std::chrono::system_clock::now();
}

void Rendering::render_viewport() {
    int texture_viewport_width = user_interface->texture_viewport_width;
    int texture_viewport_height = user_interface->texture_viewport_height;
    glViewport(0, 0, texture_viewport_width, texture_viewport_height);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    unsigned int attachments0[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
    glDrawBuffers(4, attachments0);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // view/projection transformations
    projection = glm::perspective(glm::radians(camera_viewport->Zoom), (float)texture_viewport_width / (float)texture_viewport_height, near_camera_viewport, far_camera_viewport);
    view = camera_viewport->GetViewMatrix();
    view_projection = projection * view;

    unsigned int attachments1[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments1);
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

    // Render the game objects with Phong shading and also render the unique Color IDs of each game object
    // (later for the selection technique: Color Picking)
    phong_shader->setInt("is_transform3d", 0);
    for (auto it = game_objects.begin(); it != game_objects.end(); it++) {
        GameObject* game_object = it->second;
        game_object->draw(phong_shader, false);
    }

    // Render only the last selected object (if it exists) to later outline the shape of this object
    unsigned int attachments2[1] = { GL_COLOR_ATTACHMENT3 };
    glDrawBuffers(1, attachments2);
    selection_shader->use();
    if (last_selected_object != nullptr) {
        last_selected_object->draw(selection_shader, true);
    }

    // Draw skybox    
    unsigned int attachments3[1] = { GL_COLOR_ATTACHMENT0 }; // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    glDrawBuffers(1, attachments3);
    view_skybox = glm::mat4(glm::mat3(view)); // remove translation from the view matrix so it doesn't affect the skybox
    view_projection_skybox = projection * view_skybox;
    skybox_shader->use();
    skybox_shader->setMat4("view_projection", view_projection_skybox);
    cubemap->draw(skybox_shader, skyboxes_loaded[2]);

    // Draw border outlining the selected object (if it exists one)
    outline_shader->use();
    outline_shader->setVec2("pixel_size", glm::vec2(1.0f / user_interface->texture_viewport_width, 1.0f / user_interface->texture_viewport_height));
    outline_shader->setVec3("outline_color", outline_color);
    screen_quad->draw(outline_shader, texture_selected_color_buffer, true);

    // Clear the depth buffer so the Transform3D is drawn over everything
    glClear(GL_DEPTH_BUFFER_BIT);

    // Draw 3D transforms if there is a selected object
    glDrawBuffers(3, attachments1);
    phong_shader->use();
    phong_shader->setInt("is_transform3d", 1);
    if (last_selected_object != nullptr) {
        transform3d->update_model_matrices(last_selected_object);
        transform3d->draw(phong_shader);
    }


    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Check mouse over viewport's models using Color Picking technique
GameObject* Rendering::check_mouse_over_models() {
    int texture_viewport_width = user_interface->texture_viewport_width;
    int texture_viewport_height = user_interface->texture_viewport_height;
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 mouse_pos = io.MousePos;
    ImVec2 window_pos = ImGui::GetWindowPos();
    ImVec2 mouse_pos_in_window = ImVec2(mouse_pos.x - window_pos.x, mouse_pos.y - window_pos.y);
    ImVec2& viewport_texture_pos = user_interface->viewport_texture_pos;
    ImVec2 mouse_pos_in_viewport_texture = ImVec2(mouse_pos_in_window.x - viewport_texture_pos.x, mouse_pos_in_window.y - viewport_texture_pos.y);
    std::cout << mouse_pos_in_viewport_texture.x << " " << texture_viewport_height - mouse_pos_in_viewport_texture.y << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glReadBuffer(GL_COLOR_ATTACHMENT1);
    GLubyte pixel[4];
    pixel[0] = pixel[1] = pixel[2] = pixel[3] = 0;
    glReadPixels(mouse_pos_in_viewport_texture.x, texture_viewport_height - mouse_pos_in_viewport_texture.y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
    std::cout << "PIXEL: " << (int)pixel[0] << " " << (int)pixel[1] << " " << (int)pixel[2] << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    auto it = id_color_to_game_object.find(glm::u8vec3(pixel[0], pixel[1], pixel[2]));
    if (it != id_color_to_game_object.end()) {
        return it->second;
    }
    else {
        return nullptr;
    }
}

// Check mouse over viewport's transform 3Ds models using Color Picking technique
GameObject* Rendering::check_mouse_over_transform3d() {
    int texture_viewport_width = user_interface->texture_viewport_width;
    int texture_viewport_height = user_interface->texture_viewport_height;
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 mouse_pos = io.MousePos;
    ImVec2 window_pos = ImGui::GetWindowPos();
    ImVec2 mouse_pos_in_window = ImVec2(mouse_pos.x - window_pos.x, mouse_pos.y - window_pos.y);
    ImVec2& viewport_texture_pos = user_interface->viewport_texture_pos;
    ImVec2 mouse_pos_in_viewport_texture = ImVec2(mouse_pos_in_window.x - viewport_texture_pos.x, mouse_pos_in_window.y - viewport_texture_pos.y);
    //std::cout << mouse_pos_in_viewport_texture.x << " " << texture_viewport_height - mouse_pos_in_viewport_texture.y << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glReadBuffer(GL_COLOR_ATTACHMENT2);
    GLubyte pixel[4];
    pixel[0] = pixel[1] = pixel[2] = pixel[3] = 0;
    glReadPixels(mouse_pos_in_viewport_texture.x, texture_viewport_height - mouse_pos_in_viewport_texture.y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
    //std::cout << "PIXEL: " << (int)pixel[0] << " " << (int)pixel[1] << " " << (int)pixel[2] << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    auto it = id_color_to_game_object_transform3d.find(glm::u8vec3(pixel[0], pixel[1], pixel[2]));
    if (it != id_color_to_game_object_transform3d.end()) {
        return it->second;
    }
    else {
        return nullptr;
    }
}

// Check mouse over viewport's models using Ray Casting technique
/*
std::string Rendering::check_mouse_over_models2() {
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
            if (game_object->intersected_ray(ray_dir, camera_viewport->Position, t)) {
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
}*/

void Rendering::clean() {
    delete phong_shader;
    delete selection_shader;
    delete outline_shader;
    delete skybox_shader;
    for (auto it = loaded_models.begin(); it != loaded_models.end(); it++) {
        delete it->second;
    }
    for (auto it = game_objects.begin(); it != game_objects.end(); it++) {
        delete it->second;
    }
    delete transform3d;
    delete camera_viewport;
    delete screen_quad;
    delete cubemap;
    GameObject::clean();
}

void Rendering::clean_viewport_framebuffer() {
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteTextures(1, &textureColorbuffer);
    glDeleteTextures(1, &texture_id_colors);
    glDeleteTextures(1, &texture_id_colors_transform3d);
    glDeleteTextures(1, &texture_selected_color_buffer);
    glDeleteRenderbuffers(1, &rboDepthStencil);
}