#include "cubemap.h"

#include "shader.h"

#include <glad/glad.h>
#include <iostream>
#include <stb_image.h>

Cubemap::Cubemap() {
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

void Cubemap::draw(Shader* shader, const std::string& cubemap_texture_name, CubemapTextureType type) {
    unsigned int texture_id;
    if (type == EnvironmentMap) {
        texture_id = umap_name_to_cubemap_data[cubemap_texture_name].environment_texture;
    }
    else if (type == IrradianceMap) {
        texture_id = umap_name_to_cubemap_data[cubemap_texture_name].irradiance_texture;
    }
    else { // type == PrefilterMap
        texture_id = umap_name_to_cubemap_data[cubemap_texture_name].prefilter_texture;
    }

    glActiveTexture(GL_TEXTURE0);
    shader->setInt("cubemap", 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glDepthFunc(GL_LESS); // set depth function back to default
}

void Cubemap::add_cubemap_texture(const std::string& cubemap_texture_name, const std::vector<std::string>& cubemap_textures, bool is_hdri) {
    CubemapData cubemap_data;
    cubemap_data.environment_texture = load_cubemap_textures(cubemap_textures);
    cubemap_data.is_hdri = is_hdri;
    umap_name_to_cubemap_data[cubemap_texture_name] = cubemap_data;
}

void Cubemap::add_cubemap_texture(const std::string& cubemap_texture_name, int texture_id, bool is_hdri) {
    CubemapData cubemap_data;
    cubemap_data.environment_texture = texture_id;
    cubemap_data.is_hdri = is_hdri;
    umap_name_to_cubemap_data[cubemap_texture_name] = cubemap_data;
}



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

std::string cubemap_texture_type_to_string(CubemapTextureType type) {
    if (type == EnvironmentMap) {
        return "EnvironmentMap";
    }
    else if (type == IrradianceMap) {
        return "IrradianceMap";
    }
    else { // type == PrefilterMap
        return "PrefilterMap";
    }
}