#version 450 core

in vec2 vUV;
out vec4 FragColor;

uniform sampler2D u_ColorTexture;
uniform sampler2D u_DepthTexture;
uniform vec2 u_Resolution;

uniform float u_FocusDepth;
uniform float u_BlurStrength;

// Parametry thin lens (Sec.5.1 Jeong 2022)
uniform float u_FocalLength;   // ogniskowa [m], np. 0.050
uniform float u_Aperture;      // f-number, np. 1.4
uniform float u_SensorWidth;   // szerokosc sensora [m], np. 0.036
uniform float u_NearPlane;
uniform float u_FarPlane;

// Konwersja normalized depth [0,1] do world-space distance
float linearizeDepth(float d) {
    return u_NearPlane * u_FarPlane / (u_FarPlane - d * (u_FarPlane - u_NearPlane));
}

// TODO readme.md #1,#2 - COC z thin lens
// Eq.7 z Jeong 2022, Sec.5.1:
// C(d, d_f) = (E*F / (d_f - F)) * ((d - d_f) / d)
// gdzie: E = lens radius, F = focal length, d_f = focus distance, d = object distance
float computeCoC(float depth) {
    float d = linearizeDepth(depth);              // object distance [m]
    float d_f = linearizeDepth(u_FocusDepth);     // focus distance [m]
    float F = u_FocalLength;                       // focal length [m]
    float E = F / u_Aperture;                      // lens radius [m]
    
    // Zabezpieczenie przed dzieleniem przez zero
    if (abs(d_f - F) < 0.0001 || abs(d) < 0.0001) {
        return 0.0;
    }
    
    // Eq.7: C = (E*F / (d_f - F)) * ((d - d_f) / d)
    float cocMeters = (E * F / (d_f - F)) * ((d - d_f) / d);
    
    // Konwersja metrow na piksele
    float cocPixels = abs(cocMeters) * (u_Resolution.x / u_SensorWidth);
    
    return cocPixels;
}

void main()
{
    float depth = texture(u_DepthTexture, vUV).r;
    vec3 sharp = texture(u_ColorTexture, vUV).rgb;

    float coc = computeCoC(depth);

    // COC jest juz w pikselach (Eq.7), blurStrength jako mnoznik kontrolny
    float radiusPx = coc * u_BlurStrength;
    radiusPx = min(radiusPx, 50.0);

    if (radiusPx < 0.5)
    {
        FragColor = vec4(sharp, 1.0);
        return;
    }

    vec2 texel = 1.0 / u_Resolution;

    vec3 sum = vec3(0.0);
    float count = 0.0;

    for (int x = -2; x <= 2; ++x)
    {
        for (int y = -2; y <= 2; ++y)
        {
            vec2 offset = vec2(x, y) * texel * radiusPx * 0.25;
            sum += texture(u_ColorTexture, vUV + offset).rgb;
            count += 1.0;
        }
    }

    vec3 blurred = sum / count;

    FragColor = vec4(blurred, 1.0);
}
