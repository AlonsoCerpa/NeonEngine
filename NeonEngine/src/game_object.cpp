#include "game_object.h"

#include "shader.h"
#include "rendering.h"
#include "model.h"
#include "transform3d.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//////////////////////////////// GAME_OBJECT //////////////////////////////////////

ColorGenerator* GameObject::color_generator = new ColorGenerator(100000);

GameObject::GameObject(const std::string& name, const std::string& model_name, const glm::vec3& position, const glm::vec3& rotation,
    const glm::vec3& scale, const glm::vec3& color, bool is_selected, bool render_only_ambient, bool render_one_color) {
    this->name = name;
    this->model_name = model_name;
    this->position = position;
    this->rotation = rotation;
    this->scale = scale;
    this->color = color;
    this->is_selected = is_selected;
    this->render_only_ambient = render_only_ambient;
    this->render_one_color = render_one_color;

    this->id_color = color_generator->generate_color();

    set_model_matrices_standard();
}

GameObject::~GameObject() {

}

void GameObject::set_model_matrices_standard() {
    model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, scale);

    model_inv = glm::inverse(model);

    model_normals = glm::mat3(glm::transpose(model_inv));
}

void GameObject::draw(Shader* shader, bool disable_depth_test) {
    Rendering* rendering = Rendering::get_instance();
    shader->setVec3("model_color", color);
    shader->setMat4("model", model);
    shader->setMat3("model_normals", model_normals);
    shader->setMat4("model_view_projection", rendering->view_projection * model);
    glUniform3ui(glGetUniformLocation(shader->ID, "id_color_game_object"), id_color.r, id_color.g, id_color.b);
    if (model_name != "") {
        rendering->loaded_models[model_name]->draw(shader, is_selected, disable_depth_test, render_only_ambient, render_one_color);
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

Light::Light(const std::string& name, const std::string& model_name, const glm::vec3& position, const glm::vec3& scale, const glm::vec3& rotation, const glm::vec3& color, bool is_selected, bool render_only_ambient, bool render_one_color,
    const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular)
    : GameObject(name, model_name, position, scale, rotation, color, is_selected, render_only_ambient, render_one_color) {
    this->ambient = ambient;
    this->diffuse = diffuse;
    this->specular = specular;
}

PointLight::PointLight(const std::string& name, const std::string& model_name, const glm::vec3& position, const glm::vec3& scale, const glm::vec3& rotation, const glm::vec3& color, bool is_selected, bool render_only_ambient, bool render_one_color,
    const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, float constant, float linear, float quadratic)
    : Light(name, model_name, position, scale, rotation, color, is_selected, render_only_ambient, render_one_color, ambient, diffuse, specular) {
    this->constant = constant;
    this->linear = linear;
    this->quadratic = quadratic;
}

DirectionalLight::DirectionalLight(const std::string& name, const std::string& model_name, const glm::vec3& position, const glm::vec3& scale, const glm::vec3& rotation, const glm::vec3& color, bool is_selected, bool render_only_ambient, bool render_one_color,
    const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, const glm::vec3& direction)
    : Light(name, model_name, position, scale, rotation, color, is_selected, render_only_ambient, render_one_color, ambient, diffuse, specular) {
    this->direction = direction;
}

SpotLight::SpotLight(const std::string& name, const std::string& model_name, const glm::vec3& position, const glm::vec3& scale, const glm::vec3& rotation, const glm::vec3& color, bool is_selected, bool render_only_ambient, bool render_one_color,
    const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, const glm::vec3& direction, float inner_cut_off, float outer_cut_off,
    float constant, float linear, float quadratic)
    : Light(name, model_name, position, scale, rotation, color, is_selected, render_only_ambient, render_one_color, ambient, diffuse, specular) {
    this->direction = direction;

    this->inner_cut_off = inner_cut_off;
    this->outer_cut_off = inner_cut_off;

    this->constant = constant;
    this->linear = linear;
    this->quadratic = quadratic;
}