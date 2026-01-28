#pragma once
#include <string>

struct BokehParams {
    std::string scenePath = "assets/Duck/Duck.gltf";
    
    int width = 1280;
    int height = 720;
    float modelScale = 0.01f;  // Skala modelu (--scale), 1.0 = oryginalny rozmiar
    
    float focusDepth = 0.925f;
    float blurStrength = 5.0f;
    float bokehRadius = 4.0f;
    float highlightThreshold = 0.3f;

    bool isLutUsed = true;
    
    // TODO readme.md #1,#2 - COC thin lens
    float focalLength = 0.05f;
    float aperture = 1.4f;
    float sensorWidth = 0.036f;
    
    // TODO readme.md #3 - blur vs bokeh threshold
    float cocThreshold = 4.0f;
    
    // TODO readme.md #5,#6 - visibility, depth test
    
    // TODO readme.md #10,#11 - cat-eye, chromatic
    float catEyeStrength = 0.0f;
    float chromaticStrength = 0.0f;


};
