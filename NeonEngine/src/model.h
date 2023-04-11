#pragma once

#include "mesh.h"
#include "shader.h"
#include "base_model.h"

#include <glad/glad.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>

unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma = false);

class Model : public BaseModel
{
public:
    Assimp::Importer importer;
    const aiScene* scene;

    // model data 
    std::vector<Texture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    std::vector<Mesh> meshes;
    std::string directory;
    bool gammaCorrection;

    // Data for bones
    aiMatrix4x4 global_inverse_transform;
    std::unordered_map<std::string, int> umap_bone_name_to_id;

    // constructor, expects a filepath to a 3D model.
    Model(const std::string& name, std::string const& path, bool gamma = false, bool set_flip_vertically = true) : gammaCorrection(gamma)
    {
        auto start_time = std::chrono::system_clock::now();
        stbi_set_flip_vertically_on_load(set_flip_vertically);
        this->name = name;
        loadModel(path);
        auto end_time = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end_time - start_time;
        std::cout << "Model \"" << name << "\" loaded in " << elapsed_seconds.count() << " seconds" << std::endl;
    }

    // draws the model, and thus all its meshes
    void draw(Shader* shader, bool is_selected, bool disable_depth_test, bool render_only_ambient, bool render_one_color)
    {
        for (unsigned int i = 0; i < meshes.size(); i++) {
            meshes[i].draw(shader, is_selected, disable_depth_test, render_only_ambient, render_one_color);
        }
    }

    aiNodeAnim* find_node_animation(const aiAnimation* animation, const std::string& node_name) {
        for (int i = 0; i < animation->mNumChannels; i++) {
            aiNodeAnim* node_animation = animation->mChannels[i];

            if (std::string(node_animation->mNodeName.data) == node_name) {
                return node_animation;
            }
        }
        return nullptr;
    }

    void update_bones_recursively(float animation_time_in_ticks, const aiAnimation* animation, const aiNode* node, const aiMatrix4x4& parent_transform) {
        std::string node_name(node->mName.data);
        aiMatrix4x4 node_transformation(node->mTransformation);
        const aiNodeAnim* node_animation = find_node_animation(animation, node_name);

        if (node_animation) {
            // Interpolate scaling and generate scaling transformation matrix
            aiVector3D scaling;
            calculate_interpolated_transformation(scaling, animation_time_in_ticks, node_animation, node_animation->mNumScalingKeys, node_animation->mScalingKeys);
            aiMatrix4x4 scaling_matrix;
            scaling_matrix = aiMatrix4x4::Scaling(aiVector3D(scaling.x, scaling.y, scaling.z), scaling_matrix);

            // Interpolate rotation and generate rotation transformation matrix
            aiQuaternion rotation_quaternion;
            calculate_interpolated_transformation(rotation_quaternion, animation_time_in_ticks, node_animation);
            aiMatrix4x4 rotation_matrix(rotation_quaternion.GetMatrix());

            // Interpolate translation and generate translation transformation matrix
            aiVector3D translation;
            calculate_interpolated_transformation(translation, animation_time_in_ticks, node_animation, node_animation->mNumPositionKeys, node_animation->mPositionKeys);
            aiMatrix4x4 translation_matrix;
            translation_matrix = aiMatrix4x4::Translation(aiVector3D(translation.x, translation.y, translation.z), translation_matrix);

            // Combine the above transformations
            node_transformation = translation_matrix * rotation_matrix * scaling_matrix;
        }

        aiMatrix4x4 global_transformation = parent_transform * node_transformation;

        if (umap_bone_name_to_id.find(node_name) != umap_bone_name_to_id.end()) {
            int bone_id = umap_bone_name_to_id[node_name];
            bones[bone_id].final_transformation = global_inverse_transform * global_transformation * bones[bone_id].offset_matrix;
        }

        for (int i = 0; i < node->mNumChildren; i++) {
            update_bones_recursively(animation_time_in_ticks, animation, node->mChildren[i], global_transformation);
        }
    }

    void update_bone_transformations(float animation_time_in_seconds, int animation_id) {
        aiMatrix4x4 identity;
        assert(animation_id < scene->mNumAnimations);
        float ticks_per_second = (float)(scene->mAnimations[animation_id]->mTicksPerSecond != 0 ? scene->mAnimations[animation_id]->mTicksPerSecond : 25.0f);
        float animation_time_in_ticks = animation_time_in_seconds * ticks_per_second;
        animation_time_in_ticks = fmod(animation_time_in_ticks, (float)scene->mAnimations[animation_id]->mDuration);

        update_bones_recursively(animation_time_in_ticks, scene->mAnimations[animation_id], scene->mRootNode, identity);
    }

    // For translations and scalings
    void calculate_interpolated_transformation(aiVector3D& transformation, float animation_time_in_ticks, const aiNodeAnim* node_animation,
                                               unsigned int num_keys, aiVectorKey* keys) {
        // we need at least two values to interpolate...
        if (num_keys == 1) {
            transformation = keys[0].mValue;
            return;
        }

        unsigned int transformation_index = find_transformation(animation_time_in_ticks, node_animation, num_keys, keys);
        unsigned int next_transformation_index = transformation_index + 1;
        assert(next_transformation_index < num_keys);
        float t1 = (float)keys[transformation_index].mTime;
        float t2 = (float)keys[next_transformation_index].mTime;
        float delta_time = t2 - t1;
        float factor = (animation_time_in_ticks - t1) / delta_time;
        assert(factor >= 0.0f && factor <= 1.0f);
        const aiVector3D& start = keys[transformation_index].mValue;
        const aiVector3D& end = keys[next_transformation_index].mValue;
        aiVector3D delta = end - start;
        transformation = start + factor * delta;
    }

    // For translations and scalings
    unsigned int find_transformation(float animation_time_in_ticks, const aiNodeAnim* node_animation, unsigned int num_keys, aiVectorKey* keys) {
        for (int i = 0; i < num_keys - 1; i++) {
            float t = (float)keys[i + 1].mTime;
            if (animation_time_in_ticks < t) {
                return i;
            }
        }
        return 0;
    }

    // For rotations
    void calculate_interpolated_transformation(aiQuaternion& transformation, float animation_time_in_ticks, const aiNodeAnim* node_animation) {
        // we need at least two values to interpolate...
        if (node_animation->mNumRotationKeys == 1) {
            transformation = node_animation->mRotationKeys[0].mValue;
            return;
        }

        unsigned int rotation_index = find_transformation(animation_time_in_ticks, node_animation);
        unsigned int next_rotation_index = rotation_index + 1;
        assert(next_rotation_index < node_animation->mNumRotationKeys);
        float t1 = (float)node_animation->mRotationKeys[rotation_index].mTime;
        float t2 = (float)node_animation->mRotationKeys[next_rotation_index].mTime;
        float delta_time = t2 - t1;
        float factor = (animation_time_in_ticks - t1) / delta_time;
        assert(factor >= 0.0f && factor <= 1.0f);
        const aiQuaternion& start_rotation_q = node_animation->mRotationKeys[rotation_index].mValue;
        const aiQuaternion& end_rotation_q = node_animation->mRotationKeys[next_rotation_index].mValue;
        aiQuaternion::Interpolate(transformation, start_rotation_q, end_rotation_q, factor);
        transformation.Normalize();
    }

    // For rotations
    unsigned int find_transformation(float animation_time_in_ticks, const aiNodeAnim* node_animation) {
        int num_keys = node_animation->mNumRotationKeys;
        aiQuatKey* keys = node_animation->mRotationKeys;
        for (int i = 0; i < num_keys - 1; i++) {
            float t = (float)keys[i + 1].mTime;
            if (animation_time_in_ticks < t) {
                return i;
            }
        }
        return 0;
    }

    bool intersected_ray(const glm::vec3& orig, const glm::vec3& dir, float& t) {
        for (unsigned int i = 0; i < meshes.size(); i++) {
            if (meshes[i].intersected_ray(orig, dir, t)) {
                return true;
            }
        }
        return false;
    }

private:
    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(std::string const& path)
    {
        // read file via ASSIMP
        this->scene = this->importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices);
        // check for errors
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            std::cout << "ERROR::ASSIMP:: " << this->importer.GetErrorString() << std::endl;
            return;
        }

        // retrieve the directory path of the filepath
        this->directory = path.substr(0, path.find_last_of('/'));

        // get global inverse transform out of the root node, for processing of bone animations
        this->global_inverse_transform = scene->mRootNode->mTransformation;
        this->global_inverse_transform = this->global_inverse_transform.Inverse();

        // We process each node, starting at the root node, and recursing downwards
        // Inside the processing of each node, we process all of the meshes of the current node
        // Inside the processing of each mesh, we process all the vertices of the mesh, the material of the mesh
        // and the bone data related to this mesh.
        processNode(scene->mRootNode, scene);
    }

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode* node, const aiScene* scene)
    {
        // process each mesh located at the current node
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            // the node object only contains indices to index the actual objects in the scene. 
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }
    }

    Mesh processMesh(aiMesh* mesh, const aiScene* scene)
    {
        // data to fill
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;

        // walk through each of the mesh's vertices
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
            // positions
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;
            // normals
            if (mesh->HasNormals())
            {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.Normal = vector;
            }
            // texture coordinates
            if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
            {
                glm::vec2 vec;
                // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
                // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
                // tangent
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.Tangent = vector;
                // bitangent
                vector.x = mesh->mBitangents[i].x;
                vector.y = mesh->mBitangents[i].y;
                vector.z = mesh->mBitangents[i].z;
                vertex.Bitangent = vector;
            }
            else
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);

            vertices.push_back(vertex);
        }
        // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            // retrieve all indices of the face and store them in the indices vector
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }
        // process materials
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
        // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
        // Same applies to other texture as the following list summarizes:
        // diffuse: texture_diffuseN
        // specular: texture_specularN
        // normal: texture_normalN
        // height: texture_heightN

        // 1. diffuse maps
        std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        // 2. specular maps
        std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        // 3. normal maps
        std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        // 4. height maps
        std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());


        // Process bones
        for (int i = 0; i < mesh->mNumBones; i++) {
            int bone_id = 0;
            std::string bone_name(mesh->mBones[i]->mName.data);
            if (umap_bone_name_to_id.find(bone_name) == umap_bone_name_to_id.end()) {
                bone_id = umap_bone_name_to_id.size();
                umap_bone_name_to_id[bone_name] = bone_id;
                Bone bone;
                bone.offset_matrix = mesh->mBones[i]->mOffsetMatrix;
                bones.push_back(bone);
            }
            else {
                bone_id = umap_bone_name_to_id[bone_name];
            }

            for (int j = 0; j < mesh->mBones[i]->mNumWeights; j++) {
                unsigned int vertex_id = mesh->mBones[i]->mWeights[j].mVertexId;
                float weight = mesh->mBones[i]->mWeights[j].mWeight;

                if (weight == 0.0f) {
                    continue;
                }

                bool added_bone_weight = false;
                for (int k = 0; k < MAX_BONE_INFLUENCE && !added_bone_weight; k++) {
                    if (vertices[vertex_id].BoneIds[k] == bone_id && vertices[vertex_id].BoneWeights[k] != 0.0f) {
                        added_bone_weight = true;
                    }
                    else if (vertices[vertex_id].BoneWeights[k] == 0.0f) {
                        vertices[vertex_id].BoneIds[k] = bone_id;
                        vertices[vertex_id].BoneWeights[k] = weight;
                        added_bone_weight = true;
                    }
                }
                if (!added_bone_weight) {
                    std::cout << "ERROR: NOT ENOUGH SPACE TO ADD BONE WEIGHT, INCREASE VARIABLE: MAX_BONE_INFLUENCE" << std::endl;
                    //assert(0);
                }
            }
        }

        // return a mesh object created from the extracted mesh data
        return Mesh(vertices, indices, textures);
    }

    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
    {
        std::vector<Texture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
            bool skip = false;
            for (unsigned int j = 0; j < textures_loaded.size(); j++)
            {
                if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                    break;
                }
            }
            if (!skip)
            {   // if texture hasn't been loaded already, load it
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecessary load duplicate textures.
            }
        }
        return textures;
    }
};


unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma)
{
    std::string filename = std::string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format = GL_RGB;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}