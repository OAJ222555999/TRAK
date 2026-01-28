#version 450 core
in vec2 vUV;
out vec4 FragColor;

uniform sampler2D u_HighlightTexture;
uniform sampler2D uApertureTex;
uniform float u_Radius;
uniform vec2 u_Resolution;

uniform bool u_IsLutUsed;
uniform sampler2D u_LutTexture0;
uniform sampler2D u_LutTexture1;
uniform sampler2D u_LutTexture2;
uniform sampler2D u_LutTexture3;
uniform sampler2D u_LutTexture4;
uniform sampler2D u_LutTexture5;
uniform sampler2D u_LutTexture6;
uniform sampler2D u_LutTexture7;
uniform float u_Aperture; 
uniform float u_FocusDepth;


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
    
    return mask > 0.1 ? mask : 0.0;
}


void main()
{
    vec2 texel = 1.0 / u_Resolution;
    vec3 sum = vec3(0.0);
    float weightSum = 0.0;

    const int SAMPLES_SQRT = 16;
    
    float radiusPx = u_Radius * u_Resolution.y * 0.01;
    
    float edgeDist = min(min(vUV.x, vUV.y), min(1.0 - vUV.x, 1.0 - vUV.y));
    radiusPx *= smoothstep(0.0, 0.05, edgeDist);
    radiusPx = min(radiusPx, 60.0);

    float shift = hash(vUV);

    for (int x = 0; x < SAMPLES_SQRT; x++) {
        for (int y = 0; y < SAMPLES_SQRT; y++) {
            
            vec2 samplePos = vec2(
                (float(x) / float(SAMPLES_SQRT - 1)) * 2.0 - 1.0,
                (float(y) / float(SAMPLES_SQRT - 1)) * 2.0 - 1.0
            );
            
            samplePos += (hash(samplePos + shift) - 0.5) * (2.0 / float(SAMPLES_SQRT));

            float apertureWeight = getApertureWeight(samplePos);

            if (apertureWeight < 0.1) continue; 

            vec2 offset = samplePos * radiusPx * texel;
            
            vec3 color = texture(u_HighlightTexture, vUV + offset).rgb;

            float w = apertureWeight;
            
            if(u_IsLutUsed) {
                float r = length(samplePos); 
                vec2 pointIntensity = vec2(clamp(r, 0.0, 1.0), u_FocusDepth);
                float lutWeight = 1.0; 

                if (u_Aperture <= 0.1) {
                    lutWeight = texture(u_LutTexture0, pointIntensity).r;
                }
                else if (u_Aperture <= 0.3) 
                    lutWeight = mix(texture(u_LutTexture0, pointIntensity).r, texture(u_LutTexture1, pointIntensity).r, (u_Aperture - 0.1) / (0.3 - 0.1)); 
                else if (u_Aperture <= 0.5) 
                    lutWeight = mix(texture(u_LutTexture1, pointIntensity).r, texture(u_LutTexture2, pointIntensity).r, (u_Aperture - 0.3) / (0.5 - 0.3)); 
                else if (u_Aperture <= 0.8) 
                    lutWeight = mix(texture(u_LutTexture2, pointIntensity).r, texture(u_LutTexture3, pointIntensity).r, (u_Aperture - 0.5) / (0.8 - 0.5));
                else if (u_Aperture <= 1.0) 
                    lutWeight = mix(texture(u_LutTexture3, pointIntensity).r, texture(u_LutTexture4, pointIntensity).r, (u_Aperture - 0.8) / (1.0 - 0.8));
                else if (u_Aperture <= 1.5) 
                    lutWeight = mix(texture(u_LutTexture4, pointIntensity).r, texture(u_LutTexture5, pointIntensity).r, (u_Aperture - 1.0) / (1.5 - 1.0)); 
                else if (u_Aperture <= 2.0) 
                    lutWeight = mix(texture(u_LutTexture5, pointIntensity).r, texture(u_LutTexture6, pointIntensity).r, (u_Aperture - 1.5) / (2.0 - 1.5));
                else if (u_Aperture <= 3.0) 
                    lutWeight = mix(texture(u_LutTexture6, pointIntensity).r, texture(u_LutTexture7, pointIntensity).r, (u_Aperture - 2.0) / (3.0 - 2.0));
                else lutWeight = texture(u_LutTexture7, pointIntensity).r;

                w *= lutWeight;
            }
            
            sum += color * color * w; 
            weightSum += w;
        }
    }

    if (weightSum < 0.0001) {
        FragColor = vec4(texture(u_HighlightTexture, vUV).rgb, 1.0);
        return;
    }

    vec3 result = sum / weightSum;
    result = sqrt(result);

    FragColor = vec4(result, 1.0);
}


