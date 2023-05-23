#version 460 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 IdColor;
layout (location = 2) out vec4 IdColorTransform3d;
layout (location = 3) out vec4 BrightColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

// material parameters
uniform sampler2D texture_albedo;
uniform sampler2D texture_normal;
uniform sampler2D texture_metalness;
uniform sampler2D texture_roughness;
uniform sampler2D texture_emission;
uniform sampler2D texture_ambient_occlusion;

uniform int has_texture_albedo;
uniform int has_texture_normal;
uniform int has_texture_metalness;
uniform int has_texture_roughness;
uniform int has_texture_emission;
uniform int has_texture_ambient_occlusion;

uniform float intensity;
uniform vec3 albedo_model;
uniform float metalness_model;
uniform float roughness_model;
uniform vec3 emission_model;

uniform float emission_strength;

// IBL
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

const float PI = 3.14159265359;

struct PointLight {
    vec3 position;

    vec3 light_color;
    float intensity;
	
    vec3 ambient;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct DirectionalLight {
    vec3 direction;

    vec3 light_color;
    float intensity;
	
    vec3 ambient;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;

    vec3 light_color;
    float intensity;

    vec3 ambient;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;

    float inner_cut_off;
    float outer_cut_off;    
};

uniform vec3 viewPos;

uniform int num_point_lights;
uniform int num_directional_lights;
uniform int num_spot_lights;

const int MAX_POINT_LIGHTS = 20;
const int MAX_DIRECTIONAL_LIGHTS = 5;
const int MAX_SPOT_LIGHTS = 10;

uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform DirectionalLight directionalLights[MAX_DIRECTIONAL_LIGHTS];
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];

uniform int render_only_ambient;
uniform int render_one_color;
uniform uvec3 id_color_game_object;
uniform int is_transform3d;

uniform int material_format;

// Material/File formats
const int Default = 0;
const int glTF = 1;
const int FBX = 2;

// Function declarations
vec3 getNormalFromMap();
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);

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
            // Used in particular for rendering lights with bloom
            FragColor = vec4(albedo_model * intensity, 1.0);
        }
        else {
            FragColor = vec4(vec3(texture(texture_albedo, TexCoords)), 1.0);
        }
	}
    else {
        // material properties
        vec3 albedo = albedo_model;
        if (has_texture_albedo == 1) {
            albedo = pow(texture(texture_albedo, TexCoords).rgb, vec3(2.2));
        }
        vec3 N = Normal;
        if (has_texture_normal == 1) {
            N = getNormalFromMap();
        }
        float metallic = metalness_model;
        if (has_texture_metalness == 1) {
            if (material_format == glTF) { // glTF format
                metallic = texture(texture_metalness, TexCoords).b;
            }
            else {
                metallic = texture(texture_metalness, TexCoords).r;
            }
        }
        float roughness = roughness_model;
        if (has_texture_roughness == 1) {
            if (material_format == glTF) { // glTF format
                roughness = texture(texture_roughness, TexCoords).g;
            }
            else {
                roughness = texture(texture_roughness, TexCoords).r;
            }
        }
        vec3 emission = emission_model;
        if (has_texture_emission == 1) {
            emission = pow(texture(texture_emission, TexCoords).rgb, vec3(2.2));
        }
        float ambient_occlusion = 1.0;
        if (has_texture_ambient_occlusion == 1) {
            ambient_occlusion = texture(texture_ambient_occlusion, TexCoords).r;
        }
       
        // input lighting data
        vec3 V = normalize(viewPos - FragPos);
        vec3 R = reflect(-V, N);

        // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
        // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
        vec3 F0 = vec3(0.04); 
        F0 = mix(F0, albedo, metallic);

        // reflectance equation
        vec3 Lo = vec3(0.0);

        // POINT LIGHTS
        for (int i = 0; i < num_point_lights; ++i) {
            // calculate per-light radiance
            vec3 L = normalize(pointLights[i].position - FragPos);
            vec3 H = normalize(V + L);
            // attenuation
            float dist = length(pointLights[i].position - FragPos);
            float attenuation = 1.0 / (pointLights[i].constant + pointLights[i].linear * dist + pointLights[i].quadratic * (dist * dist));
            // radiance
            vec3 radiance = pointLights[i].light_color * pointLights[i].intensity * attenuation;

            // Cook-Torrance BRDF
            float NDF = DistributionGGX(N, H, roughness);
            float G   = GeometrySmith(N, V, L, roughness);    
            vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);        
        
            vec3 numerator    = NDF * G * F;
            float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
            vec3 specular = numerator / denominator;
        
             // kS is equal to Fresnel
            vec3 kS = F;
            // for energy conservation, the diffuse and specular light can't
            // be above 1.0 (unless the surface emits light); to preserve this
            // relationship the diffuse component (kD) should equal 1.0 - kS.
            vec3 kD = vec3(1.0) - kS;
            // multiply kD by the inverse metalness such that only non-metals 
            // have diffuse lighting, or a linear blend if partly metal (pure metals
            // have no diffuse light).
            kD *= 1.0 - metallic;	                
            
            // scale light by NdotL
            float NdotL = max(dot(N, L), 0.0);        

            // add to outgoing radiance Lo
            Lo += (kD * albedo / PI + specular) * radiance * NdotL; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
        }
        
        // DIRECTIONAL LIGHTS
        for (int i = 0; i < num_directional_lights; ++i) {
            // calculate per-light radiance
            vec3 L = normalize(-directionalLights[i].direction);
            vec3 H = normalize(V + L);
            // radiance
            vec3 radiance = directionalLights[i].light_color * directionalLights[i].intensity;

            // Cook-Torrance BRDF
            float NDF = DistributionGGX(N, H, roughness);   
            float G   = GeometrySmith(N, V, L, roughness);    
            vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);        
        
            vec3 numerator    = NDF * G * F;
            float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
            vec3 specular = numerator / denominator;
        
             // kS is equal to Fresnel
            vec3 kS = F;
            // for energy conservation, the diffuse and specular light can't
            // be above 1.0 (unless the surface emits light); to preserve this
            // relationship the diffuse component (kD) should equal 1.0 - kS.
            vec3 kD = vec3(1.0) - kS;
            // multiply kD by the inverse metalness such that only non-metals 
            // have diffuse lighting, or a linear blend if partly metal (pure metals
            // have no diffuse light).
            kD *= 1.0 - metallic;	                
            
            // scale light by NdotL
            float NdotL = max(dot(N, L), 0.0);        

            // add to outgoing radiance Lo
            Lo += (kD * albedo / PI + specular) * radiance * NdotL; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
        }

        // SPOT LIGHTS
        for (int i = 0; i < num_spot_lights; ++i) {
            // calculate per-light radiance
            vec3 L = normalize(spotLights[i].position - FragPos);
            vec3 H = normalize(V + L);
            // attenuation
            float dist = length(spotLights[i].position - FragPos);
            float attenuation = 1.0 / (spotLights[i].constant + spotLights[i].linear * dist + spotLights[i].quadratic * (dist * dist));
            // spotlight intensity
            float theta = dot(L, normalize(-spotLights[i].direction)); 
            float epsilon = spotLights[i].inner_cut_off - spotLights[i].outer_cut_off;
            float intensity = clamp((theta - spotLights[i].outer_cut_off) / epsilon, 0.0, 1.0);
            // radiance
            vec3 radiance = spotLights[i].light_color * spotLights[i].intensity * attenuation * intensity;

            // Cook-Torrance BRDF
            float NDF = DistributionGGX(N, H, roughness);
            float G   = GeometrySmith(N, V, L, roughness);    
            vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);        
        
            vec3 numerator    = NDF * G * F;
            float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
            vec3 specular = numerator / denominator;
        
             // kS is equal to Fresnel
            vec3 kS = F;
            // for energy conservation, the diffuse and specular light can't
            // be above 1.0 (unless the surface emits light); to preserve this
            // relationship the diffuse component (kD) should equal 1.0 - kS.
            vec3 kD = vec3(1.0) - kS;
            // multiply kD by the inverse metalness such that only non-metals 
            // have diffuse lighting, or a linear blend if partly metal (pure metals
            // have no diffuse light).
            kD *= 1.0 - metallic;	                
            
            // scale light by NdotL
            float NdotL = max(dot(N, L), 0.0);        

            // add to outgoing radiance Lo
            Lo += (kD * albedo / PI + specular) * radiance * NdotL; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
        }
    
        // ambient lighting (we now use IBL as the ambient term)
        vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    
        vec3 kS = F;
        vec3 kD = 1.0 - kS;
        kD *= 1.0 - metallic;	  
    
        vec3 irradiance = texture(irradianceMap, N).rgb;
        vec3 diffuse      = irradiance * albedo;
    
        // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
        const float MAX_REFLECTION_LOD = 4.0;
        vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;    
        vec2 brdf  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
        vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

        vec3 ambient = (kD * diffuse + specular) * ambient_occlusion;
    
        vec3 color = ambient + Lo;

        FragColor = vec4(color + emission_strength * emission , 1.0);
    }
    if (isinf(FragColor.x) == true || isinf(FragColor.y) == true || isinf(FragColor.z) == true ||
        isnan(FragColor.x) == true || isnan(FragColor.y) == true || isnan(FragColor.z) == true) {
        FragColor = vec4(0.0001, 0.0001, 0.0001, 1.0);
    }

    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(FragColor.rgb, 1.0);
	else
		BrightColor = vec4(0.0001, 0.0001, 0.0001, 1.0);

    // For lights, we render the object with only the albedo color, but the bright color takes
    // into consideration both the albedo color and the intensity
    if (render_one_color == 1 && render_only_ambient == 1) {
        FragColor = vec4(albedo_model, 1.0);
    }
}

// Easy trick to get tangent-normals to world-space to keep PBR code simplified.
// Don't worry if you don't get what's going on; you generally want to do normal 
// mapping the usual way for performance anways; I do plan make a note of this 
// technique somewhere later in the normal mapping tutorial.
vec3 getNormalFromMap() {
    vec3 tangentNormal = texture(texture_normal, TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(FragPos);
    vec3 Q2  = dFdy(FragPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N   = normalize(Normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}