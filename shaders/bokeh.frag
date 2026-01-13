#version 450 core

in vec2 vUV;
out vec4 FragColor;

uniform sampler2D u_HighlightTexture;
uniform sampler2D uApertureTex;
uniform float u_Radius;
uniform vec2 u_Resolution;

float hash(vec2 p)
{
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

// TODO readme.md #4 - aperture shape
// Funkcja: getApertureWeight(offset) -> float
// Sample aperture texture (Sec.5.4.1 Jeong 2022)
float getApertureWeight(vec2 offset) {
    vec2 uv = offset * 0.5 + 0.5;
    float mask = texture(uApertureTex, uv).r;
    return mask;
}

void main()
{
    vec2 texel = 1.0 / u_Resolution;

    vec3 sum = vec3(0.0);
    float weightSum = 0.0;

    const int SAMPLES = 32; 
    const float GOLDEN_ANGLE = 2.39996323;

    float radiusPx = u_Radius * u_Resolution.y * 0.01;
    radiusPx = min(radiusPx, 40.0);

    float randAngle = hash(vUV * u_Resolution) * 6.2831853;

    for (int i = 0; i < SAMPLES; ++i)
    {
        float r = sqrt((float(i) + 0.5) / float(SAMPLES));
        float a = float(i) * GOLDEN_ANGLE + randAngle;

        vec2 offset =
            vec2(cos(a), sin(a)) *
            r * radiusPx * texel;

        vec3 color =
            texture(u_HighlightTexture, vUV + offset).rgb;

        float diskMask = smoothstep(1.0, 0.95, r);
        float apertureWeight = getApertureWeight(vec2(cos(a), sin(a)) * r);

        float w = diskMask * apertureWeight;

        sum += color * color * w;
        weightSum += w;
    }

    vec3 result = sum / max(weightSum, 0.0001);
    result = sqrt(result);

    FragColor = vec4(result, 1.0);
}
