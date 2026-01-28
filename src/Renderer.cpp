#include <iostream>
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Renderer.h"
#include "loader/SceneLoader.h"

#include <stb_image.h>
#include <filesystem>

namespace fs = std::filesystem;





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

	glDeleteTextures(1, &m_ApertureTexture);

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

	if (params.isLutUsed)
	{
		// LUT 
		std::cout << "[INIT] Look-up table initialization..." << std::endl;
		unsigned int lutLoadedTexturesNumber = 0;
		for (short i = 0; i < lutTexturesNumber; i++)
		{
			int width, height, channels;
			unsigned char* lutTextureData = stbi_load(lutTextureFiles[i].c_str(), &width, &height, &channels, 4);

			if (lutTextureData)
			{
				glGenTextures(1, &Renderer::lookUpTable[i]);
				glBindTexture(GL_TEXTURE_2D, lookUpTable[i]);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, lutTextureData);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				std::cout << "\t[SUCCESS] Loaded LUT texture: width=" << width << ", height=" << height << ", file=" << lutTextureFiles[i] << std::endl;
				lutLoadedTexturesNumber++;
				stbi_image_free(lutTextureData);
			}
			else
			{
				std::cerr << "\t[FAILED] Cannot load LUT texture, file=" << lutTextureFiles[i] << std::endl;
			}
		}
		std::cout << "\n\tThe number of LUT texture loaded: " << lutLoadedTexturesNumber << "/" << lutTexturesNumber << std::endl;

		if (lutLoadedTexturesNumber != lutTexturesNumber)
		{
			params.isLutUsed = false;
			std::cerr << "\t[FAILED] Some LUT textures are missing... Look-up table will not be used" << std::endl;
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

	std::cout << "[RENDERER] Loading scene..." << std::flush;
	m_Scene = SceneLoader::loadGLTF(params.scenePath);
	std::cout << "[RENDERER] Loaded scene: " << params.scenePath
		<< " (" << m_Scene.objects.size() << " objects)" << std::endl;


	// APERTURE
	glGenTextures(1, &m_ApertureTexture);
	glBindTexture(GL_TEXTURE_2D, m_ApertureTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	m_ApertureFiles.clear();
	std::string aperturePath = "assets/Aperatures";
	if (fs::exists(aperturePath) && fs::is_directory(aperturePath)) {
		for (const auto& entry : fs::directory_iterator(aperturePath)) {
			auto ext = entry.path().extension();
			if (ext == ".png" || ext == ".jpg" || ext == ".jpeg") {
				m_ApertureFiles.push_back(entry.path().string());
			}
		}
	}

	if (!m_ApertureFiles.empty()) {
		m_CurrentApertureIndex = 0;
		loadCurrentAperture();
	}
	else {
		std::cerr << "[ERROR] No aperture textures found in " << aperturePath << std::endl;
		unsigned char white[] = { 255, 255, 255, 255 };
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
	}
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
		glm::mat4 model =
    		glm::scale(glm::mat4(1.0f), glm::vec3(m_Scene.globalScale)) *
    		obj.model;



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

	// Depth texture dla COC
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_DepthTexture);
	glUniform1i(glGetUniformLocation(m_HighlightShader->getID(), "u_DepthTexture"), 1);

	// #3 - parametry COC dla blur vs bokeh (Sec.4.1 Jeong 2022)
	glUniform1f(glGetUniformLocation(m_HighlightShader->getID(), "u_CocThreshold"), params.cocThreshold);
	glUniform1f(glGetUniformLocation(m_HighlightShader->getID(), "u_FocalLength"), params.focalLength);
	glUniform1f(glGetUniformLocation(m_HighlightShader->getID(), "u_Aperture"), params.aperture);
	glUniform1f(glGetUniformLocation(m_HighlightShader->getID(), "u_SensorWidth"), params.sensorWidth);
	glUniform1f(glGetUniformLocation(m_HighlightShader->getID(), "u_FocusDepth"), params.focusDepth);
	glUniform1f(glGetUniformLocation(m_HighlightShader->getID(), "u_NearPlane"), 0.1f);
	glUniform1f(glGetUniformLocation(m_HighlightShader->getID(), "u_FarPlane"), 100.0f);
	glUniform2f(glGetUniformLocation(m_HighlightShader->getID(), "u_Resolution"), 1280.0f, 720.0f);

	glBindVertexArray(m_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// PASS 1.75: BOKEH 

	glBindFramebuffer(GL_FRAMEBUFFER, m_BokehFBO);
	glViewport(0, 0, 1280, 720);
	glDisable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT);

	m_BokehShader->bind();
	GLuint bokehShaderID = m_BokehShader->getID();

	glUniform1f(glGetUniformLocation(bokehShaderID, "u_IsLutUsed"), params.isLutUsed);

	// LUT
	if (params.isLutUsed)
	{
		;
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, lookUpTable[0]);
		glUniform1i(glGetUniformLocation(bokehShaderID, "u_LutTexture0"), 2);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, lookUpTable[1]);
		glUniform1i(glGetUniformLocation(bokehShaderID, "u_LutTexture1"), 3);

		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, lookUpTable[2]);
		glUniform1i(glGetUniformLocation(bokehShaderID, "u_LutTexture2"), 4);

		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, lookUpTable[3]);
		glUniform1i(glGetUniformLocation(bokehShaderID, "u_LutTexture3"), 5);

		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, lookUpTable[4]);
		glUniform1i(glGetUniformLocation(bokehShaderID, "u_LutTexture4"), 6);

		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_2D, lookUpTable[5]);
		glUniform1i(glGetUniformLocation(bokehShaderID, "u_LutTexture5"), 7);

		glActiveTexture(GL_TEXTURE8);
		glBindTexture(GL_TEXTURE_2D, lookUpTable[6]);
		glUniform1i(glGetUniformLocation(bokehShaderID, "u_LutTexture6"), 8);

		glActiveTexture(GL_TEXTURE9);
		glBindTexture(GL_TEXTURE_2D, lookUpTable[7]);
		glUniform1i(glGetUniformLocation(bokehShaderID, "u_LutTexture7"), 9);

		glUniform1f(glGetUniformLocation(bokehShaderID, "u_Aperture"), params.aperture);

		glUniform1f(glGetUniformLocation(bokehShaderID, "u_FocusDepth"), params.focusDepth);
	}

	// Highlight
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_HighlightTexture);
	glUniform1i(glGetUniformLocation(bokehShaderID, "u_HighlightTexture"), 0);

	// Aparature
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_ApertureTexture);
	glUniform1i(glGetUniformLocation(bokehShaderID, "uApertureTex"), 1);

	glUniform1f(glGetUniformLocation(bokehShaderID, "u_Radius"), params.bokehRadius);
	glUniform2f(glGetUniformLocation(bokehShaderID, "u_Resolution"), 1280.0f, 720.0f);
	glUniform1f(glGetUniformLocation(bokehShaderID, "u_Aperture"), params.aperture);
	glUniform1f(glGetUniformLocation(bokehShaderID, "u_FocusDepth"), params.focusDepth);
	glUniform1i(glGetUniformLocation(bokehShaderID, "u_IsLutUsed"), params.isLutUsed ? 1 : 0);

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

	// #1,#2 - parametry thin lens dla COC (Sec.5.1 Jeong 2022)
	glUniform1f(glGetUniformLocation(m_DOFShader->getID(), "u_FocalLength"), params.focalLength);
	glUniform1f(glGetUniformLocation(m_DOFShader->getID(), "u_Aperture"), params.aperture);
	glUniform1f(glGetUniformLocation(m_DOFShader->getID(), "u_SensorWidth"), params.sensorWidth);
	glUniform1f(glGetUniformLocation(m_DOFShader->getID(), "u_NearPlane"), 0.1f);
	glUniform1f(glGetUniformLocation(m_DOFShader->getID(), "u_FarPlane"), 100.0f);


	glBindVertexArray(m_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);


	// PASS FINAL / DEBUG
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, 1280, 720);
	glDisable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindVertexArray(m_VAO);


	switch (m_DebugView)
	{
	case DebugView::SceneColor:
		m_ScreenShader->bind();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_ColorTexture);
		glUniform1i(
			glGetUniformLocation(m_ScreenShader->getID(), "u_ScreenTexture"),
			0
		);
		break;


	case DebugView::Highlight:
		m_ScreenShader->bind();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_HighlightTexture);
		glUniform1i(
			glGetUniformLocation(m_ScreenShader->getID(), "u_ScreenTexture"),
			0
		);
		break;

	case DebugView::BokehOnly:
		m_ScreenShader->bind();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_BokehTexture);
		glUniform1i(
			glGetUniformLocation(m_ScreenShader->getID(), "u_ScreenTexture"),
			0
		);
		break;

	case DebugView::DOFOnly:
		m_ScreenShader->bind();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_DOFTexture);
		glUniform1i(
			glGetUniformLocation(m_ScreenShader->getID(), "u_ScreenTexture"),
			0
		);
		break;

	case DebugView::Final:
	default:
		m_FinalShader->bind();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_DOFTexture);
		glUniform1i(
			glGetUniformLocation(m_FinalShader->getID(), "u_DOFTexture"),
			0
		);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_BokehTexture);
		glUniform1i(
			glGetUniformLocation(m_FinalShader->getID(), "u_BokehTexture"),
			1
		);
		break;
	}

	glDrawArrays(GL_TRIANGLES, 0, 6);


}

void Renderer::loadCurrentAperture() {
	if (m_ApertureFiles.empty()) return;

	int width, height, nrChannels;
	std::string filePath = m_ApertureFiles[m_CurrentApertureIndex];

	stbi_set_flip_vertically_on_load(false);
	unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 4);

	if (data) {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_ApertureTexture);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		glGenerateMipmap(GL_TEXTURE_2D);
		stbi_image_free(data);
	}
}

void Renderer::changeAperture(int delta) {
	if (m_ApertureFiles.empty()) return;

	m_CurrentApertureIndex += delta;

	if (m_CurrentApertureIndex >= (int)m_ApertureFiles.size()) m_CurrentApertureIndex = 0;
	if (m_CurrentApertureIndex < 0) m_CurrentApertureIndex = (int)m_ApertureFiles.size() - 1;

	loadCurrentAperture();
}


