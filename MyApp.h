#pragma once

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

// GLEW
#include <GL/glew.h>

// SDL
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

// Utils
#include "Camera.h"
#include "CameraManipulator.h"
#include "GLUtils.hpp"

struct SUpdateInfo
{
	float ElapsedTimeInSec = 0.0f; // Program indulása óta eltelt idő
	float DeltaTimeInSec = 0.0f;   // Előző Update óta eltelt idő
};

struct Ray
{
	glm::vec3 origin;
	glm::vec3 direction;
};

struct Intersection
{
	glm::vec2 uv;
	float t;
};

struct Card
{
	int face;
	glm::vec3 pos;

	Card(int face, glm::vec3 pos)
	{
		this->face = face;
		this->pos = pos;
	}
};

/*struct Chip
{
	glm::vec3 color;
	glm::vec3 pos;

	Chip(glm::vec3 color, glm::vec3 pos)
	{
		this->color = color;
		this->pos = pos;
	}
};*/

class CMyApp
{
public:
	CMyApp();
	~CMyApp();

	bool Init();
	void Clean();

	void Update(const SUpdateInfo&);
	void Render();
	void RenderGUI();

	void RenderTable();
	void RenderCards();
	void RenderHand();
	void RenderChips();

	void KeyboardDown(const SDL_KeyboardEvent&);
	void KeyboardUp(const SDL_KeyboardEvent&);
	void MouseMove(const SDL_MouseMotionEvent&);
	void MouseDown(const SDL_MouseButtonEvent&);
	void MouseUp(const SDL_MouseButtonEvent&);
	void MouseWheel(const SDL_MouseWheelEvent&);
	void Resize(int, int);

	void OtherEvent(const SDL_Event&);

protected:
	void SetupDebugCallback();

	// Adat változók

	float m_ElapsedTimeInSec = 0.0f;

	std::vector<Card> cards = {};

	float handAngle = 0;

	int N = 10;
	int K = 5;

	bool animate = true;
	float lastStop = 0.0;

	glm::vec3 c1 = glm::vec3(1.0, 1.0, 1.0);
	glm::vec3 c2 = glm::vec3(1.0, 1.0, 1.0);

	ImageRGBA cardImage;
	ImageRGBA modifiedImage;

	int pick = 0;

	// Picking

	glm::ivec2 m_PickedPixel = glm::ivec2( 0, 0 );
	bool m_IsPicking = false;
	bool m_IsCtrlDown = false;

	glm::uvec2 m_windowSize = glm::uvec2(0, 0);

	Ray CalculatePixelRay(glm::vec2 pickerPos) const;


	// Kamera
	Camera m_camera;
	CameraManipulator m_cameraManipulator;

	//
	// OpenGL-es dolgok
	//

	// shaderekhez szükséges változók
	GLuint m_programID = 0; // shaderek programja

	bool lightSwitch = true;
	int state = 0;

	// Shaderek inicializálása, és törlése
	void InitShaders();
	void CleanShaders() const;

	// Geometriával kapcsolatos változók

	void SetCommonUniforms();
	void DrawObject(OGLObject& obj, const glm::mat4& world);

	OGLObject m_quadGPU = {};
	OGLObject m_handGPU = {};
	OGLObject m_chipGPU = {};

	// Geometria inicializálása, és törlése
	void InitGeometry();
	void CleanGeometry();

	// Textúrázás, és változói
	GLuint m_SamplerID = 0;

	GLuint m_cardTextureID = 0;
	GLuint m_tableTextureID = 0;
	GLuint m_handTextureID = 0;
	GLuint m_chipTextureID = 0;

	void InitTextures();
	void CleanTextures() const;
};
