#pragma once

#include "shader.h"
#include "rendering.h"
#include "model.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class GameObject {
public:
    int idx_loaded_models = -1;
    glm::vec3 position;
    glm::vec3 scale;
    glm::vec3 rotation;
    glm::mat4 model;
    glm::mat4 model_inv;
    glm::mat3 model_normals;
    glm::vec3 color;
    bool is_selected;
    std::vector<GameObject*> children_game_objects;

    GameObject(int idx_loaded_models = 0, const glm::vec3& position = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
               const glm::vec3& rotation = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3& color = glm::vec3(0.0f, 0.0f, 0.0f), bool is_selected = false) {
        this->idx_loaded_models = idx_loaded_models;
        this->position = position;
        this->scale = scale;
        this->rotation = rotation;
        this->color = color;
        this->is_selected = is_selected;

        set_model_matrices_standard();
    }

    ~GameObject() {
        for (int i = 0; i < children_game_objects.size(); i++) {
            delete children_game_objects[i];
        }
    }

    void set_model_matrices_standard() {
        model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, scale);

        model_inv = glm::inverse(model);

        model_normals = glm::mat3(glm::transpose(model_inv));
    }

    void set_model_matrices_type1(GameObject* parent) {
        model = glm::mat4(1.0f);
        model = glm::translate(model, parent->position);
        model = glm::rotate(model, glm::radians(parent->rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(parent->rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(parent->rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

        model = glm::translate(model, position * parent->scale);
        model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, scale * parent->scale);

        model_inv = glm::inverse(model);

        model_normals = glm::mat3(glm::transpose(model_inv));
    }

    void draw(Shader* shader, Rendering* rendering) {
        shader->setVec3("model_color", color);
        shader->setMat4("model", model);
        shader->setMat3("model_normals", model_normals);
        shader->setMat4("model_view_projection", rendering->projection * rendering->view * model);
        if (idx_loaded_models != -1) {
            rendering->loaded_models[idx_loaded_models]->draw(*shader, is_selected, rendering);
        }
        for (int i = 0; i < children_game_objects.size(); i++) {
            children_game_objects[i]->draw(shader, rendering);
        }
    }

    bool intersected_ray(Rendering* rendering, const glm::vec3& ray_dir, const glm::vec3& camera_position, float& t) {
        glm::vec3 ray_dir_model = model_inv * glm::vec4(ray_dir, 0.0f);
        glm::vec3 ray_origin_model = model_inv * glm::vec4(camera_position, 1.0f);
        if (idx_loaded_models != -1 && rendering->loaded_models[idx_loaded_models]->intersected_ray(ray_origin_model, ray_dir_model, t)) {
            return true;
        }
        for (int i = 0; i < children_game_objects.size(); i++) {
            if (children_game_objects[i]->intersected_ray(rendering, ray_dir, camera_position, t)) {
                return true;
            }
        }
        return false;
    }

    void set_select_state(bool is_game_obj_selected) {
        is_selected = is_game_obj_selected;
        for (int i = 0; i < children_game_objects.size(); i++) {
            children_game_objects[i]->set_select_state(is_game_obj_selected);
        }
    }
};

class Light : public GameObject {
public:
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    Light() {}

    Light(int idx_loaded_models, const glm::vec3& position, const glm::vec3& scale, const glm::vec3& rotation, const glm::vec3& color, bool is_selected,
          const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular)
          : GameObject(idx_loaded_models, position, scale, rotation, color, is_selected) {
        this->ambient = ambient;
        this->diffuse = diffuse;
        this->specular = specular;
    }
};

class PointLight : public Light {
public:
    float constant;
    float linear;
    float quadratic;

    PointLight(int idx_loaded_models, const glm::vec3& position, const glm::vec3& scale, const glm::vec3& rotation, const glm::vec3& color, bool is_selected,
               const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, float constant, float linear, float quadratic)
               : Light(idx_loaded_models, position, scale, rotation, color, is_selected, ambient, diffuse, specular) {
        this->constant = constant;
        this->linear = linear;
        this->quadratic = quadratic;
    }
};

class DirectionalLight : public Light {
public:
    glm::vec3 direction;

    DirectionalLight(int idx_loaded_models, const glm::vec3& position, const glm::vec3& scale, const glm::vec3& rotation, const glm::vec3& color, bool is_selected,
                     const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, const glm::vec3& direction)
                     : Light(idx_loaded_models, position, scale, rotation, color, is_selected, ambient, diffuse, specular) {
        this->direction = direction;
    }
};

class SpotLight : public Light {
public:
    glm::vec3 direction;

    float inner_cut_off;
    float outer_cut_off;

    float constant;
    float linear;
    float quadratic;

    SpotLight(int idx_loaded_models, const glm::vec3& position, const glm::vec3& scale, const glm::vec3& rotation, const glm::vec3& color, bool is_selected,
              const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, const glm::vec3& direction, float inner_cut_off, float outer_cut_off,
              float constant, float linear, float quadratic)
              : Light(idx_loaded_models, position, scale, rotation, color, is_selected, ambient, diffuse, specular) {
        this->direction = direction;

        this->inner_cut_off = inner_cut_off;
        this->outer_cut_off = inner_cut_off;

        this->constant = constant;
        this->linear = linear;
        this->quadratic = quadratic;
    }
};