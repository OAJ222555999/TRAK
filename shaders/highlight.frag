#version 450 core

in vec2 vUV;
out vec4 FragColor;

uniform sampler2D u_SceneTexture;
uniform float u_Threshold;

float luminance(vec3 c)
{
    return dot(c, vec3(0.2126, 0.7152, 0.0722));
}

// TODO readme.md #3 - blur vs bokeh
// Funkcja: isBokehSource(lum, coc) -> bool
// Wymaga: depth texture z COC
bool isBokehSource(float lum) {
    return lum > u_Threshold;
}

void main()
{
    vec3 color = texture(u_SceneTexture, vUV).rgb;
    float lum = luminance(color);

    float highlight = isBokehSource(lum) ? max(lum - u_Threshold, 0.0) : 0.0;


    FragColor = vec4(color * highlight*2.5, 1.0);
}
