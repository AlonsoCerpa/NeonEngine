#pragma once

#include <glm/glm.hpp>
#include <string>

class Shader;
class Rendering;

class Model {
public:
    std::string name;

    virtual void draw(Shader* shader, bool is_selected, bool disable_depth_test, bool render_only_ambient, bool render_one_color) = 0;
    virtual bool intersected_ray(const glm::vec3& orig, const glm::vec3& dir, float& t) = 0;
};