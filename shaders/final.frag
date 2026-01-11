#version 450 core

in vec2 vUV;
out vec4 FragColor;

uniform sampler2D u_DOFTexture;
uniform sampler2D u_BokehTexture;

// TODO readme.md #7 - energy conservation
// Funkcja: blendBokeh(dof, bokeh) -> color
// Energy-conservative blend (Sec.4.1 Jeong 2022)
vec3 blendBokeh(vec3 dof, vec3 bokeh) {
    return dof + bokeh;
}

void main()
{
    vec3 dofColor   = texture(u_DOFTexture, vUV).rgb;
    vec3 bokehColor = texture(u_BokehTexture, vUV).rgb;

    vec3 color = blendBokeh(dofColor, bokehColor);

    FragColor = vec4(color, 1.0);
}
