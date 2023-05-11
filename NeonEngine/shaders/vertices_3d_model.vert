#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 5) in ivec4 aBoneIds;
layout (location = 6) in vec4 aBoneWeights;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat3 model_normals;
uniform mat4 view_projection;

const int MAX_NUMBER_BONES = 200;

uniform mat4 bone_transforms[MAX_NUMBER_BONES];
uniform int is_animated;

void main() {
    mat4 BoneTransform = bone_transforms[aBoneIds[0]] * aBoneWeights[0];
    BoneTransform += bone_transforms[aBoneIds[1]] * aBoneWeights[1];
    BoneTransform += bone_transforms[aBoneIds[2]] * aBoneWeights[2];
    BoneTransform += bone_transforms[aBoneIds[3]] * aBoneWeights[3];

    vec4 PosLocal = vec4(aPos, 1.0);
    vec4 NormalLocal = vec4(aNormal, 0.0);
    if (is_animated == 1) {
        PosLocal = BoneTransform * PosLocal;
        NormalLocal = BoneTransform * NormalLocal;
    }
    FragPos = vec3(model * PosLocal);
    Normal = model_normals * vec3(NormalLocal);
    TexCoords = aTexCoords;
    gl_Position = view_projection * vec4(FragPos, 1.0);
}