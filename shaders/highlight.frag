#version 450 core

in vec2 vUV;
out vec4 FragColor;

uniform sampler2D u_SceneTexture;
uniform sampler2D u_DepthTexture;
uniform float u_Threshold;
uniform float u_CocThreshold;    // piksele, np. 4.0

// Parametry thin lens (te same co w dof.frag)
uniform float u_FocalLength;
uniform float u_Aperture;
uniform float u_SensorWidth;
uniform float u_FocusDepth;
uniform float u_NearPlane;
uniform float u_FarPlane;
uniform vec2 u_Resolution;

float luminance(vec3 c)
{
    return dot(c, vec3(0.2126, 0.7152, 0.0722));
}

// Konwersja normalized depth do world-space distance
float linearizeDepth(float d) {
    return u_NearPlane * u_FarPlane / (u_FarPlane - d * (u_FarPlane - u_NearPlane));
}

// Eq.7 z Jeong 2022 - COC w pikselach
float computeCoC(float depth) {
    float d = linearizeDepth(depth);
    float d_f = linearizeDepth(u_FocusDepth);
    float F = u_FocalLength;
    float E = F / u_Aperture;
    
    if (abs(d_f - F) < 0.0001 || abs(d) < 0.0001) {
        return 0.0;
    }
    
    float cocMeters = (E * F / (d_f - F)) * ((d - d_f) / d);
    return abs(cocMeters) * (u_Resolution.x / u_SensorWidth);
}

// TODO readme.md #3 - blur vs bokeh
// Sec.4.1 Jeong 2022: highlight = jasny piksel Z duzym COC
bool isBokehSource(float lum, float coc) {
    return lum > u_Threshold && coc > u_CocThreshold;
}

void main()
{
    vec3 color = texture(u_SceneTexture, vUV).rgb;
    float lum = luminance(color);
    float depth = texture(u_DepthTexture, vUV).r;
    float coc = computeCoC(depth);

    float highlight = isBokehSource(lum, coc) ? max(lum - u_Threshold, 0.0) : 0.0;

    FragColor = vec4(color * highlight * 2.5, 1.0);
}
