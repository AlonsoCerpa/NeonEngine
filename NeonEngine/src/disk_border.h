#pragma once

#include "model.h"
#include "shader.h"

#include <vector>
#include <glad/glad.h>
#define _USE_MATH_DEFINES
#include <math.h>

class DiskBorder : public Model {
public:
    unsigned int VAO;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    DiskBorder(const std::string& name, float max_angle = M_PI / 2.0f, float inner_radius = 1.0f, float outer_radius = 2.0f, int sector_count = 100) {
        this->name = name;
        set(max_angle, sector_count, inner_radius, outer_radius);

        GLuint vbo, ebo;

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        // vertex Positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        // vertex normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(sizeof(float) * 3));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void set(float max_angle, int sector_count, float inner_radius, float outer_radius) {
        float x, y, z = 0.0f, theta;
        float sector_step = max_angle / sector_count;
        float nx = 0.0f, ny = 0.0f, nz = 1.0f;
        for (int i = 0; i <= sector_count; i++) {
            theta = sector_step * i;
            x = inner_radius * cos(theta);
            y = inner_radius * sin(theta);

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            vertices.push_back(nx);
            vertices.push_back(ny);
            vertices.push_back(nz);
        }
        for (int i = 0; i <= sector_count; i++) {
            theta = sector_step * i;
            x = outer_radius * cos(theta);
            y = outer_radius * sin(theta);

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            vertices.push_back(nx);
            vertices.push_back(ny);
            vertices.push_back(nz);
        }

        int k1 = 0;
        int k2 = sector_count + 1;
        for (int i = 0; i < sector_count; i++, k1++, k2++) {
            indices.push_back(k1);
            indices.push_back(k2);
            indices.push_back(k2 + 1);

            indices.push_back(k1);
            indices.push_back(k2 + 1);
            indices.push_back(k1 + 1);
        }
    }

    bool intersected_ray(const glm::vec3& orig, const glm::vec3& dir, float& t) {
        return false;
    }

    void draw(Shader* shader, bool is_selected, bool disable_depth_test, bool render_only_ambient, bool render_one_color) {
        if (disable_depth_test) {
            glDisable(GL_DEPTH_TEST);
        }

        shader->setInt("render_only_ambient", render_only_ambient);
        shader->setInt("render_one_color", render_one_color);
        shader->setInt("paint_selected_texture", is_selected);

        // draw mesh
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // always good practice to set everything back to defaults once configured.
        glActiveTexture(GL_TEXTURE0);

        if (disable_depth_test) {
            glEnable(GL_DEPTH_TEST);
        }
    }
};