#pragma once

#include "game_object.h"
#include "camera.h"
#include "rendering.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

enum TransformType {
    TRANSLATION,
    ROTATION,
    SCALING
};

class Transform3D {
public:
    TransformType type; // translation, rotation, scaling
    float scale_transform3d;
    std::unordered_map<std::string, GameObject*> translation_game_objects;
    std::unordered_map<std::string, GameObject*> rotation_game_objects;
    std::unordered_map<std::string, GameObject*> scaling_game_objects;

    Transform3D(TransformType type = TRANSLATION, float scale_transform3d = 0.05) {
        this->type = type;
        this->scale_transform3d = scale_transform3d;

        // Translation game objects
        GameObject* z_arrow_body_translation = new GameObject("z_arrow_body_translation", "cylinder", glm::vec3(0.0f, 0.0f, 1.5f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.1f, 0.1f, 3.0f), glm::vec3(0.0f, 0.0f, 1.0f), false, true, true);
        GameObject* z_arrow_head_translation = new GameObject("z_arrow_head_translation", "cone", glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.3f, 0.3f, 0.6f), glm::vec3(0.0f, 0.0f, 1.0f), false, true, true);
        GameObject* y_arrow_body_translation = new GameObject("y_arrow_body_translation", "cylinder", glm::vec3(0.0f, 1.5f, 0.0f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.1f, 0.1f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), false, true, true);
        GameObject* y_arrow_head_translation = new GameObject("y_arrow_head_translation", "cone", glm::vec3(0.0f, 3.0f, 0.0f), glm::vec3(-90.0f, 0.0f, 0.0f), glm::vec3(0.3f, 0.3f, 0.6f), glm::vec3(0.0f, 1.0f, 0.0f), false, true, true);
        GameObject* x_arrow_body_translation = new GameObject("x_arrow_body_translation", "cylinder", glm::vec3(1.5f, 0.0f, 0.0f), glm::vec3(0.0f, 90.0f, 0.0f), glm::vec3(0.1f, 0.1f, 3.0f), glm::vec3(1.0f, 0.0f, 0.0f), false, true, true);
        GameObject* x_arrow_head_translation = new GameObject("x_arrow_head_translation", "cone", glm::vec3(3.0f, 0.0f, 0.0f), glm::vec3(0.0f, 90.0f, 0.0f), glm::vec3(0.3f, 0.3f, 0.6f), glm::vec3(1.0f, 0.0f, 0.0f), false, true, true);
    
        translation_game_objects[x_arrow_body_translation->name] = x_arrow_body_translation;
        translation_game_objects[x_arrow_head_translation->name] = x_arrow_head_translation;
        translation_game_objects[y_arrow_body_translation->name] = y_arrow_body_translation;
        translation_game_objects[y_arrow_head_translation->name] = y_arrow_head_translation;
        translation_game_objects[z_arrow_body_translation->name] = z_arrow_body_translation;
        translation_game_objects[z_arrow_head_translation->name] = z_arrow_head_translation;


        // Scaling game objects
        GameObject* z_arrow_body_scaling = new GameObject("z_arrow_body_scaling", "cylinder", glm::vec3(0.0f, 0.0f, 1.5f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.1f, 0.1f, 3.0f), glm::vec3(0.0f, 0.0f, 1.0f), false, true, true);
        GameObject* z_arrow_head_scaling = new GameObject("z_arrow_head_scaling", "cube", glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.6f, 0.6f, 0.6f), glm::vec3(0.0f, 0.0f, 1.0f), false, true, true);
        GameObject* y_arrow_body_scaling = new GameObject("y_arrow_body_scaling", "cylinder", glm::vec3(0.0f, 1.5f, 0.0f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.1f, 0.1f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), false, true, true);
        GameObject* y_arrow_head_scaling = new GameObject("y_arrow_head_scaling", "cube", glm::vec3(0.0f, 3.0f, 0.0f), glm::vec3(-90.0f, 0.0f, 0.0f), glm::vec3(0.6f, 0.6f, 0.6f), glm::vec3(0.0f, 1.0f, 0.0f), false, true, true);
        GameObject* x_arrow_body_scaling = new GameObject("x_arrow_body_scaling", "cylinder", glm::vec3(1.5f, 0.0f, 0.0f), glm::vec3(0.0f, 90.0f, 0.0f), glm::vec3(0.1f, 0.1f, 3.0f), glm::vec3(1.0f, 0.0f, 0.0f), false, true, true);
        GameObject* x_arrow_head_scaling = new GameObject("x_arrow_head_scaling", "cube", glm::vec3(3.0f, 0.0f, 0.0f), glm::vec3(0.0f, 90.0f, 0.0f), glm::vec3(0.6f, 0.6f, 0.6f), glm::vec3(1.0f, 0.0f, 0.0f), false, true, true);
        GameObject* cube_center_scaling = new GameObject("cube_center_scaling", "cube", glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.8f, 0.8f, 0.8f), glm::vec3(1.0f, 1.0f, 1.0f), false, true, true);

        scaling_game_objects[z_arrow_body_scaling->name] = z_arrow_body_scaling;
        scaling_game_objects[x_arrow_body_scaling->name] = x_arrow_body_scaling;
        scaling_game_objects[y_arrow_body_scaling->name] = y_arrow_body_scaling;

        scaling_game_objects[cube_center_scaling->name] = cube_center_scaling;

        scaling_game_objects[z_arrow_head_scaling->name] = z_arrow_head_scaling;
        scaling_game_objects[x_arrow_head_scaling->name] = x_arrow_head_scaling;
        scaling_game_objects[y_arrow_head_scaling->name] = y_arrow_head_scaling;


        Rendering* rendering = Rendering::get_instance();
        for (auto it = translation_game_objects.begin(); it != translation_game_objects.end(); it++) {
            rendering->id_color_to_game_object_transform3d[it->second->id_color] = it->second;
        }
        for (auto it = rotation_game_objects.begin(); it != rotation_game_objects.end(); it++) {
            rendering->id_color_to_game_object_transform3d[it->second->id_color] = it->second;
        }
        for (auto it = scaling_game_objects.begin(); it != scaling_game_objects.end(); it++) {
            rendering->id_color_to_game_object_transform3d[it->second->id_color] = it->second;
        }
    }

    ~Transform3D() {
        for (auto it = translation_game_objects.begin(); it != translation_game_objects.end(); it++) {
            delete it->second;
        }
        for (auto it = rotation_game_objects.begin(); it != rotation_game_objects.end(); it++) {
            delete it->second;
        }
        for (auto it = scaling_game_objects.begin(); it != scaling_game_objects.end(); it++) {
            delete it->second;
        }
    }

    void transform(const glm::vec2& transform_vector) {
        const float VELOCITY_TRANSFORMATION = 0.001f;
        Rendering* rendering = Rendering::get_instance();
        GameObject* game_object_transform3d = rendering->last_selected_object_transform3d;
        if (rendering->last_selected_object == nullptr) {
            std::cout << "Error: No game object selected available to transform" << std::endl;
            return;
        }
        glm::mat4 model_view_projection = rendering->view_projection * rendering->last_selected_object->model;
        float lenght_camera_to_game_object = glm::length(rendering->camera_viewport->Position - rendering->last_selected_object->position);

        if (type == TRANSLATION) {
            glm::vec4 axis;
            if (game_object_transform3d->name == "x_arrow_body_translation" || game_object_transform3d->name == "x_arrow_head_translation") {
                axis = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
            }
            else if (game_object_transform3d->name == "y_arrow_body_translation" || game_object_transform3d->name == "y_arrow_head_translation") {
                axis = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
            }
            else { // game_object_transform3d->name == "z_arrow_body_translation" || game_object_transform3d->name == "z_arrow_head_translation"
                axis = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
            }
            glm::vec2 arrow(model_view_projection * axis);
            arrow = glm::normalize(arrow);
            float delta_transformation = glm::dot(arrow, transform_vector);
            glm::vec4 direction = rendering->last_selected_object->model * axis;
            rendering->last_selected_object->position += glm::vec3(direction) * delta_transformation * lenght_camera_to_game_object * VELOCITY_TRANSFORMATION;
            rendering->last_selected_object->set_model_matrices_standard();
        }
        else if (type == ROTATION) {

        }
        else { // SCALING
            glm::vec4 axis;
            if (game_object_transform3d->name == "x_arrow_body_scaling" || game_object_transform3d->name == "x_arrow_head_scaling") {
                axis = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
            }
            else if (game_object_transform3d->name == "y_arrow_body_scaling" || game_object_transform3d->name == "y_arrow_head_scaling") {
                axis = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
            }
            else if (game_object_transform3d->name == "z_arrow_body_scaling" || game_object_transform3d->name == "z_arrow_head_scaling") {
                axis = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
            }
            else { // game_object_transform3d->name == "cube_center_scaling"
                axis = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
            }
            glm::vec2 arrow(model_view_projection * axis);
            arrow = glm::normalize(arrow);
            float delta_transformation = glm::dot(arrow, transform_vector);
            rendering->last_selected_object->scale += glm::vec3(axis) * delta_transformation * lenght_camera_to_game_object * VELOCITY_TRANSFORMATION;
            rendering->last_selected_object->set_model_matrices_standard();
        }
    }

    void set_highlight(bool active) {
        Rendering* rendering = Rendering::get_instance();
        GameObject* game_object = rendering->last_selected_object_transform3d;
        if (type == TRANSLATION) {
            if (game_object->name == "x_arrow_body_translation" || game_object->name == "x_arrow_head_translation") {
                if (active) {
                    translation_game_objects["x_arrow_body_translation"]->color = glm::vec3(1.0f, 1.0f, 0.0f);
                    translation_game_objects["x_arrow_head_translation"]->color = glm::vec3(1.0f, 1.0f, 0.0f);
                }
                else {
                    translation_game_objects["x_arrow_body_translation"]->color = glm::vec3(1.0f, 0.0f, 0.0f);
                    translation_game_objects["x_arrow_head_translation"]->color = glm::vec3(1.0f, 0.0f, 0.0f);
                }
            }
            else if (game_object->name == "y_arrow_body_translation" || game_object->name == "y_arrow_head_translation") {
                if (active) {
                    translation_game_objects["y_arrow_body_translation"]->color = glm::vec3(1.0f, 1.0f, 0.0f);
                    translation_game_objects["y_arrow_head_translation"]->color = glm::vec3(1.0f, 1.0f, 0.0f);
                }
                else {
                    translation_game_objects["y_arrow_body_translation"]->color = glm::vec3(0.0f, 1.0f, 0.0f);
                    translation_game_objects["y_arrow_head_translation"]->color = glm::vec3(0.0f, 1.0f, 0.0f);
                }
            }
            else if (game_object->name == "z_arrow_body_translation" || game_object->name == "z_arrow_head_translation") {
                if (active) {
                    translation_game_objects["z_arrow_body_translation"]->color = glm::vec3(1.0f, 1.0f, 0.0f);
                    translation_game_objects["z_arrow_head_translation"]->color = glm::vec3(1.0f, 1.0f, 0.0f);
                }
                else {
                    translation_game_objects["z_arrow_body_translation"]->color = glm::vec3(0.0f, 0.0f, 1.0f);
                    translation_game_objects["z_arrow_head_translation"]->color = glm::vec3(0.0f, 0.0f, 1.0f);
                }
            }
        }
        else if (type == ROTATION) {

        }
        else { // SCALING
            if (game_object->name == "x_arrow_body_scaling" || game_object->name == "x_arrow_head_scaling") {
                if (active) {
                    scaling_game_objects["x_arrow_body_scaling"]->color = glm::vec3(1.0f, 1.0f, 0.0f);
                    scaling_game_objects["x_arrow_head_scaling"]->color = glm::vec3(1.0f, 1.0f, 0.0f);
                }
                else {
                    scaling_game_objects["x_arrow_body_scaling"]->color = glm::vec3(1.0f, 0.0f, 0.0f);
                    scaling_game_objects["x_arrow_head_scaling"]->color = glm::vec3(1.0f, 0.0f, 0.0f);
                }
            }
            else if (game_object->name == "y_arrow_body_scaling" || game_object->name == "y_arrow_head_scaling") {
                if (active) {
                    scaling_game_objects["y_arrow_body_scaling"]->color = glm::vec3(1.0f, 1.0f, 0.0f);
                    scaling_game_objects["y_arrow_head_scaling"]->color = glm::vec3(1.0f, 1.0f, 0.0f);
                }
                else {
                    scaling_game_objects["y_arrow_body_scaling"]->color = glm::vec3(0.0f, 1.0f, 0.0f);
                    scaling_game_objects["y_arrow_head_scaling"]->color = glm::vec3(0.0f, 1.0f, 0.0f);
                }
            }
            else if (game_object->name == "z_arrow_body_scaling" || game_object->name == "z_arrow_head_scaling") {
                if (active) {
                    scaling_game_objects["z_arrow_body_scaling"]->color = glm::vec3(1.0f, 1.0f, 0.0f);
                    scaling_game_objects["z_arrow_head_scaling"]->color = glm::vec3(1.0f, 1.0f, 0.0f);
                }
                else {
                    scaling_game_objects["z_arrow_body_scaling"]->color = glm::vec3(0.0f, 0.0f, 1.0f);
                    scaling_game_objects["z_arrow_head_scaling"]->color = glm::vec3(0.0f, 0.0f, 1.0f);
                }
            }
            else { // (game_object->name == "cube_center_scaling")
                if (active) {
                    scaling_game_objects["cube_center_scaling"]->color = glm::vec3(1.0f, 1.0f, 0.0f);
                }
                else {
                    scaling_game_objects["cube_center_scaling"]->color = glm::vec3(1.0f, 1.0f, 1.0f);
                }
            }
        }
    }

    void update_model_matrices(GameObject* parent) {
        Rendering* rendering = Rendering::get_instance();
        std::unordered_map<std::string, GameObject*>* transformation_game_objects;
        if (type == TRANSLATION) {
            transformation_game_objects = &translation_game_objects;
        }
        else if (type == ROTATION) {
            transformation_game_objects = &rotation_game_objects;
        }
        else { // SCALING
            transformation_game_objects = &scaling_game_objects;
        }
        for (auto it = transformation_game_objects->begin(); it != transformation_game_objects->end(); it++) {
            GameObject* game_object = it->second;
            glm::mat4& model = game_object->model;
            glm::vec3& position = game_object->position;
            glm::vec3& rotation = game_object->rotation;
            glm::vec3& scale = game_object->scale;
            glm::mat4& model_inv = game_object->model_inv;
            glm::mat3& model_normals = game_object->model_normals;
            float distance_camera_to_parent_object = glm::length(rendering->camera_viewport->Position - parent->position);

            model = glm::mat4(1.0f);
            model = glm::translate(model, parent->position);
            model = glm::rotate(model, glm::radians(parent->rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(parent->rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, glm::radians(parent->rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

            model = glm::translate(model, position * distance_camera_to_parent_object * scale_transform3d);
            model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::scale(model, scale * distance_camera_to_parent_object * scale_transform3d);

            model_inv = glm::inverse(model);

            model_normals = glm::mat3(glm::transpose(model_inv));
        }
    }

    void draw(Shader* shader) {
        if (type == TRANSLATION) { // Draw 3D translation axes
            for (auto it = translation_game_objects.begin(); it != translation_game_objects.end(); it++) {
                it->second->draw(shader, false);
            }
        }
        else if (type == ROTATION) {
            for (auto it = rotation_game_objects.begin(); it != rotation_game_objects.end(); it++) {
                it->second->draw(shader, false);
            }
        }
        else { // SCALING
            for (auto it = scaling_game_objects.begin(); it != scaling_game_objects.end(); it++) {
                it->second->draw(shader, false);
            }
        }
    }
};