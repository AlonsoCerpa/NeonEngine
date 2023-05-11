#version 460 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 IdColor;
layout (location = 2) out vec4 IdColorTransform3d;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform sampler2D texture_albedo;
uniform sampler2D texture_specular;

const float SHININESS = 32.0;

struct PointLight {
    vec3 position;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct DirectionalLight {
    vec3 direction;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;   

    float constant;
    float linear;
    float quadratic;

    float inner_cut_off;
    float outer_cut_off;    
};

uniform vec3 albedo_model;
uniform vec3 viewPos;

uniform int num_point_lights;
uniform int num_directional_lights;
uniform int num_spot_lights;

const int MAX_POINT_LIGHTS = 50;
const int MAX_DIRECTIONAL_LIGHTS = 10;
const int MAX_SPOT_LIGHTS = 50;

uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform DirectionalLight directionalLights[MAX_DIRECTIONAL_LIGHTS];
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];

uniform int render_only_ambient;
uniform int render_one_color;
uniform uvec3 id_color_game_object;
uniform int is_transform3d;

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main() {
    if (is_transform3d == 0) {
        IdColor = vec4(id_color_game_object/255.0, 1.0);
        IdColorTransform3d = vec4(0.0);
    }
    else {
        IdColor = vec4(0.0);
        IdColorTransform3d = vec4(id_color_game_object/255.0, 1.0);
    }
    
	if (render_only_ambient == 1) {
        if (render_one_color == 1) {
            FragColor = vec4(albedo_model, 1.0);
        }
        else {
            FragColor = vec4(vec3(texture(texture_albedo, TexCoords)), 1.0);
        }
	}
	else {
        vec3 norm = normalize(Normal);
        vec3 viewDir = normalize(viewPos - FragPos);

        vec3 result = vec3(0.0);

        for (int i = 0; i < num_point_lights; i++) {
            result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
        }
        for (int i = 0; i < num_directional_lights; i++) {
            result += CalcDirectionalLight(directionalLights[i], norm, viewDir);
        }
        for (int i = 0; i < num_spot_lights; i++) {
            result += CalcSpotLight(spotLights[i], norm, FragPos, viewDir);
        }

		FragColor = vec4(result, 1.0);
	}
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), SHININESS);
    // attenuation
    float dist = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * (dist * dist));    
    // combine results
    vec3 ambient, diffuse, specular;
    if (render_one_color == 0) {
        ambient = light.ambient * vec3(texture(texture_albedo, TexCoords));
        diffuse = light.diffuse * diff * vec3(texture(texture_albedo, TexCoords));
        specular = light.specular * spec * vec3(texture(texture_specular, TexCoords));
    }
    else {
        ambient = light.ambient * albedo_model;
        diffuse = light.diffuse * diff * albedo_model;
        specular = light.specular * spec * vec3(0.5);
    }
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

// calculates the color when using a directional light.
vec3 CalcDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), SHININESS);
    // combine results
    vec3 ambient, diffuse, specular;
    if (render_one_color == 0) {
        ambient = light.ambient * vec3(texture(texture_albedo, TexCoords));
        diffuse = light.diffuse * diff * vec3(texture(texture_albedo, TexCoords));
        specular = light.specular * spec * vec3(texture(texture_specular, TexCoords));
    }
    else {
        ambient = light.ambient * albedo_model;
        diffuse = light.diffuse * diff * albedo_model;
        specular = light.specular * spec * vec3(0.5);
    }
    return (ambient + diffuse + specular);
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), SHININESS);
    // attenuation
    float dist = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * (dist * dist));    
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.inner_cut_off - light.outer_cut_off;
    float intensity = clamp((theta - light.outer_cut_off) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient, diffuse, specular;
    if (render_one_color == 0) {
        ambient = light.ambient * vec3(texture(texture_albedo, TexCoords));
        diffuse = light.diffuse * diff * vec3(texture(texture_albedo, TexCoords));
        specular = light.specular * spec * vec3(texture(texture_specular, TexCoords));
    }
    else {
        ambient = light.ambient * albedo_model;
        diffuse = light.diffuse * diff * albedo_model;
        specular = light.specular * spec * vec3(0.5);
    }
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}