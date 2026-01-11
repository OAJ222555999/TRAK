#include <iostream>
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Renderer.h"
#include "loader/SceneLoader.h"




Renderer::Renderer()
{
    std::cout << "Renderer created\n";
}

Renderer::~Renderer()
{
    delete m_Camera;
    delete m_SceneShader;
    delete m_ScreenShader;
    delete m_HighlightShader;
    delete m_DOFShader;
    delete m_Scene3DShader;
    delete m_BokehShader;
    delete m_FinalShader;
    
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteVertexArrays(1, &m_CubeVAO);
    glDeleteBuffers(1, &m_CubeVBO);
    
    glDeleteFramebuffers(1, &m_FBO);
    glDeleteTextures(1, &m_ColorTexture);
    glDeleteTextures(1, &m_DepthTexture);
    
    glDeleteFramebuffers(1, &m_HighlightFBO);
    glDeleteTextures(1, &m_HighlightTexture);
    
    glDeleteFramebuffers(1, &m_DOFFBO);
    glDeleteTextures(1, &m_DOFTexture);
    
    glDeleteFramebuffers(1, &m_BokehFBO);
    glDeleteTextures(1, &m_BokehTexture);
    
    std::cout << "Renderer destroyed (cleanup done)" << std::endl;
}

void Renderer::init()
{
    m_Camera = new Camera(
    60.0f,           // FOV
    1280.0f / 720.0f,
    0.1f,
    100.0f
    );
    std::cout << "[CAMERA] created\n";

    float cubeVertices[] = {
    // front
    -1, -1,  1,   1, -1,  1,   1,  1,  1,
    -1, -1,  1,   1,  1,  1,  -1,  1,  1,

    // back
    -1, -1, -1,  -1,  1, -1,   1,  1, -1,
    -1, -1, -1,   1,  1, -1,   1, -1, -1,

    // left
    -1, -1, -1,  -1, -1,  1,  -1,  1,  1,
    -1, -1, -1,  -1,  1,  1,  -1,  1, -1,

    // right
     1, -1, -1,   1,  1,  1,   1, -1,  1,
     1, -1, -1,   1,  1, -1,   1,  1,  1,

    // top
    -1,  1, -1,  -1,  1,  1,   1,  1,  1,
    -1,  1, -1,   1,  1,  1,   1,  1, -1,

    // bottom
    -1, -1, -1,   1, -1,  1,  -1, -1,  1,
    -1, -1, -1,   1, -1, -1,   1, -1,  1
};
    glGenVertexArrays(1, &m_CubeVAO);
    glGenBuffers(1, &m_CubeVBO);

    glBindVertexArray(m_CubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_CubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);
    



    float quad[] = {
        // pos      // uv
        -1, -1,     0, 0,
         1, -1,     1, 0,
         1,  1,     1, 1,
        -1, -1,     0, 0,
         1,  1,     1, 1,
        -1,  1,     0, 1
    };

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);

    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                          4 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                          4 * sizeof(float),
                          (void*)(2 * sizeof(float)));

   m_SceneShader = new Shader(
    "shaders/fullscreen.vert",
    "shaders/fullscreen.frag"
    );

    m_ScreenShader = new Shader(
        "shaders/fullscreen.vert",
        "shaders/screen.frag"
    );

    m_HighlightShader = new Shader(
    "shaders/fullscreen.vert",
    "shaders/highlight.frag"
    );

    m_DOFShader = new Shader(
        "shaders/fullscreen.vert",
        "shaders/dof.frag"
    );

    m_Scene3DShader = new Shader(
    "shaders/scene3d.vert",
    "shaders/scene3d.frag"
    );

    m_BokehShader = new Shader(
    "shaders/fullscreen.vert",
    "shaders/bokeh.frag"
    );

    m_FinalShader = new Shader(
    "shaders/fullscreen.vert",
    "shaders/final.frag"
);


    m_Camera->setPosition(glm::vec3(0.0f, 0.0f, 8.0f));
    





    std::cout << "[INIT] SceneShader ID = "
          << m_SceneShader->getID() << std::endl;

    std::cout << "[INIT] ScreenShader ID = "
          << m_ScreenShader->getID() << std::endl;


    // --- Framebuffer ---
    glGenFramebuffers(1, &m_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

    // Color texture
    glGenTextures(1, &m_ColorTexture);
    glBindTexture(GL_TEXTURE_2D, m_ColorTexture);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA16F,
        1280,
        720,
        0,
        GL_RGBA,
        GL_FLOAT,
        nullptr
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D,
        m_ColorTexture,
        0
    );

    GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);


    // Depth texture
    glGenTextures(1, &m_DepthTexture);
    glBindTexture(GL_TEXTURE_2D, m_DepthTexture);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_DEPTH_COMPONENT24,
        1280,
        720,
        0,
        GL_DEPTH_COMPONENT,
        GL_FLOAT,
        nullptr
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT,
        GL_TEXTURE_2D,
        m_DepthTexture,
        0
    );

    // Check FBO
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "Framebuffer NOT complete!" << std::endl;
    }
    else
    {
        std::cout << "Framebuffer complete" << std::endl;
    }

    


    // Highlight framebuffer
    glGenFramebuffers(1, &m_HighlightFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_HighlightFBO);

    glGenTextures(1, &m_HighlightTexture);
    glBindTexture(GL_TEXTURE_2D, m_HighlightTexture);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA8,
        1280,
        720,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        nullptr
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D,
        m_HighlightTexture,
        0
    );

    GLenum hb[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, hb);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "Highlight FBO NOT complete!" << std::endl;
    }
    else
    {
        std::cout << "Highlight FBO complete" << std::endl;
    }

    glGenFramebuffers(1, &m_DOFFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_DOFFBO);

    glGenTextures(1, &m_DOFTexture);
    glBindTexture(GL_TEXTURE_2D, m_DOFTexture);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA8,
        1280,
        720,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        nullptr
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D,
        m_DOFTexture,
        0
    );

    GLenum db[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, db);

    // BOKEH FRAMEBUFFER
    glGenFramebuffers(1, &m_BokehFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_BokehFBO);

    glGenTextures(1, &m_BokehTexture);
    glBindTexture(GL_TEXTURE_2D, m_BokehTexture);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA16F,
        1280,
        720,
        0,
        GL_RGBA,
        GL_FLOAT,
        nullptr
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D,
        m_BokehTexture,
        0
);

    GLenum bokehBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, bokehBuffers);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "Bokeh FBO NOT complete!" << std::endl;
    }
    else
    {
        std::cout << "Bokeh FBO complete" << std::endl;
    }


        glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

        std::cout << "[RENDERER] Loading scene..." << std::flush;
        m_Scene = SceneLoader::loadGLTF(params.scenePath);
        std::cout << "[RENDERER] Loaded scene: " << params.scenePath
            << " (" << m_Scene.objects.size() << " objects)" << std::endl;



}



void Renderer::render()
{


// PASS 1: SCENA 3D → FBO
glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
glViewport(0, 0, 1280, 720);

glEnable(GL_DEPTH_TEST);
glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

m_Scene3DShader->bind();

GLuint shaderID = m_Scene3DShader->getID();

glUniform3f(
    glGetUniformLocation(shaderID, "u_LightDir"),
    0.4f, -0.6f, 1.0f
);

glUniform3f(
    glGetUniformLocation(shaderID, "u_LightColor"),
    0.9f, 0.95f, 1.0f
);

glUniform1f(
    glGetUniformLocation(shaderID, "u_LightStrength"),
    0.5f
);


for (const SceneObject& obj : m_Scene.objects)
{
    glm::mat4 model = glm::scale(obj.model, glm::vec3(0.01f));

    glm::mat4 mvp =
        m_Camera->getProjection() *
        m_Camera->getView() *
        model;

    // MVP
    glUniformMatrix4fv(
        glGetUniformLocation(m_Scene3DShader->getID(), "u_MVP"),
        1,
        GL_FALSE,
        &mvp[0][0]
    );

    //MODEL
    glUniformMatrix4fv(
        glGetUniformLocation(m_Scene3DShader->getID(), "u_Model"),
        1,
        GL_FALSE,
        &model[0][0]
    );

    glBindVertexArray(obj.vao);

    // ALBEDO
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, obj.albedoTexture);

    glUniform1i(
        glGetUniformLocation(m_Scene3DShader->getID(), "u_Albedo"),
        0
    );
    // NORMAL MAP
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, obj.normalTexture);
    glUniform1i(
        glGetUniformLocation(m_Scene3DShader->getID(), "u_NormalMap"),
        1
    );
    
  // EMISSIVE MAP
glActiveTexture(GL_TEXTURE2);
glBindTexture(
    GL_TEXTURE_2D,
    obj.emissiveTexture ? obj.emissiveTexture : 0
);
glUniform1i(
    glGetUniformLocation(m_Scene3DShader->getID(), "u_EmissiveMap"),
    2
);

// EMISSIVE COLOR
glUniform3fv(
    glGetUniformLocation(m_Scene3DShader->getID(), "u_EmissiveColor"),
    1,
    &obj.emissiveColor[0]
);

// EMISSIVE STRENGTH
glUniform1f(
    glGetUniformLocation(m_Scene3DShader->getID(), "u_EmissiveStrength"),
    obj.emissiveStrength
);




    glDrawElements(
        GL_TRIANGLES,
        obj.indexCount,
        obj.indexType,
        nullptr
    );
}

    // PASS 1.5: HIGHLIGHT

    glBindFramebuffer(GL_FRAMEBUFFER, m_HighlightFBO);
    glViewport(0, 0, 1280, 720);
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);

    m_HighlightShader->bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_ColorTexture);
    glUniform1i(
        glGetUniformLocation(m_HighlightShader->getID(), "u_SceneTexture"),
        0
    );

    glUniform1f(
        glGetUniformLocation(m_HighlightShader->getID(), "u_Threshold"),
        params.highlightThreshold
    );

    glBindVertexArray(m_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // PASS 1.75: BOKEH 

    glBindFramebuffer(GL_FRAMEBUFFER, m_BokehFBO);
    glViewport(0, 0, 1280, 720);
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);

    m_BokehShader->bind();

    // wejście = highlight
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_HighlightTexture);
    glUniform1i(
        glGetUniformLocation(m_BokehShader->getID(), "u_HighlightTexture"),
        0
    );

    glUniform1f(
        glGetUniformLocation(m_BokehShader->getID(), "u_Radius"),
        params.bokehRadius
    );

    glUniform2f(
        glGetUniformLocation(m_BokehShader->getID(), "u_Resolution"),
        1280.0f, 720.0f
    );

    glBindVertexArray(m_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);



    // PASS 2: DOF (BACKGROUND BLUR)

    glBindFramebuffer(GL_FRAMEBUFFER, m_DOFFBO);
    glViewport(0, 0, 1280, 720);
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);

    m_DOFShader->bind();

    // color
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_ColorTexture);
    glUniform1i(
        glGetUniformLocation(m_DOFShader->getID(), "u_ColorTexture"),
        0
    );

    // depth
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_DepthTexture);
    glUniform1i(
        glGetUniformLocation(m_DOFShader->getID(), "u_DepthTexture"),
        1
    );

    glUniform2f(
        glGetUniformLocation(m_DOFShader->getID(), "u_Resolution"),
        1280.0f, 720.0f
    );

    glUniform1f(
        glGetUniformLocation(m_DOFShader->getID(), "u_FocusDepth"),
        params.focusDepth
    );

    glUniform1f(
        glGetUniformLocation(m_DOFShader->getID(), "u_BlurStrength"),
        params.blurStrength
    );

    glBindVertexArray(m_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);


    // PASS FINAL: DOF + BOKEH
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, 1280, 720);
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);

    m_FinalShader->bind();

    // DOF (rozmyta scena)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_DOFTexture);
    glUniform1i(
        glGetUniformLocation(m_FinalShader->getID(), "u_DOFTexture"),
        0
    );

    // Bokeh (krążki światła)
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_BokehTexture);
    glUniform1i(
        glGetUniformLocation(m_FinalShader->getID(), "u_BokehTexture"),
        1
    );

    glBindVertexArray(m_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);


}



