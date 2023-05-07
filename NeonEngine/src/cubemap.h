#pragma once

#include "shader.h"

#include <string>
#include <iostream>
#include <glad/glad.h>
#include <stb_image.h>

unsigned int load_cubemap_textures(const std::vector<std::string>& cubemap_textures);

class Cubemap {
public:
    unsigned int VAO;
    std::unordered_map<std::string, unsigned int> umap_name_to_texture_id;

    Cubemap(const std::string& cubemap_texture_name, const std::vector<std::string>& cubemap_textures) {
        add_cubemap_texture(cubemap_texture_name, cubemap_textures);

        unsigned int VBO;

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        glBufferData(GL_ARRAY_BUFFER, vertex_data.size() * sizeof(float), &(vertex_data[0]), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

        glBindVertexArray(0);
    }

    void draw(Shader* shader, const std::string& cubemap_texture_name) {
        glActiveTexture(GL_TEXTURE0);
        shader->setInt("cubemap", 0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, umap_name_to_texture_id[cubemap_texture_name]);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        
        glBindVertexArray(VAO);        
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        glDepthFunc(GL_LESS); // set depth function back to default
    }

    void add_cubemap_texture(const std::string& cubemap_texture_name, const std::vector<std::string>& cubemap_textures) {
        umap_name_to_texture_id[cubemap_texture_name] = load_cubemap_textures(cubemap_textures);
    }

    void add_cubemap_texture(const std::string& cubemap_texture_name, int texture_id) {
        umap_name_to_texture_id[cubemap_texture_name] = texture_id;
    }
    
private:
    std::vector<float> vertex_data = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };
};

// Loads the 6 textures of a cubemap in the following order:
// +X (right), -X (left), +Y (top), -Y (bottom), +Z (front), -Z (back)
unsigned int load_cubemap_textures(const std::vector<std::string>& cubemap_textures) {
    unsigned int texture_id;

    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);

    int width, height, number_channels;
    for (int i = 0; i < cubemap_textures.size(); i++) {
        unsigned char* data = stbi_load(cubemap_textures[i].c_str(), &width, &height, &number_channels, 0);
        GLenum format;
        if (number_channels == 1) {
            format = GL_RED;
        }
        else if (number_channels == 3) {
            format = GL_RGB;
        }
        else { // number_channels == 4
            format = GL_RGBA;
        }
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else {
            std::cout << "Cubemap texture failed to load at path: " << cubemap_textures[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    return texture_id;
}