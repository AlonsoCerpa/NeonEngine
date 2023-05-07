#include "game_object.h"

#include "shader.h"
#include "rendering.h"
#include "base_model.h"
#include "transform3d.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <chrono>

const int MAX_NUMBER_BONES = 200;

//////////////////////////////// GAME_OBJECT_TYPE //////////////////////////////////////

std::string game_object_type_to_string(GameObjectType type) {
    std::string game_object_type;
    if (type == TypeBaseModel) {
        game_object_type = "BaseModel";
    }
    else if (type == TypePointLight) {
        game_object_type = "PointLight";
    }
    else if (type == TypeDirectionalLight) {
        game_object_type = "DirectionalLight";
    }
    else if (type == TypeSpotLight) {
        game_object_type = "SpotLight";
    }
    else {
        game_object_type = "UnrecognizedType";
    }
    return game_object_type;
}

//////////////////////////////// GAME_OBJECT //////////////////////////////////////

ColorGenerator* GameObject::color_generator = new ColorGenerator(100000);

GameObject::GameObject(const std::string& name, const std::string& model_name) {
    this->name = name;
    this->model_name = model_name;
    type = TypeBaseModel;
    position = glm::vec3(0.0f);
    rotation = glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    scale = glm::vec3(1.0f);
    albedo = glm::vec3(1.0f);
    metalness = 0.9f;
    roughness = 0.1f;
    emission = glm::vec3(0.0f);
    animation_id = -1;
    is_selected = false;
    render_only_ambient = false;
    render_one_color = false;
    material = nullptr;
    id_color = color_generator->generate_color();

    set_model_matrices_standard();
}

GameObject::~GameObject() {

}

void GameObject::set_model_matrices_standard() {
    model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model *= glm::mat4_cast(rotation);
    model = glm::scale(model, scale);

    model_inv = glm::inverse(model);

    model_normals = glm::mat3(glm::transpose(model_inv));
}

void GameObject::draw(Shader* shader, bool disable_depth_test) {
    Rendering* rendering = Rendering::get_instance();

    shader->setVec3("albedo_model", albedo);
    shader->setFloat("metalness_model", metalness);
    shader->setFloat("roughness_model", roughness);
    shader->setVec3("emission_model", emission);

    shader->setMat4("model", model);
    shader->setMat3("model_normals", model_normals);

    glUniform3ui(glGetUniformLocation(shader->ID, "id_color_game_object"), id_color.r, id_color.g, id_color.b);

    if (model_name != "") {
        if (this->animation_id != -1) { // There is an animation specified for the model of this game object
            auto current_time = std::chrono::system_clock::now();
            std::chrono::duration<float> elapsed_seconds = current_time - rendering->time_before_rendering;
            rendering->loaded_models[model_name]->update_bone_transformations(elapsed_seconds.count(), this->animation_id);
            shader->setInt("is_animated", true);
            assert(rendering->loaded_models[model_name]->bones.size() <= MAX_NUMBER_BONES);
            for (int i = 0; i < rendering->loaded_models[model_name]->bones.size(); i++) {
                glm::mat4 glm_matrix;
                std::memcpy(glm::value_ptr(glm_matrix), &(rendering->loaded_models[model_name]->bones[i].final_transformation), sizeof(aiMatrix4x4));
                glm_matrix = glm::transpose(glm_matrix);
                shader->setMat4("bone_transforms[" + std::to_string(i) + "]", glm_matrix);
            }
        }
        else {
            shader->setInt("is_animated", false);
        }
        rendering->loaded_models[model_name]->draw(shader, material, is_selected, disable_depth_test, render_only_ambient, render_one_color);
    }
}

bool GameObject::intersected_ray(const glm::vec3& ray_dir, const glm::vec3& camera_position, float& t) {
    Rendering* rendering = Rendering::get_instance();
    glm::vec3 ray_dir_model = model_inv * glm::vec4(ray_dir, 0.0f);
    glm::vec3 ray_origin_model = model_inv * glm::vec4(camera_position, 1.0f);
    if (model_name != "" && rendering->loaded_models[model_name]->intersected_ray(ray_origin_model, ray_dir_model, t)) {
        return true;
    }
    return false;
}

void GameObject::set_select_state(bool is_game_obj_selected) {
    is_selected = is_game_obj_selected;
}

void GameObject::clean() {
    delete color_generator;
}

//////////////////////////////// LIGHTS //////////////////////////////////////

Light::Light(const std::string& name, const std::string& model_name) : GameObject(name, model_name) {
    ambient = glm::vec3(0.05f);
    diffuse = glm::vec3(0.8f);
    specular = glm::vec3(1.0f);
}

PointLight::PointLight(const std::string& name, const std::string& model_name) : Light(name, model_name) {
    constant = 1.0f;
    linear = 0.045f;
    quadratic = 0.0075f;
}

DirectionalLight::DirectionalLight(const std::string& name, const std::string& model_name) : Light(name, model_name) {
    direction = glm::vec3(-1.0f);
}

SpotLight::SpotLight(const std::string& name, const std::string& model_name) : Light(name, model_name) {
    direction = glm::vec3(0.0f, 0.0f, -1.0f);

    inner_cut_off_angle = 12.5f;
    outer_cut_off_angle = 15.0f;

    constant = 1.0f;
    linear = 0.09f;
    quadratic = 0.032f;
}

float SpotLight::get_inner_cut_off() {
    return glm::cos(glm::radians(inner_cut_off_angle));
}

float SpotLight::get_outer_cut_off() {
    return glm::cos(glm::radians(outer_cut_off_angle));
}