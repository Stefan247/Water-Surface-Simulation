#pragma once

#include <Component/SimpleScene.h>

class WaterSimulation : public SimpleScene
{
public:
	WaterSimulation();
	~WaterSimulation();

	void Init() override;

private:
	void FrameStart() override;
	void Update(float deltaTimeSeconds) override;
	void FrameEnd() override;

	void DrawSky();
	void RenderWater(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, Texture2D* texture, const glm::vec3& color = glm::vec3(1));

	unsigned int UploadCubeMapTexture(const std::string& posx, const std::string& posy, const std::string& posz, const std::string& negx, const std::string& negy, const std::string& negz);

	void OnInputUpdate(float deltaTime, int mods) override;
	void OnKeyPress(int key, int mods) override;
	void OnKeyRelease(int key, int mods) override;
	void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
	void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
	void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
	void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
	void OnWindowResize(int width, int height) override;


protected:
	std::unordered_map<std::string, Texture2D*> mapTextures;
	int  cubeMapTextureID;
	glm::mat4 modelMatrix;

	glm::vec3 lightPosition;
	unsigned int materialShininess;
	float materialKd;
	float materialKs;
};
