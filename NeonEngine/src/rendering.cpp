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
#include "pbr.h"

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
    pbr_shader = nullptr;
    equirectangularToCubemapShader = nullptr;
    irradianceShader = nullptr;
    prefilterShader = nullptr;
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
    exposure = 2.0f;
    loaded_materials["Default"] = nullptr;
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
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // PBR: for sampling in lower mip levels in the pre-filter map
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void Rendering::create_and_set_viewport_framebuffer() {
    int texture_viewport_width = user_interface->texture_viewport_width;
    int texture_viewport_height = user_interface->texture_viewport_height;

    int color_texture_width = texture_viewport_width;
    int color_texture_height = texture_viewport_height;
    int depth_stencil_rbo_width = texture_viewport_width;
    int depth_stencil_rbo_height = texture_viewport_height;

    // Create and bind Framebuffer
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);


    // Create and set Color Texture for main rendering
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, color_texture_width, color_texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Attach the Texture to the currently bound Framebuffer object
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);


    // Create and set Color Texture for rendering the id colors
    glGenTextures(1, &texture_id_colors);
    glBindTexture(GL_TEXTURE_2D, texture_id_colors);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, color_texture_width, color_texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Attach the Texture to the currently bound Framebuffer object
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, texture_id_colors, 0);


    // Create and set Color Texture for rendering the id colors of the Transform3D object
    glGenTextures(1, &texture_id_colors_transform3d);
    glBindTexture(GL_TEXTURE_2D, texture_id_colors_transform3d);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, color_texture_width, color_texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Attach the Texture to the currently bound Framebuffer object
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, texture_id_colors_transform3d, 0);


    // Create and set Color Texture for rendering of selected objects
    glGenTextures(1, &texture_selected_color_buffer);
    glBindTexture(GL_TEXTURE_2D, texture_selected_color_buffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, color_texture_width, color_texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Attach the Texture to the currently bound Framebuffer object
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, texture_selected_color_buffer, 0);


    // Create and set Depth and Stencil Renderbuffer
    glGenRenderbuffers(1, &rboDepthStencil);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepthStencil);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, depth_stencil_rbo_width, depth_stencil_rbo_height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // Attach the Renderbuffer to the currently bound Gramebuffer object
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboDepthStencil);

    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    unsigned int attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
    glDrawBuffers(4, attachments);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

    // Bind to the default Framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Rendering::set_viewport_shaders() {
    // build and compile shaders
    phong_shader = new Shader("shaders/vertices_3d_model.vert", "shaders/phong_lighting.frag");
    pbr_shader = new Shader("shaders/vertices_3d_model.vert", "shaders/PBR.frag");
    equirectangularToCubemapShader = new Shader("shaders/cubemap.vert", "shaders/equirectangular_to_cubemap.frag");
    irradianceShader = new Shader("shaders/cubemap.vert", "shaders/irradiance_convolution.frag");
    prefilterShader = new Shader("shaders/cubemap.vert", "shaders/prefilter.frag");
    brdfShader = new Shader("shaders/brdf.vert", "shaders/brdf.frag");
    selection_shader = new Shader("shaders/vertices_3d_model.vert", "shaders/paint_selected.frag");
    outline_shader = new Shader("shaders/vertices_quad.vert", "shaders/edge_outlining.frag");
    skybox_shader = new Shader("shaders/skybox.vert", "shaders/skybox.frag");
}

void Rendering::set_viewport_data() {
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

    cubemap = new Cubemap("ocean_with_sky", skybox_textures_ocean_with_sky);
    cubemap->add_cubemap_texture("nebula", skybox_textures_nebula);
    cubemap->add_cubemap_texture("red_space", skybox_textures_red_space);

    // Materials:

    // Rusted Iron
    Material* mat_rusted_iron = new Material("mat_rusted_iron");
    mat_rusted_iron->format = FileFormat::Default;
    
    Texture* tex_albedo_rusted_iron = new Texture("tex_rusted_iron");
    tex_albedo_rusted_iron->path = "materials/rusted_iron/albedo.png";
    tex_albedo_rusted_iron->id = load_texture(tex_albedo_rusted_iron->path, tex_albedo_rusted_iron->num_channels);
    tex_albedo_rusted_iron->types.insert(TexAlbedo);
    loaded_textures[tex_albedo_rusted_iron->get_name()] = tex_albedo_rusted_iron;

    Texture* tex_normal_rusted_iron = new Texture("tex_rusted_iron");
    tex_normal_rusted_iron->path = "materials/rusted_iron/normal.png";
    tex_normal_rusted_iron->id = load_texture(tex_normal_rusted_iron->path, tex_normal_rusted_iron->num_channels);
    tex_normal_rusted_iron->types.insert(TexNormal);
    loaded_textures[tex_normal_rusted_iron->get_name()] = tex_normal_rusted_iron;

    Texture* tex_metalness_rusted_iron = new Texture("tex_rusted_iron");
    tex_metalness_rusted_iron->path = "materials/rusted_iron/metallic.png";
    tex_metalness_rusted_iron->id = load_texture(tex_metalness_rusted_iron->path, tex_metalness_rusted_iron->num_channels);
    tex_metalness_rusted_iron->types.insert(TexMetalness);
    loaded_textures[tex_metalness_rusted_iron->get_name()] = tex_metalness_rusted_iron;

    Texture* tex_roughness_rusted_iron = new Texture("tex_rusted_iron");
    tex_roughness_rusted_iron->path = "materials/rusted_iron/roughness.png";
    tex_roughness_rusted_iron->id = load_texture(tex_roughness_rusted_iron->path, tex_roughness_rusted_iron->num_channels);
    tex_roughness_rusted_iron->types.insert(TexRoughness);
    loaded_textures[tex_roughness_rusted_iron->get_name()] = tex_roughness_rusted_iron;

    Texture* tex_ambient_occlussion_rusted_iron = new Texture("tex_rusted_iron");
    tex_ambient_occlussion_rusted_iron->path = "materials/rusted_iron/ao.png";
    tex_ambient_occlussion_rusted_iron->id = load_texture(tex_ambient_occlussion_rusted_iron->path, tex_ambient_occlussion_rusted_iron->num_channels);
    tex_ambient_occlussion_rusted_iron->types.insert(TexAmbientOcclusion);
    loaded_textures[tex_ambient_occlussion_rusted_iron->get_name()] = tex_ambient_occlussion_rusted_iron;
    
    mat_rusted_iron->textures[TexAlbedo] = tex_albedo_rusted_iron;
    mat_rusted_iron->textures[TexNormal] = tex_normal_rusted_iron;
    mat_rusted_iron->textures[TexMetalness] = tex_metalness_rusted_iron;
    mat_rusted_iron->textures[TexRoughness] = tex_roughness_rusted_iron;
    mat_rusted_iron->textures[TexAmbientOcclusion] = tex_ambient_occlussion_rusted_iron;

    loaded_materials[mat_rusted_iron->name] = mat_rusted_iron;

    // Gold
    Material* mat_gold = new Material("mat_gold");
    mat_gold->format = FileFormat::Default;

    Texture* tex_albedo_gold = new Texture("tex_gold");
    tex_albedo_gold->path = "materials/gold/albedo.png";
    tex_albedo_gold->id = load_texture(tex_albedo_gold->path, tex_albedo_gold->num_channels);
    tex_albedo_gold->types.insert(TexAlbedo);
    loaded_textures[tex_albedo_gold->get_name()] = tex_albedo_gold;

    Texture* tex_normal_gold = new Texture("tex_gold");
    tex_normal_gold->path = "materials/gold/normal.png";
    tex_normal_gold->id = load_texture(tex_normal_gold->path, tex_normal_gold->num_channels);
    tex_normal_gold->types.insert(TexNormal);
    loaded_textures[tex_normal_gold->get_name()] = tex_normal_gold;

    Texture* tex_metalness_gold = new Texture("tex_gold");
    tex_metalness_gold->path = "materials/gold/metallic.png";
    tex_metalness_gold->id = load_texture(tex_metalness_gold->path, tex_metalness_gold->num_channels);
    tex_metalness_gold->types.insert(TexMetalness);
    loaded_textures[tex_metalness_gold->get_name()] = tex_metalness_gold;

    Texture* tex_roughness_gold = new Texture("tex_gold");
    tex_roughness_gold->path = "materials/gold/roughness.png";
    tex_roughness_gold->id = load_texture(tex_roughness_gold->path, tex_roughness_gold->num_channels);
    tex_roughness_gold->types.insert(TexRoughness);
    loaded_textures[tex_roughness_gold->get_name()] = tex_roughness_gold;

    Texture* tex_ambient_occlussion_gold = new Texture("tex_gold");
    tex_ambient_occlussion_gold->path = "materials/gold/ao.png";
    tex_ambient_occlussion_gold->id = load_texture(tex_ambient_occlussion_gold->path, tex_ambient_occlussion_gold->num_channels);
    tex_ambient_occlussion_gold->types.insert(TexAmbientOcclusion);
    loaded_textures[tex_ambient_occlussion_gold->get_name()] = tex_ambient_occlussion_gold;

    mat_gold->textures[TexAlbedo] = tex_albedo_gold;
    mat_gold->textures[TexNormal] = tex_normal_gold;
    mat_gold->textures[TexMetalness] = tex_metalness_gold;
    mat_gold->textures[TexRoughness] = tex_roughness_gold;
    mat_gold->textures[TexAmbientOcclusion] = tex_ambient_occlussion_gold;

    loaded_materials[mat_gold->name] = mat_gold;



    // Screen Quad
    screen_quad = new Quad();
    
    /*
    // Lava Planet
    Model* lava_planet = new Model("lava_planet", "models/lava_planet/lava_planet.gltf", false, false);
    add_model_to_loaded_data(lava_planet);

    // Sun
    Model* sun = new Model("sun", "models/sun/sun.gltf", false, false);
    add_model_to_loaded_data(sun);

    // Space Station 1
    Model* space_station1 = new Model("space_station1", "models/space_station1/space_station1.gltf", false, false);
    add_model_to_loaded_data(space_station1);

    // Space Station 2
    Model* space_station2 = new Model("space_station2", "models/space_station2/space_station2.gltf", false, false);
    add_model_to_loaded_data(space_station2);

    // Vampire
    Model* vampire = new Model("vampire", "models/vampire/vampire.gltf", false, false);
    add_model_to_loaded_data(vampire);*/

    // Knight
    Model* knight = new Model("knight", "models/knight/knight.gltf", false, false);
    add_model_to_loaded_data(knight);

    // Mutant
    Model* mutant = new Model("mutant", "models/mutant/mutant.gltf", false, false);
    add_model_to_loaded_data(mutant);

    // Android
    Model* android = new Model("android", "models/android/android.gltf", false, false);
    add_model_to_loaded_data(android);



    // Primitive shapes:

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
    BaseModel* sphere = new Sphere("sphere", 1.0f, 360, 180);
    loaded_models[sphere->name] = sphere;

    // Disk border
    BaseModel* disk_border = new DiskBorder("disk_border", 2.0f * M_PI);
    loaded_models[disk_border->name] = disk_border;

    // Quarter of a disk border
    BaseModel* quarter_disk_border = new DiskBorder("quarter_disk_border", M_PI / 2.0f, 1.0f, 1.3f);
    loaded_models[quarter_disk_border->name] = quarter_disk_border;

    std::cout << std::endl;
    print_names_loaded_models();

    std::cout << std::endl;
    print_names_loaded_materials();

    std::cout << std::endl;
    print_names_loaded_textures();
}

void Rendering::print_names_loaded_models() {
    std::cout << "LOADED MODELS:" << std::endl;
    for (auto it = loaded_models.begin(); it != loaded_models.end(); it++) {
        std::cout << it->first << std::endl;
    }
}

void Rendering::print_names_loaded_materials() {
    std::cout << "LOADED MATERIALS:" << std::endl;
    for (auto it = loaded_materials.begin(); it != loaded_materials.end(); it++) {
        std::cout << it->first << std::endl;
    }
}

void Rendering::print_names_loaded_textures() {
    std::cout << "LOADED TEXTURES:" << std::endl;
    for (auto it = loaded_textures.begin(); it != loaded_textures.end(); it++) {
        std::cout << it->first << std::endl;
    }
}

void Rendering::add_model_to_loaded_data(Model* model) {
    loaded_models[model->name] = model;
    for (auto it = model->loaded_materials.begin(); it != model->loaded_materials.end(); it++) {
        Material* material = it->second;
        loaded_materials[material->name] = material;
    }
    for (auto it = model->loaded_textures.begin(); it != model->loaded_textures.end(); it++) {
        Texture* texture = it->second;
        loaded_textures[texture->get_name()] = texture;
    }
}

void Rendering::initialize_game_objects() {
    /*
    GameObject* lava_planet1 = new GameObject("lava_planet1", "lava_planet");
    lava_planet1->position = glm::vec3(-30.0f, 30.0f, -15.0f);
    lava_planet1->scale = glm::vec3(10.0f);
    lava_planet1->set_model_matrices_standard();
    game_objects[lava_planet1->name] = lava_planet1;
    id_color_to_game_object[lava_planet1->id_color] = lava_planet1;

    GameObject* sun1 = new GameObject("sun1", "sun");
    sun1->position = glm::vec3(20.0f, 40.0f, -30.0f);
    sun1->metalness = 0.1;
    sun1->roughness = 0.8;
    sun1->set_model_matrices_standard();
    game_objects[sun1->name] = sun1;
    id_color_to_game_object[sun1->id_color] = sun1;

    GameObject* space_station1_1 = new GameObject("space_station1_1", "space_station1");
    space_station1_1->position = glm::vec3(-7.0f, 20.0f, -2.0f);
    space_station1_1->set_model_matrices_standard();
    game_objects[space_station1_1->name] = space_station1_1;
    id_color_to_game_object[space_station1_1->id_color] = space_station1_1;

    GameObject* space_station2_1 = new GameObject("space_station2_1", "space_station2");
    space_station2_1->position = glm::vec3(30.0f, 15.0f, -15.0f);
    space_station2_1->rotation = glm::angleAxis(glm::radians(60.0f), glm::normalize(glm::vec3(1.0f, 0.3f, 0.8f)));
    space_station2_1->scale = glm::vec3(10.0f);
    space_station2_1->set_model_matrices_standard();
    game_objects[space_station2_1->name] = space_station2_1;
    id_color_to_game_object[space_station2_1->id_color] = space_station2_1;



    GameObject* vampire1 = new GameObject("vampire1", "vampire");
    vampire1->position = glm::vec3(-7.0f, 0.0f, -2.0f);
    vampire1->scale = glm::vec3(0.03f);
    vampire1->animation_id = 0;
    vampire1->metalness = 0.1;
    vampire1->roughness = 0.5;
    vampire1->set_model_matrices_standard();
    game_objects[vampire1->name] = vampire1;
    id_color_to_game_object[vampire1->id_color] = vampire1;*/

    GameObject* knight1 = new GameObject("knight1", "knight");
    knight1->position = glm::vec3(-2.0f, 0.0f, -2.0f);
    knight1->scale = glm::vec3(0.03f);
    knight1->animation_id = 10;
    knight1->set_model_matrices_standard();
    game_objects[knight1->name] = knight1;
    id_color_to_game_object[knight1->id_color] = knight1;

    GameObject* mutant1 = new GameObject("mutant1", "mutant");
    mutant1->position = glm::vec3(6.0f, 0.0f, -2.0f);
    mutant1->scale = glm::vec3(0.03f);
    mutant1->animation_id = 0;
    mutant1->metalness = 0.1;
    mutant1->roughness = 0.5;
    mutant1->set_model_matrices_standard();
    game_objects[mutant1->name] = mutant1;
    id_color_to_game_object[mutant1->id_color] = mutant1;

    GameObject* android1 = new GameObject("android1", "android");
    android1->position = glm::vec3(2.0f, 0.0f, -2.0f);
    android1->scale = glm::vec3(0.03f);
    android1->animation_id = 0;
    android1->set_model_matrices_standard();
    game_objects[android1->name] = android1;
    id_color_to_game_object[android1->id_color] = android1;

    GameObject* android2 = new GameObject("android2", "android");
    android2->position = glm::vec3(0.0f, 0.0f, -2.0f);
    android2->scale = glm::vec3(0.03f);
    android2->animation_id = 0;
    android2->material = loaded_materials["mat_gold"];
    android2->set_model_matrices_standard();
    game_objects[android2->name] = android2;
    id_color_to_game_object[android2->id_color] = android2;



    GameObject* cylinder1 = new GameObject("cylinder1", "cylinder");
    cylinder1->position = glm::vec3(3.0f, 7.0f, -6.0f);
    cylinder1->rotation = glm::angleAxis(glm::radians(45.0f), glm::normalize(glm::vec3(0.8f, 0.6f, 1.0f)));
    cylinder1->albedo = glm::vec3(1.0f, 1.0f, 0.0f);
    cylinder1->render_one_color = true;
    cylinder1->set_model_matrices_standard();
    game_objects[cylinder1->name] = cylinder1;
    id_color_to_game_object[cylinder1->id_color] = cylinder1;

    GameObject* cone1 = new GameObject("cone1", "cone");
    cone1->position = glm::vec3(-3.0f, 7.0f, -6.0f);
    cone1->albedo = glm::vec3(0.0f, 1.0f, 0.0f);
    cone1->render_one_color = true;
    cone1->set_model_matrices_standard();
    game_objects[cone1->name] = cone1;
    id_color_to_game_object[cone1->id_color] = cone1;

    GameObject* cylinder2 = new GameObject("cylinder2", "cylinder");
    cylinder2->position = glm::vec3(-3.0f, -7.0f, -6.0f);
    cylinder2->albedo = glm::vec3(0.0f, 0.0f, 1.0f);
    cylinder2->render_one_color = true;
    cylinder2->material = loaded_materials["mat_gold"];
    cylinder2->set_model_matrices_standard();
    game_objects[cylinder2->name] = cylinder2;
    id_color_to_game_object[cylinder2->id_color] = cylinder2;
    
    GameObject* cube1 = new GameObject("cube1", "cube");
    cube1->position = glm::vec3(2.5f, -1.5f, -4.0f);
    cube1->material = loaded_materials["mat_rusted_iron"];
    cube1->set_model_matrices_standard();
    game_objects[cube1->name] = cube1;
    id_color_to_game_object[cube1->id_color] = cube1;

    GameObject* disk_border1 = new GameObject("disk_border1", "disk_border");
    disk_border1->position = glm::vec3(3.5f, 1.5f, -4.5f);
    disk_border1->scale = glm::vec3(0.5f);
    disk_border1->albedo = glm::vec3(1.0f, 0.0f, 0.0f);
    disk_border1->render_one_color = true;
    disk_border1->set_model_matrices_standard();
    game_objects[disk_border1->name] = disk_border1;
    id_color_to_game_object[disk_border1->id_color] = disk_border1;
    

    
    PointLight* point_light1 = new PointLight("point_light1", "sphere");
    point_light1->type = TypePointLight;
    point_light1->position = glm::vec3(-10.0f, 10.0f, 10.0f);
    point_light1->albedo = glm::vec3(0.0f, 1.0f, 0.0f);
    point_light1->light_color = glm::vec3(1.0f, 1.0f, 1.0f);
    point_light1->intensity = 5.0f;
    point_light1->set_model_matrices_standard();
    game_objects[point_light1->name] = point_light1;
    point_lights[point_light1->name] = point_light1;
    id_color_to_game_object[point_light1->id_color] = point_light1;

    PointLight* point_light2 = new PointLight("point_light2", "sphere");
    point_light2->type = TypePointLight;
    point_light2->position = glm::vec3(10.0f, 10.0f, 10.0f);
    point_light2->light_color = glm::vec3(1.0f, 1.0f, 1.0f);
    point_light2->intensity = 5.0f;
    point_light2->set_model_matrices_standard();
    game_objects[point_light2->name] = point_light2;
    point_lights[point_light2->name] = point_light2;
    id_color_to_game_object[point_light2->id_color] = point_light2;

    PointLight* point_light3 = new PointLight("point_light3", "sphere");
    point_light3->type = TypePointLight;
    point_light3->position = glm::vec3(-10.0f, -10.0f, 10.0f);
    point_light3->light_color = glm::vec3(1.0f, 1.0f, 1.0f);
    point_light3->intensity = 5.0f;
    point_light3->set_model_matrices_standard();
    game_objects[point_light3->name] = point_light3;
    point_lights[point_light3->name] = point_light3;
    id_color_to_game_object[point_light3->id_color] = point_light3;

    PointLight* point_light4 = new PointLight("point_light4", "sphere");
    point_light4->type = TypePointLight;
    point_light4->position = glm::vec3(10.0f, -10.0f, 10.0f);
    point_light4->light_color = glm::vec3(1.0f, 1.0f, 1.0f);
    point_light4->intensity = 5.0f;
    point_light4->set_model_matrices_standard();
    game_objects[point_light4->name] = point_light4;
    point_lights[point_light4->name] = point_light4;
    id_color_to_game_object[point_light4->id_color] = point_light4;

    DirectionalLight* directional_light1 = new DirectionalLight("directional_light1", "sphere");
    directional_light1->type = TypeDirectionalLight;
    directional_light1->position = glm::vec3(-5.0f, 5.0f, -3.0f);
    directional_light1->ambient = glm::vec3(0.3f);
    directional_light1->light_color = glm::vec3(1.0f, 1.0f, 1.0f);
    directional_light1->intensity = 1.0f;
    directional_light1->specular = glm::vec3(0.9f);
    directional_light1->direction = glm::vec3(3.0f, -4.0f, -3.0f);
    directional_light1->set_model_matrices_standard();
    game_objects[directional_light1->name] = directional_light1;
    directional_lights[directional_light1->name] = directional_light1;
    id_color_to_game_object[directional_light1->id_color] = directional_light1;

    SpotLight* spot_light1 = new SpotLight("spot_light1", "sphere");
    spot_light1->type = TypeSpotLight;
    spot_light1->position = glm::vec3(-5.0f, -5.0f, -3.0f);
    spot_light1->albedo = glm::vec3(1.0f, 1.0f, 0.0f);
    spot_light1->ambient = glm::vec3(0.0f);
    spot_light1->light_color = glm::vec3(1.0f, 1.0f, 1.0f);
    spot_light1->intensity = 300.0f;
    spot_light1->specular = glm::vec3(1.0f);
    spot_light1->direction = glm::vec3(5.0f, 5.0f, -3.0f);
    spot_light1->set_model_matrices_standard();
    game_objects[spot_light1->name] = spot_light1;
    spot_lights[spot_light1->name] = spot_light1;
    id_color_to_game_object[spot_light1->id_color] = spot_light1;
}

void Rendering::set_pbr_shader() {
    const int ENVIRONMENT_MAP_WIDTH = 2048;
    const int ENVIRONMENT_MAP_HEIGHT = 2048;
    const int IRRADIANCE_MAP_WIDTH = 128;
    const int IRRADIANCE_MAP_HEIGHT = 128;
    const int PREFILTER_MAP_WIDTH = 512;
    const int PREFILTER_MAP_HEIGHT = 512;
    const int BRDF_LUT_MAP_WIDTH = 2048;
    const int BRDF_LUT_MAP_HEIGHT = 2048;

    // set up projection and view matrices for capturing data onto the 6 cubemap face directions
    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    std::vector<glm::mat4> captureViews =
    {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    
    unsigned int captureFBO = create_framebuffer_pbr();
    envCubemap = create_environment_map_from_equirectangular_map("HDRIs/earth_space_low_res.hdr", captureFBO,
                                                                 ENVIRONMENT_MAP_WIDTH, ENVIRONMENT_MAP_HEIGHT, captureProjection, captureViews,
                                                                 equirectangularToCubemapShader);
    irradianceMap = create_irradiance_map_from_environment_map(captureFBO, envCubemap, IRRADIANCE_MAP_WIDTH, IRRADIANCE_MAP_HEIGHT, captureProjection, captureViews, irradianceShader);
    prefilterMap = create_prefilter_map_from_environment_map(captureFBO, envCubemap, ENVIRONMENT_MAP_WIDTH, PREFILTER_MAP_WIDTH, PREFILTER_MAP_HEIGHT, captureProjection, captureViews, prefilterShader);
    brdfLUTTexture = create_brdf_lut_texture(captureFBO, BRDF_LUT_MAP_WIDTH, BRDF_LUT_MAP_HEIGHT, brdfShader);
    
    /*
    stbi_set_flip_vertically_on_load(false);
    envCubemap = load_hdr_file_to_cubemap({ "PBR_data/environment_cubemap.hdr" }, ENVIRONMENT_MAP_WIDTH, ENVIRONMENT_MAP_HEIGHT);
    irradianceMap = load_hdr_file_to_cubemap({ "PBR_data/irradiance_cubemap.hdr" }, IRRADIANCE_MAP_WIDTH, IRRADIANCE_MAP_HEIGHT);
    prefilterMap = load_hdr_file_to_cubemap({ "PBR_data/prefilter_cubemap0.hdr", "PBR_data/prefilter_cubemap1.hdr", "PBR_data/prefilter_cubemap2.hdr", "PBR_data/prefilter_cubemap3.hdr", "PBR_data/prefilter_cubemap4.hdr" },
                                            PREFILTER_MAP_WIDTH, PREFILTER_MAP_HEIGHT);
    brdfLUTTexture = load_hdr_texture("PBR_data/brdf_LUT_texture.hdr");
    stbi_set_flip_vertically_on_load(true);*/
    
    
    save_cubemap_to_hdr_file(envCubemap, 3, { ENVIRONMENT_MAP_WIDTH }, { ENVIRONMENT_MAP_HEIGHT }, { "PBR_data/environment_cubemap.hdr" });
    save_cubemap_to_hdr_file(irradianceMap, 3, { IRRADIANCE_MAP_WIDTH }, { IRRADIANCE_MAP_HEIGHT }, { "PBR_data/irradiance_cubemap.hdr" });
    save_cubemap_to_hdr_file(prefilterMap, 3, { PREFILTER_MAP_WIDTH, PREFILTER_MAP_WIDTH / 2, PREFILTER_MAP_WIDTH / 4, PREFILTER_MAP_WIDTH / 8, PREFILTER_MAP_WIDTH / 16 },
                             { PREFILTER_MAP_HEIGHT, PREFILTER_MAP_HEIGHT / 2, PREFILTER_MAP_HEIGHT / 4, PREFILTER_MAP_HEIGHT / 8, PREFILTER_MAP_HEIGHT / 16 },
                             { "PBR_data/prefilter_cubemap0.hdr", "PBR_data/prefilter_cubemap1.hdr", "PBR_data/prefilter_cubemap2.hdr", "PBR_data/prefilter_cubemap3.hdr", "PBR_data/prefilter_cubemap4.hdr" });
    save_texture_to_hdr_file(brdfLUTTexture, 4, BRDF_LUT_MAP_WIDTH, BRDF_LUT_MAP_HEIGHT, "PBR_data/brdf_LUT_texture.hdr");

    cubemap->add_cubemap_texture("hdri_cubemap", envCubemap);
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

    Shader* lighting_shader = pbr_shader;
    //Shader* lighting_shader = phong_shader;

    // bind pre-computed IBL data
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);

    lighting_shader->use();
    lighting_shader->setInt("irradianceMap", 0);
    lighting_shader->setInt("prefilterMap", 1);
    lighting_shader->setInt("brdfLUT", 2);

    lighting_shader->setMat4("view_projection", view_projection);

    lighting_shader->setVec3("viewPos", camera_viewport->Position);

    lighting_shader->setInt("num_point_lights", point_lights.size());
    lighting_shader->setInt("num_directional_lights", directional_lights.size());
    lighting_shader->setInt("num_spot_lights", spot_lights.size());

    int idx_point_light = 0;
    for (auto it = point_lights.begin(); it != point_lights.end(); it++, idx_point_light++) {
        lighting_shader->setVec3("pointLights[" + std::to_string(idx_point_light) + "].position", it->second->position);
        lighting_shader->setVec3("pointLights[" + std::to_string(idx_point_light) + "].ambient", it->second->ambient);
        lighting_shader->setVec3("pointLights[" + std::to_string(idx_point_light) + "].radiance", it->second->light_color * it->second->intensity);
        lighting_shader->setVec3("pointLights[" + std::to_string(idx_point_light) + "].specular", it->second->specular);
        lighting_shader->setFloat("pointLights[" + std::to_string(idx_point_light) + "].constant", it->second->constant);
        lighting_shader->setFloat("pointLights[" + std::to_string(idx_point_light) + "].linear", it->second->linear);
        lighting_shader->setFloat("pointLights[" + std::to_string(idx_point_light) + "].quadratic", it->second->quadratic);
    }

    int idx_directional_light = 0;
    for (auto it = directional_lights.begin(); it != directional_lights.end(); it++, idx_directional_light++) {
        lighting_shader->setVec3("directionalLights[" + std::to_string(idx_directional_light) + "].direction", it->second->direction);
        lighting_shader->setVec3("directionalLights[" + std::to_string(idx_directional_light) + "].ambient", it->second->ambient);
        lighting_shader->setVec3("directionalLights[" + std::to_string(idx_directional_light) + "].radiance", it->second->light_color * it->second->intensity);
        lighting_shader->setVec3("directionalLights[" + std::to_string(idx_directional_light) + "].specular", it->second->specular);
    }

    int idx_spot_light = 0;
    for (auto it = spot_lights.begin(); it != spot_lights.end(); it++, idx_spot_light++) {
        lighting_shader->setVec3("spotLights[" + std::to_string(idx_spot_light) + "].position", it->second->position);
        lighting_shader->setVec3("spotLights[" + std::to_string(idx_spot_light) + "].direction", it->second->direction);
        lighting_shader->setVec3("spotLights[" + std::to_string(idx_spot_light) + "].ambient", it->second->ambient);
        lighting_shader->setVec3("spotLights[" + std::to_string(idx_spot_light) + "].radiance", it->second->light_color * it->second->intensity);
        lighting_shader->setVec3("spotLights[" + std::to_string(idx_spot_light) + "].specular", it->second->specular);
        lighting_shader->setFloat("spotLights[" + std::to_string(idx_spot_light) + "].constant", it->second->constant);
        lighting_shader->setFloat("spotLights[" + std::to_string(idx_spot_light) + "].linear", it->second->linear);
        lighting_shader->setFloat("spotLights[" + std::to_string(idx_spot_light) + "].quadratic", it->second->quadratic);
        lighting_shader->setFloat("spotLights[" + std::to_string(idx_spot_light) + "].inner_cut_off", it->second->get_inner_cut_off());
        lighting_shader->setFloat("spotLights[" + std::to_string(idx_spot_light) + "].outer_cut_off", it->second->get_outer_cut_off());
    }

    // Render the game objects with the selected lighting shading and also render the unique Color IDs of each game object
    // (later for the selection technique: Color Picking)

    lighting_shader->setInt("is_transform3d", 0);
    lighting_shader->setFloat("exposure", exposure);

    for (auto it = game_objects.begin(); it != game_objects.end(); it++) {
        GameObject* game_object = it->second;
        game_object->draw(lighting_shader, false);
    }

    // Render only the last selected object (if it exists) to later outline the shape of this object
    unsigned int attachments2[1] = { GL_COLOR_ATTACHMENT3 };
    glDrawBuffers(1, attachments2);
    selection_shader->use();
    selection_shader->setMat4("view_projection", view_projection);
    if (last_selected_object != nullptr) {
        last_selected_object->draw(selection_shader, true);
    }

    // Draw skybox
    unsigned int attachments3[1] = { GL_COLOR_ATTACHMENT0 }; // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    glDrawBuffers(1, attachments3);
    view_skybox = glm::mat4(glm::mat3(view)); // remove translation from the view matrix so it doesn't affect the skybox
    view_projection_skybox = projection * view_skybox;
    skybox_shader->use();
    skybox_shader->setInt("is_hdr", true);
    skybox_shader->setFloat("exposure", exposure);
    skybox_shader->setMat4("view_projection", view_projection_skybox);
    cubemap->draw(skybox_shader, "hdri_cubemap");

    // Draw border outlining the selected object (if it exists one)
    outline_shader->use();
    outline_shader->setVec2("pixel_size", glm::vec2(1.0f / user_interface->texture_viewport_width, 1.0f / user_interface->texture_viewport_height));
    outline_shader->setVec3("outline_color", outline_color);
    screen_quad->draw(outline_shader, texture_selected_color_buffer, true);

    // Clear the depth buffer so the Transform3D is drawn over everything
    glClear(GL_DEPTH_BUFFER_BIT);

    // Draw 3D transforms if there is a selected object
    glDrawBuffers(3, attachments1);
    lighting_shader->use();
    lighting_shader->setInt("is_transform3d", 1);
    if (last_selected_object != nullptr) {
        transform3d->update_model_matrices(last_selected_object);
        transform3d->draw(lighting_shader);
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
    delete pbr_shader;
    delete equirectangularToCubemapShader;
    delete irradianceShader;
    delete prefilterShader;
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