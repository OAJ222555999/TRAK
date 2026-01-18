#include "Application.h"
#include "scene/Scene.h"
#include "loader/SceneLoader.h"
#include <iostream>
#include <string>
#include <cstring>

void printHelp() {
    std::cout << "Bokeh Renderer - Jeong 2022\n";
    std::cout << "Uzycie: BokehRenderer.exe [opcje]\n\n";
    std::cout << "Opcje:\n";
    std::cout << "  --scene <path>      Sciezka do pliku GLTF\n";
    std::cout << "  --focus <0.0-1.0>   Focus depth (default: 0.925)\n";
    std::cout << "  --aperture <f>      Aperture f-number (default: 1.4)\n";
    std::cout << "  --radius <px>       Bokeh radius (default: 4.0)\n";
    std::cout << "  --scale <f>         Model scale (default: 0.01, 1.0=oryginalny)\n";
    std::cout << "  --help              Wyswietl pomoc\n";
    std::cout << "  --lut   <0/1>       Use LUT (default: 1 - true)\n";
    std::cout << "\nDostepne sceny:\n";
    std::cout << "  assets/Duck/Duck.gltf           (mala, szybka)\n";
    std::cout << "  assets/cyborg/scene.gltf        (srednia)\n";
    std::cout << "  assets/fountain_base/scene.gltf (duza, wolna)\n";
    std::cout << "\nSterowanie w runtime:\n";
    std::cout << "  Strzalki gora/dol   Focus depth\n";
    std::cout << "  Strzalki lewo/prawo Bokeh radius\n";
    std::cout << "  Q/E                 Aperture\n";
    std::cout << "  WASD                Kamera\n";
}

int main(int argc, char* argv[])
{
    std::string scenePath = "assets/la_night_city/scene.gltf";
    //std::string scenePath = "assets/Duck/Duck.gltf";
    //std::string scenePath = "assets/PunctualLamp/LightsPunctualLamp.gltf";
    //std::string scenePath = "assets/fountain_base/scene.gltf";
    float focusDepth = 0.925f;
    float aperture = 1.4f;
    float bokehRadius = 4.0f;
    float modelScale = 0.01f;
    bool isLutUsed = 1;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--help") == 0) {
            printHelp();
            return 0;
        }
        if (strcmp(argv[i], "--scene") == 0 && i + 1 < argc) {
            scenePath = argv[++i];
        }
        if (strcmp(argv[i], "--focus") == 0 && i + 1 < argc) {
            focusDepth = std::stof(argv[++i]);
        }
        if (strcmp(argv[i], "--aperture") == 0 && i + 1 < argc) {
            aperture = std::stof(argv[++i]);
        }
        if (strcmp(argv[i], "--radius") == 0 && i + 1 < argc) {
            bokehRadius = std::stof(argv[++i]);
        }
        if (strcmp(argv[i], "--scale") == 0 && i + 1 < argc) {
            modelScale = std::stof(argv[++i]);
        }
        if (strcmp(argv[i], "--lut") == 0 && i + 1 < argc) {
            isLutUsed = std::stof(argv[++i]);
        }
    }

    std::cout << "[CLI] Scene: " << scenePath << "\n";
    std::cout << "[CLI] Focus: " << focusDepth << " Aperture: f/" << aperture << " Radius: " << bokehRadius << " Scale: " << modelScale << " LUT: " << isLutUsed << "\n";

    Application app;
    app.getRenderer().params.focusDepth = focusDepth;
    app.getRenderer().params.aperture = aperture;
    app.getRenderer().params.bokehRadius = bokehRadius;
    app.getRenderer().params.modelScale = modelScale;
    app.getRenderer().params.scenePath = scenePath;
    app.getRenderer().params.isLutUsed = isLutUsed;
    
    app.run();

    return 0;
}

