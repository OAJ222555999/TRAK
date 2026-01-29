#version 450 core

in vec3 vNormal;
in vec2 vUV;

out vec4 FragColor;

// textures
uniform sampler2D u_Albedo;
uniform sampler2D u_EmissiveMap;

// lighting
uniform vec3 u_LightDir;     // światło główne (np. księżyc)
uniform vec3 u_LightColor;
uniform float u_LightStrength;

// emissive
uniform vec3 u_EmissiveColor;
uniform float u_EmissiveStrength;

void main()
{

    // ALBEDO + ALPHA CUTOUT
    vec4 tex = texture(u_Albedo, vUV);
    if (tex.a < 0.5)
        discard; // usuwamy fragmenty z przezroczystością poniżej 0.5

    vec3 albedo = tex.rgb;

    // normal
    vec3 N = normalize(vNormal);
    vec3 L = normalize(u_LightDir);

    // diffuse light
    float NdotL = max(dot(N, L), 0.0);
    vec3 diffuse = albedo * u_LightColor * NdotL * u_LightStrength;

    vec3 ambient = albedo * 0.15;

    // emissive (latarenka)
    vec3 emissive = 
        texture(u_EmissiveMap, vUV).rgb *
        u_EmissiveColor *
        u_EmissiveStrength;

    vec3 color = ambient + diffuse + emissive;

    FragColor = vec4(color, 1.0);
}
