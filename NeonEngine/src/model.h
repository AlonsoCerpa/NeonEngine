#pragma once

#include "mesh.h"
#include "base_model.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <unordered_map>

FileFormat get_format_from_path(const std::string& path);
unsigned int TextureFromFile(const char* path, const std::string& directory, int& num_channels, bool gamma = false);
unsigned int EmbeddedTextureFromFile(const aiTexture* texture, int& num_channels);

struct ModelNode {
    std::string name;
    aiMatrix4x4 transformation;
    std::vector<ModelNode*> children;

    ~ModelNode() {
        for (int i = 0; i < children.size(); i++) {
            delete children[i];
        }
    }
};

struct NodeAnimation {
    std::map<double, aiVector3D> map_time_to_position;
    std::map<double, aiQuaternion> map_time_to_rotation;
    std::map<double, aiVector3D> map_time_to_scaling;
};

struct Animation {
    std::string name;
    double ticks_per_second;
    double duration;
    std::unordered_map<std::string, NodeAnimation> umap_node_name_to_channels;
};

class Model : public BaseModel
{
public:
    // Model data 
    std::map<std::string, Texture*> loaded_textures;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    std::map<unsigned int, Material*> loaded_materials;
    std::vector<Mesh> meshes;
    std::string directory;
    FileFormat format;
    bool gammaCorrection;

    // Data for bones
    aiMatrix4x4 global_inverse_transform;
    std::unordered_map<std::string, int> umap_bone_name_to_id;
    std::vector<Animation> animations;
    ModelNode* root_node;

    Model(const std::string& name, std::string const& path, bool gamma = false, bool set_flip_vertically = true);
    ~Model();
    void draw(Shader* shader, Material* draw_material, bool is_selected, bool disable_depth_test, bool render_only_ambient, bool render_one_color);
    NodeAnimation* find_node_animation(Animation& animation, const std::string& node_name);
    void update_bones_recursively(float animation_time_in_ticks, Animation& animation, ModelNode* node, const aiMatrix4x4& parent_transform);
    void update_bone_transformations(float animation_time_in_seconds, int animation_id);
    //void update_bone_transformations_blended(float animation_time_in_seconds, int animation_id1, int animation_id2, float blend_factor);
    void calculate_interpolated_transformation(aiVector3D& transformation, float animation_time_in_ticks, std::map<double, aiVector3D>& map_time_to_transform);
    void calculate_interpolated_transformation(aiQuaternion& transformation, float animation_time_in_ticks, std::map<double, aiQuaternion>& map_time_to_transform);
    bool intersected_ray(const glm::vec3& orig, const glm::vec3& dir, float& t);

private:
    void loadModel(std::string const& path);
    void processNode(aiNode* node, const aiScene* scene, ModelNode* model_node);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    void print_loaded_textures(const std::map<std::string, Texture*>& loaded_textures);
    std::vector<Texture*> loadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureType texture_type, const aiScene* scene, const std::string& material_name);
};