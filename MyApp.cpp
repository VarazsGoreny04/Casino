#include "MyApp.h"
#include "ObjParser.h"
#include "SDL_GLDebugMessageCallback.h"

#include <imgui.h>

CMyApp::CMyApp()
{
}

CMyApp::~CMyApp()
{
}

void CMyApp::SetupDebugCallback()
{
	// engedélyezzük és állítsuk be a debug callback függvényt ha debug context-ben vagyunk 
	GLint context_flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &context_flags);
	if (context_flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
		glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, GL_DONT_CARE, 0, nullptr, GL_FALSE);
		glDebugMessageCallback(SDL_GLDebugMessageCallback, nullptr);
	}
}

void CMyApp::InitShaders()
{
	m_programID = glCreateProgram();
	AttachShader(m_programID, GL_VERTEX_SHADER, "Shaders/Vert_PosNormTex.vert");
	AttachShader(m_programID, GL_FRAGMENT_SHADER, "Shaders/Frag_ZH.frag");
	LinkProgram(m_programID);
}

void CMyApp::CleanShaders() const
{
	glDeleteProgram(m_programID);
}

static MeshObject<Vertex> createQuad()
{
	MeshObject<Vertex> mesh;

	mesh.vertexArray = {
		{{-0.5f, 0.0f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
		{{ 0.5f, 0.0f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
		{{ 0.5f, 0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
		{{-0.5f, 0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
		{{-0.5f, 0.0f,  0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
		{{ 0.5f, 0.0f,  0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
		{{ 0.5f, 0.0f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
		{{-0.5f, 0.0f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
	};

	mesh.indexArray = { 0, 1, 2, 2, 3, 0, 4, 6, 5, 6, 4, 7 };

	return mesh;
}

void CMyApp::InitGeometry()
{
	const std::initializer_list<VertexAttributeDescriptor> vertexAttribList =
	{
		{0, offsetof(Vertex, position), 3, GL_FLOAT},
		{1, offsetof(Vertex, normal), 3, GL_FLOAT},
		{2, offsetof(Vertex, texcoord), 2, GL_FLOAT},
	};

	m_quadGPU = CreateGLObjectFromMesh(createQuad(), vertexAttribList);

	m_handGPU = CreateGLObjectFromMesh(ObjParser::parse("Assets/hand.obj"), vertexAttribList);
	m_chipGPU = CreateGLObjectFromMesh(ObjParser::parse("Assets/pokerchip.obj"), vertexAttribList);
}

void CMyApp::CleanGeometry()
{
	CleanOGLObject(m_quadGPU);
	CleanOGLObject(m_handGPU);
	CleanOGLObject(m_chipGPU);
}

void CMyApp::InitTextures()
{
	glCreateSamplers(1, &m_SamplerID);
	glSamplerParameteri(m_SamplerID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(m_SamplerID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(m_SamplerID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glSamplerParameteri(m_SamplerID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	cardImage = ImageFromFile("Assets/cards.png");
	glCreateTextures(GL_TEXTURE_2D, 1, &m_cardTextureID);
	glTextureStorage2D(m_cardTextureID, NumberOfMIPLevels(cardImage), GL_RGBA8, cardImage.width, cardImage.height);
	glTextureSubImage2D(m_cardTextureID, 0, 0, 0, cardImage.width, cardImage.height, GL_RGBA, GL_UNSIGNED_BYTE, cardImage.data());
	glGenerateTextureMipmap(m_cardTextureID);

	ImageRGBA image = ImageFromFile("Assets/green_fabric.jpg");
	glCreateTextures(GL_TEXTURE_2D, 1, &m_tableTextureID);
	glTextureStorage2D(m_tableTextureID, NumberOfMIPLevels(image), GL_RGBA8, image.width, image.height);
	glTextureSubImage2D(m_tableTextureID, 0, 0, 0, image.width, image.height, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
	glGenerateTextureMipmap(m_tableTextureID);

	image = ImageFromFile("Assets/hand.png");
	glCreateTextures(GL_TEXTURE_2D, 1, &m_handTextureID);
	glTextureStorage2D(m_handTextureID, NumberOfMIPLevels(image), GL_RGBA8, image.width, image.height);
	glTextureSubImage2D(m_handTextureID, 0, 0, 0, image.width, image.height, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
	glGenerateTextureMipmap(m_handTextureID);

	image = ImageFromFile("Assets/pokerchip.jpg");
	glCreateTextures(GL_TEXTURE_2D, 1, &m_chipTextureID);
	glTextureStorage2D(m_chipTextureID, NumberOfMIPLevels(image), GL_RGBA8, image.width, image.height);
	glTextureSubImage2D(m_chipTextureID, 0, 0, 0, image.width, image.height, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
	glGenerateTextureMipmap(m_chipTextureID);
}

void CMyApp::CleanTextures() const
{
	glDeleteSamplers(1, &m_SamplerID);
	glDeleteTextures(1, &m_cardTextureID);
	glDeleteTextures(1, &m_tableTextureID);
	glDeleteTextures(1, &m_handTextureID);
	glDeleteTextures(1, &m_chipTextureID);
}

bool CMyApp::Init()
{
	SetupDebugCallback();

	// törlési szín legyen kékes
	glClearColor(0.125f, 0.25f, 0.5f, 1.0f);

	for (int i = 0; i < 5; ++i)
		cards.push_back(Card(i, glm::vec3(static_cast<float>(-4 + (i * 2)), 0.0, 0.0)));

	InitShaders();
	InitGeometry();
	InitTextures();

	//
	// egyéb inicializálás
	//

	glEnable(GL_CULL_FACE); // kapcsoljuk be a hátrafelé néző lapok eldobását
	glCullFace(GL_BACK);    // GL_BACK: a kamerától "elfelé" néző lapok, GL_FRONT: a kamera felé néző lapok

	glEnable(GL_DEPTH_TEST); // mélységi teszt bekapcsolása (takarás)

	// kamera
	m_camera.SetView(
		glm::vec3(0.0, 30.0, 25.0),  // honnan nézzük a színteret	   - eye
		glm::vec3(0.0, 0.0, 0.0),  // a színtér melyik pontját nézzük - at
		glm::vec3(0.0, 1.0, 0.0)); // felfelé mutató irány a világban - up

	m_cameraManipulator.SetCamera(&m_camera);

	return true;
}

void CMyApp::Clean()
{
	CleanShaders();
	CleanGeometry();
	CleanTextures();
}

static bool HitPlane(const Ray& ray, const glm::vec3& planeQ, const glm::vec3& planeI, const glm::vec3& planeJ, Intersection& result)
{
	// sík parametrikus egyenlete: palneQ + u * planeI + v * planeJ
	glm::mat3 A(-ray.direction, planeI, planeJ);
	glm::vec3 B = ray.origin - planeQ;

	if (fabsf(glm::determinant(A)) < 1e-6) return false;
	glm::vec3 X = glm::inverse(A) * B;

	if (X.x < 0.0) {
		return false;
	}
	result.t = X.x;
	result.uv.x = X.y;
	result.uv.y = X.z;

	return true;
}


static bool HitSphere(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const glm::vec3& sphereCenter, float sphereRadius, float& t)
{
	glm::vec3 p_m_c = rayOrigin - sphereCenter;
	float a = glm::dot(rayDir, rayDir);
	float b = 2.0f * glm::dot(rayDir, p_m_c);
	float c = glm::dot(p_m_c, p_m_c) - sphereRadius * sphereRadius;

	float discriminant = b * b - 4.0f * a * c;

	if (discriminant < 0.0f)
	{
		return false;
	}

	float sqrtDiscriminant = sqrtf(discriminant);

	// Mivel 2*a, es sqrt(D) mindig pozitívak, ezért tudjuk, hogy t0 < t1
	float t0 = (-b - sqrtDiscriminant) / (2.0f * a);
	float t1 = (-b + sqrtDiscriminant) / (2.0f * a);

	if (t1 < 0.0f) // mivel t0 < t1, ha t1 negatív, akkor t0 is az
	{
		return false;
	}

	if (t0 < 0.0f)
	{
		t = t1;
	}
	else
	{
		t = t0;
	}

	return true;
}

Ray CMyApp::CalculatePixelRay(glm::vec2 pixel) const
{
	// NDC koordináták kiszámítása
	glm::vec3 pickedNDC = glm::vec3(
		2.0f * (pixel.x + 0.5f) / m_windowSize.x - 1.0f,
		1.0f - 2.0f * (pixel.y + 0.5f) / m_windowSize.y, 0.0f);

	// A világ koordináták kiszámítása az inverz ViewProj mátrix segítségével
	glm::vec4 pickedWorld = glm::inverse(m_camera.GetViewProj()) * glm::vec4(pickedNDC, 1.0f);
	pickedWorld /= pickedWorld.w; // homogén osztás
	Ray ray = Ray();

	// Raycasting kezdőpontja a kamera pozíciója
	ray.origin = m_camera.GetEye();
	// Iránya a kamera pozíciójából a kattintott pont világ koordinátái felé
	// FIGYELEM: NEM egység hosszúságú vektor!
	ray.direction = glm::vec3(pickedWorld) - ray.origin;
	return ray;
}

void CMyApp::Update(const SUpdateInfo& updateInfo)
{
	m_ElapsedTimeInSec = updateInfo.ElapsedTimeInSec;

	if (m_IsPicking) {
		// a felhasználó Ctrl + kattintott, itt kezeljük le
		// sugár indítása a kattintott pixelen át
		Ray ray = CalculatePixelRay(glm::vec2(m_PickedPixel.x, m_PickedPixel.y));
		Intersection intersect;
		
		if (HitPlane(ray, glm::vec3(0.0, -0.1, 0.0), glm::vec3(30.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 21.0), intersect) && 
			(std::abs(intersect.uv.x) <= 0.5 && std::abs(intersect.uv.y) <= 0.5))
		{
			cards.push_back(Card(pick, glm::vec3(std::round((30.0 * intersect.uv.x) / 2.0) * 2, 0.0, std::round((21.0 * intersect.uv.y) / 3.0) * 3)));
		}
		
        m_IsPicking = false;
	}

	m_cameraManipulator.Update(updateInfo.DeltaTimeInSec);
}

void CMyApp::SetCommonUniforms()
{
	// - Uniform paraméterek

	// view és projekciós mátrix
	glProgramUniformMatrix4fv(m_programID, ul(m_programID, "viewProj"), 1, GL_FALSE, glm::value_ptr(m_camera.GetViewProj()));
	// - Fényforrások beállítása
	glProgramUniform3fv(m_programID, ul(m_programID, "cameraPosition"), 1, glm::value_ptr(m_camera.GetEye()));
	glProgramUniform1f(m_programID, ul(m_programID, "t"), (animate ? m_ElapsedTimeInSec - lastStop : lastStop) * glm::half_pi<float>());
	glProgramUniform1f(m_programID, ul(m_programID, "lightSwitch"), lightSwitch);

	glProgramUniform1i(m_programID, ul(m_programID, "state"), state);

	/*glProgramUniform4fv(m_programID, ul(m_programID, "lightPos"), 1, glm::value_ptr(m_lightPos));
	
	glProgramUniform3fv(m_programID, ul(m_programID, "La"), 1, glm::value_ptr(m_La));
	glProgramUniform3fv(m_programID, ul(m_programID, "Ld"), 1, glm::value_ptr(m_Ld));
	glProgramUniform3fv(m_programID, ul(m_programID, "Ls"), 1, glm::value_ptr(m_Ls));
	
	glProgramUniform1f(m_programID, ul(m_programID, "lightConstantAttenuation"), m_lightConstantAttenuation);
	glProgramUniform1f(m_programID, ul(m_programID, "lightLinearAttenuation"), m_lightLinearAttenuation);
	glProgramUniform1f(m_programID, ul(m_programID, "lightQuadraticAttenuation"), m_lightQuadraticAttenuation);*/
}

void CMyApp::DrawObject(OGLObject& obj, const glm::mat4& world) {
	glUniformMatrix4fv(ul("world"), 1, GL_FALSE, glm::value_ptr(world));
	glUniformMatrix4fv(ul("worldIT"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(world))));
	glBindVertexArray(obj.vaoID);
	glDrawElements(GL_TRIANGLES, obj.count, GL_UNSIGNED_INT, nullptr);
}

void CMyApp::RenderTable()
{
	glUniform1i(ul("state"), 0);
	glUniform1i(ul("lightSwitch"), false);

	glBindTextureUnit(0, m_tableTextureID);

	DrawObject(m_quadGPU, glm::translate(glm::vec3(0.0, -0.1, 0.0)) * glm::scale(glm::vec3(30.0, 0.0, 21.0)));
	
	glUniform1i(ul("lightSwitch"), true);
}

void CMyApp::RenderCards()
{
	glDeleteTextures(1, &m_cardTextureID);

	modifiedImage = cardImage;

	float c1F, c2F;
	ImageRGBA::TexelRGBA texel;

	for (unsigned int y = 0; y < modifiedImage.height; ++y)
	{
		c1F = static_cast<float>(y) / modifiedImage.height;
		c2F = 1 - c1F;

		unsigned int x = 0;
		for (; x < modifiedImage.width / 5; ++x)
		{
			modifiedImage.SetTexel(x, y, glm::u8vec4((c1.x * c1F + c2.x * c2F) * 255.f, (c1.y * c1F + c2.y * c2F) * 255.f, (c1.z * c1F + c2.z * c2F) * 255.f, 0.f));
		}

		for (; x < 2 * modifiedImage.width / 5; ++x)
		{
			texel = modifiedImage.GetTexel(x, y);

			if (texel.x + texel.y + texel.z > 760)
				modifiedImage.SetTexel(x, y, glm::u8vec4(c2.x * 255.f, c2.y * 255.f, c2.z * 255.f, 0.0));
		}

		for (; x < 3 * modifiedImage.width / 5; ++x)
		{
			texel = modifiedImage.GetTexel(x, y);

			if (texel.x + texel.y + texel.z > 760)
				modifiedImage.SetTexel(x, y, glm::u8vec4(c1.x * 255.f, c1.y * 255.f, c1.z * 255.f, 0.0));
		}

		for (; x < 4 * modifiedImage.width / 5; ++x)
		{
			texel = modifiedImage.GetTexel(x, y);

			if (texel.x + texel.y + texel.z > 760)
				modifiedImage.SetTexel(x, y, glm::u8vec4(c2.x * 255.f, c2.y * 255.f, c2.z * 255.f, 0.0));
		}

		for (; x < 5 * modifiedImage.width / 5; ++x)
		{
			texel = modifiedImage.GetTexel(x, y);

			if (texel.x + texel.y + texel.z > 760)
				modifiedImage.SetTexel(x, y, glm::u8vec4(c1.x * 255.f, c1.y * 255.f, c1.z * 255.f, 0.0));
		}
	}

	glCreateTextures(GL_TEXTURE_2D, 1, &m_cardTextureID);
	glTextureStorage2D(m_cardTextureID, NumberOfMIPLevels(modifiedImage), GL_RGBA8, modifiedImage.width, modifiedImage.height);
	glTextureSubImage2D(m_cardTextureID, 0, 0, 0, modifiedImage.width, modifiedImage.height, GL_RGBA, GL_UNSIGNED_BYTE, modifiedImage.data());
	glGenerateTextureMipmap(m_cardTextureID);

	glBindTextureUnit(0, m_cardTextureID);
	
	for (int i = 0; i < cards.size(); ++i)
	{
		glUniform1i(ul("card"), cards[i].face);
		DrawObject(m_quadGPU, glm::translate(cards[i].pos) * glm::scale(glm::vec3(1.6, 1.0, 2.48)));
	}

	glUniform1i(ul("card"), -1);
}

void CMyApp::RenderHand()
{
	glBindTextureUnit(0, m_handTextureID);
	
	DrawObject(m_handGPU, glm::translate(glm::vec3(-3.0, 2.0, 12.0)) * glm::rotate(handAngle, glm::vec3(0.0, 1.0, 0.0)) * glm::scale(glm::vec3(3.0)));
}

void CMyApp::RenderChips()
{
	glBindTextureUnit(0, m_chipTextureID);
	
	for (int i = 0; i < N; ++i)
	{
		glUniform1i(ul("state"), i % 6);

		DrawObject(m_chipGPU, glm::translate(glm::vec3(-10.0, static_cast<float>(i) * 0.15, 5.0)));
	}
	
	float alpha = 2.f / static_cast<float>(K) * glm::pi<float>();

	for (int i = 0; i < K; ++i)
	{
		glUniform1i(ul("state"), i % 6);

		DrawObject(m_chipGPU,
			glm::translate(glm::vec3(-10.0, 1.0, 5.0)) *

			glm::rotate(m_ElapsedTimeInSec + (alpha * static_cast<float>(i)), glm::vec3(0.0, 1.0, 0.0)) *
			glm::translate(glm::vec3(4.0, 0.0, 0.0)) *

			glm::rotate(glm::half_pi<float>(), glm::vec3(0.0, 0.0, 1.0)) *
			glm::rotate(m_ElapsedTimeInSec * 4.f, glm::vec3(0.0, 1.0, 0.0))
		);
	}

	glUniform1i(ul("state"), 0);
}

void CMyApp::Render()
{
	// töröljük a frampuffert (GL_COLOR_BUFFER_BIT)...
	// ... és a mélységi Z puffert (GL_DEPTH_BUFFER_BIT)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	SetCommonUniforms();

	glUseProgram(m_programID);

	glUniform1i(ul("texImage"), 0);
	//glUniform1i(ul("textureShine"), 1);

	glBindSampler(0, m_SamplerID);
	//glBindSampler(1, m_SamplerID);

	RenderCards();
	RenderTable();
	RenderHand();
	RenderChips();

	glBindTextureUnit(0, 0);
	glBindSampler(0, 0);

	glBindVertexArray(0);
	// shader kikapcsolasa
	glUseProgram(0);
}

void CMyApp::RenderGUI()
{
	if (ImGui::Begin("Variables"))
	{
		ImGui::SliderAngle("Hand angle", &handAngle, -45, 45);
		ImGui::SliderInt("Tower height", &N, 1, 20);
		ImGui::SliderInt("Number of circlers", &K, 1, 20);

		if (ImGui::Button("Animate"))
		{
			lastStop = m_ElapsedTimeInSec - lastStop;

			animate = !animate;
		}

		ImGui::Checkbox("Lighting", &lightSwitch);

		ImGui::ColorEdit3("Color 1", &c1.x);
		ImGui::ColorEdit3("Color 2", &c2.x);
	}
	ImGui::End();
}

// https://wiki.libsdl.org/SDL2/SDL_KeyboardEvent
// https://wiki.libsdl.org/SDL2/SDL_Keysym
// https://wiki.libsdl.org/SDL2/SDL_Keycode
// https://wiki.libsdl.org/SDL2/SDL_Keymod

void CMyApp::KeyboardDown(const SDL_KeyboardEvent& key)
{
	if (key.repeat == 0) // Először lett megnyomva
	{
		if (key.keysym.sym == SDLK_F5 && key.keysym.mod & KMOD_CTRL)
		{
			CleanShaders();
			InitShaders();
		}
		if (key.keysym.sym == SDLK_F1)
		{
			GLint polygonModeFrontAndBack[2] = {};
			// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGet.xhtml
			glGetIntegerv(GL_POLYGON_MODE, polygonModeFrontAndBack); // Kérdezzük le a jelenlegi polygon módot! Külön adja a front és back módokat.
			GLenum polygonMode = (polygonModeFrontAndBack[0] != GL_FILL ? GL_FILL : GL_LINE); // Váltogassuk FILL és LINE között!
			// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glPolygonMode.xhtml
			glPolygonMode(GL_FRONT_AND_BACK, polygonMode); // Állítsuk be az újat!
		}

		if (key.keysym.sym == SDLK_LCTRL || key.keysym.sym == SDLK_RCTRL)
		{
			m_IsCtrlDown = true;
		}
	}
	m_cameraManipulator.KeyboardDown(key);
}

void CMyApp::KeyboardUp(const SDL_KeyboardEvent& key)
{
	m_cameraManipulator.KeyboardUp(key);
	if (key.keysym.sym == SDLK_LCTRL || key.keysym.sym == SDLK_RCTRL)
	{
		m_IsCtrlDown = false;
	}
	else if (key.keysym.sym >= SDLK_0 && key.keysym.sym <= SDLK_4)
	{
		pick = key.keysym.sym - SDLK_0;
	}
}

// https://wiki.libsdl.org/SDL2/SDL_MouseMotionEvent

void CMyApp::MouseMove(const SDL_MouseMotionEvent& mouse)
{
	m_cameraManipulator.MouseMove(mouse);
}

// https://wiki.libsdl.org/SDL2/SDL_MouseButtonEvent

void CMyApp::MouseDown(const SDL_MouseButtonEvent& mouse)
{
	if ( m_IsCtrlDown )
	{
		m_IsPicking = true;
	}
	m_PickedPixel = { mouse.x, mouse.y };
}

void CMyApp::MouseUp(const SDL_MouseButtonEvent& mouse)
{

}

// https://wiki.libsdl.org/SDL2/SDL_MouseWheelEvent

void CMyApp::MouseWheel(const SDL_MouseWheelEvent& wheel)
{
	m_cameraManipulator.MouseWheel(wheel);
}

// a két paraméterben az új ablakméret szélessége (_w) és magassága (_h)
// található
void CMyApp::Resize(int _w, int _h)
{
	glViewport(0, 0, _w, _h);
	m_windowSize = glm::uvec2(_w, _h);
	m_camera.SetAspect(static_cast<float>(_w) / _h);
}

// Le nem kezelt, egzotikus esemény kezelése
// https://wiki.libsdl.org/SDL2/SDL_Event

void CMyApp::OtherEvent(const SDL_Event& ev)
{

}