#pragma once

#include <vector>
#include <glm/glm.hpp>

class Shader;

struct TextureAndSize {
    unsigned int texture_id;
    glm::vec2 size;
};

unsigned int initialize_bloom(int num_bloom_textures, std::vector<TextureAndSize>& bloom_textures, int width, int height);
void bloom_downsampling(Shader* shader, unsigned int starting_texture_id, std::vector<TextureAndSize>& bloom_textures, int starting_width, int starting_height);
void bloom_upsampling(Shader* shader, std::vector<TextureAndSize>& bloom_textures, float filter_radius);