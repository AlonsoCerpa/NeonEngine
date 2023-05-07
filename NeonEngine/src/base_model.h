#pragma once

#include <glm/glm.hpp>
#include <string>
#include <iostream>
#include <assimp/Importer.hpp>
#include <vector>

class Shader;
class Rendering;
class Material;

struct Bone {
    aiMatrix4x4 offset_matrix;
    aiMatrix4x4 final_transformation;
};

class BaseModel {
public:
    std::string name;
    std::vector<Bone> bones;

    virtual void draw(Shader* shader, Material* draw_material, bool is_selected, bool disable_depth_test, bool render_only_ambient, bool render_one_color) = 0;
    virtual bool intersected_ray(const glm::vec3& orig, const glm::vec3& dir, float& t) = 0;

    virtual void update_bone_transformations(float animation_time_in_seconds, int animation_id) {
        std::cout << "Trying to do animations on a model with no skeletal animations support" << std::endl;
    }
};