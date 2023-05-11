#pragma once

#include <unordered_map>
#include <string>
#include <vector>

class Shader;

unsigned int load_cubemap_textures(const std::vector<std::string>& cubemap_textures);

enum CubemapTextureType {
    EnvironmentMap,
    IrradianceMap,
    PrefilterMap
};

std::string cubemap_texture_type_to_string(CubemapTextureType type);

struct CubemapData {
    unsigned int environment_texture;
    unsigned int irradiance_texture;
    unsigned int prefilter_texture;
    bool is_hdri;
};

class Cubemap {
public:
    unsigned int VAO;
    std::unordered_map<std::string, CubemapData> umap_name_to_cubemap_data;

    Cubemap();
    void draw(Shader* shader, const std::string& cubemap_texture_name, CubemapTextureType type);
    void add_cubemap_texture(const std::string& cubemap_texture_name, const std::vector<std::string>& cubemap_textures, bool is_hdri);
    void add_cubemap_texture(const std::string& cubemap_texture_name, int texture_id, bool is_hdri);
    
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