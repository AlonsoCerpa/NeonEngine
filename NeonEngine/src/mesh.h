#pragma once

#include "geometry.h"
#include "shader.h"

#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>
#include <set>
#include <map>

const int MAX_BONE_INFLUENCE = 4;

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
    int BoneIds[MAX_BONE_INFLUENCE] = {0};
    float BoneWeights[MAX_BONE_INFLUENCE] = {0.0f};
};

enum TextureType {
    TexAlbedo, TexNormal, TexMetalness, TexRoughness, TexEmission, TexAmbientOcclusion, TexSpecular, TexLast
};

struct Texture {
public:
    unsigned int id;
    std::set<TextureType> types;
    std::string path;
    int num_channels;

    Texture(const std::string& base_name) {
        this->base_name = base_name;
    }

    static std::string get_short_name_of_texture_type(TextureType texture_type) {
        if (texture_type == TexAlbedo) {
            return "alb";
        }
        else if (texture_type == TexNormal) {
            return "norm";
        }
        else if (texture_type == TexMetalness) {
            return "metal";
        }
        else if (texture_type == TexRoughness) {
            return "rough";
        }
        else if (texture_type == TexEmission) {
            return "emi";
        }
        else if (texture_type == TexAmbientOcclusion) {
            return "occ";
        }
        else {
            return "spec";
        }
    }

    static std::string get_long_name_of_texture_type(TextureType texture_type) {
        if (texture_type == TexAlbedo) {
            return "texture_albedo";
        }
        else if (texture_type == TexNormal) {
            return "texture_normal";
        }
        else if (texture_type == TexMetalness) {
            return "texture_metalness";
        }
        else if (texture_type == TexRoughness) {
            return "texture_roughness";
        }
        else if (texture_type == TexEmission) {
            return "texture_emission";
        }
        else if (texture_type == TexAmbientOcclusion) {
            return "texture_ambient_occlusion";
        }
        else {
            return "texture_specular";
        }
    }

    std::string get_name() {
        std::string output_name = base_name;
        for (auto it = types.begin(); it != types.end(); it++) {
            output_name += "_" + get_short_name_of_texture_type(*it);
        }
        return output_name;
    }

private:
    std::string base_name;
};

enum FileFormat {
    Default, glTF, FBX
};

struct Material {
    std::string name;
    std::map<TextureType, Texture*> textures;
    FileFormat format;

    Material(const std::string& name) {
        this->name = name;
    }
};

class Mesh {
public:
    // mesh Data
    std::string name;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    Material* material;
    unsigned int VAO;

    // constructor
    Mesh(const std::string& name, std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, Material* material)
    {
        this->name = name;
        this->vertices = vertices;
        this->indices = indices;
        this->material = material;

        // now that we have all the required data, set the vertex buffers and its attribute pointers.
        setupMesh();
    }

    // render the mesh
    void draw(Shader* shader, Material* draw_material, bool is_selected, bool disable_depth_test, bool render_only_ambient, bool render_one_color)
    {
        if (disable_depth_test) {
            glDisable(GL_DEPTH_TEST);
        }

        const unsigned int OFFSET_TEXTURES = 3; // Accounting for PBR indirect light textures (irradiance, prefilter and BRDF maps)

        if (draw_material == nullptr) {
            draw_material = this->material;
        }

        shader->setInt("material_format", draw_material->format);

        int num_active_textures = 0;
        for (int type = TexAlbedo; type < TexLast; type++) {
            TextureType texture_type = (TextureType) type;
            std::string str_texture_type = Texture::get_long_name_of_texture_type(texture_type);
            if (draw_material->textures.find(texture_type) != draw_material->textures.end()) {
                Texture* texture = draw_material->textures[texture_type];
                glActiveTexture(GL_TEXTURE0 + OFFSET_TEXTURES + num_active_textures);
                shader->setInt(str_texture_type, OFFSET_TEXTURES + num_active_textures);
                shader->setInt("has_" + str_texture_type, true);
                glBindTexture(GL_TEXTURE_2D, texture->id);
                num_active_textures++;
            }
            else {
                shader->setInt("has_" + str_texture_type, false);
            }
        }

        shader->setInt("render_only_ambient", render_only_ambient);
        shader->setInt("render_one_color", render_one_color);
        shader->setInt("paint_selected_texture", is_selected);

        // draw mesh
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // always good practice to set everything back to defaults once configured.
        glActiveTexture(GL_TEXTURE0);

        if (disable_depth_test) {
            glEnable(GL_DEPTH_TEST);
        }
    }

    bool intersected_ray(const glm::vec3& orig, const glm::vec3& dir, float& t) {
        float min_t = std::numeric_limits<float>::max();
        float t_aux;
        for (int i = 0; i < indices.size(); i += 3) {
            glm::vec3 v0 = vertices[i].Position;
            glm::vec3 v1 = vertices[i+1].Position;
            glm::vec3 v2 = vertices[i+2].Position;
            if (ray_triangle_intersection(orig, dir, v0, v1, v2, t_aux)) {
                if (t_aux < min_t) {
                    min_t = t_aux;
                }
            }
        }
        if (min_t != std::numeric_limits<float>::max()) {
            t = min_t;
            return true;
        }
        else {
            t = -1.0f;
            return false;
        }
    }

private:
    // render data 
    unsigned int VBO, EBO;

    // initializes all the buffer objects/arrays
    void setupMesh()
    {
        // create buffers/arrays
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        // load data into vertex buffers
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // A great thing about structs is that their memory layout is sequential for all its items.
        // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
        // again translates to 3/2 floats which translates to a byte array.
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);  

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // set the vertex attribute pointers
        // vertex positions
        glEnableVertexAttribArray(0);	
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        // vertex normals
        glEnableVertexAttribArray(1);	
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        // vertex texture coords
        glEnableVertexAttribArray(2);	
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
        // vertex tangent
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
        // vertex bitangent
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
		// vertex bone ids
		glEnableVertexAttribArray(5);
		glVertexAttribIPointer(5, MAX_BONE_INFLUENCE, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, BoneIds));
		// vertex bone weights
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, MAX_BONE_INFLUENCE, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, BoneWeights));

        glBindVertexArray(0);
    }
};