#pragma once

#include "shader.h"

#include <glad/glad.h> // holds all OpenGL type declarations
#include <vector>

class Quad {
public:
    unsigned int VAO, VBO;
    std::vector<float> vertices;

    ~Quad() {

    }

	Quad() {
		vertices = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
            // positions   // texCoords
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
             1.0f, -1.0f,  1.0f, 0.0f,

            -1.0f,  1.0f,  0.0f, 1.0f,
             1.0f, -1.0f,  1.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 1.0f
        };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

        glBindVertexArray(0);
	}

	void draw(Shader* shader, unsigned int texture_color_buffer, bool disable_depth_test) {
        if (disable_depth_test) {
            glDisable(GL_DEPTH_TEST);
        }

        glActiveTexture(GL_TEXTURE0);
        shader->setInt("screen_texture", 0);
        glBindTexture(GL_TEXTURE_2D, texture_color_buffer);	// use the color attachment texture as the texture of the quad plane

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        if (disable_depth_test) {
            glEnable(GL_DEPTH_TEST);
        }
	}
};