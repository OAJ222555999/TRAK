#pragma once
#include <vector>
#include <string>
#include "Shader.h"
#include "core/Camera.h"
#include "scene/Scene.h"
#include "BokehParams.h"

class Renderer
{
public:
    Renderer();
    ~Renderer();

    void init();
    void render();

    unsigned int m_VAO = 0;
    unsigned int m_VBO = 0;

    Camera* getCamera() { return m_Camera; }
    BokehParams params;
    Shader* m_BokehShader = nullptr;

    unsigned int m_CubeVAO = 0;
    unsigned int m_CubeVBO = 0;
    Shader* m_Scene3DShader = nullptr;
    Shader* m_FinalShader = nullptr;
    unsigned int m_BokehFBO = 0;
    unsigned int m_BokehTexture = 0;
        
    Shader* m_SceneShader = nullptr;
    Shader* m_ScreenShader = nullptr;

    unsigned int m_FBO = 0;
    unsigned int m_ColorTexture = 0;
    unsigned int m_DepthTexture = 0;

    unsigned int m_HighlightFBO = 0;
    unsigned int m_HighlightTexture = 0;
    Shader* m_HighlightShader = nullptr;

    unsigned int m_DOFFBO = 0;
    unsigned int m_DOFTexture = 0;
    Shader* m_DOFShader = nullptr;
    GLuint m_ApertureTexture = 0;
    void changeAperture(int delta);

    GLuint m_SceneFBO = 0;
    GLuint m_SceneColor = 0;
    GLuint m_SceneDepth = 0;

private:
    Camera* m_Camera = nullptr;
    Scene m_Scene;

    static constexpr unsigned int lutTexturesNumber = 8; 
    std::string lutTextureFiles[lutTexturesNumber] = { 
                        "assets/lut_textures/1.png", 
                        "assets/lut_textures/3.png",
                        "assets/lut_textures/5.png",
                        "assets/lut_textures/8.png",
                        "assets/lut_textures/10.png",
                        "assets/lut_textures/15.png",
                        "assets/lut_textures/20.png",
                        "assets/lut_textures/30.png"
                    };

    GLuint lookUpTable[lutTexturesNumber];

    std::vector<std::string> m_ApertureFiles;
    int m_CurrentApertureIndex = 0;
    void loadCurrentAperture();
};
