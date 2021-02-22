#include "WaterSimulation.h"
#include <vector>
#include <iostream>
#include <math.h>
#include <algorithm>
#include <stb/stb_image.h>
#include <Core/Engine.h>
#include <Laboratoare\WaterSim\Transform3D.h>
#define M_PI	3.14159265358979323846

using namespace std;

WaterSimulation::WaterSimulation()
{
}

WaterSimulation::~WaterSimulation()
{
}

void WaterSimulation::Init()
{
	auto camera = GetSceneCamera();
	camera->SetPositionAndRotation(glm::vec3(0, 30, 30), glm::quat(glm::vec3(-40 * TO_RADIANS, 0, 0)));
	camera->Update();

	// SHADERS
	{
		Shader* shader = new Shader("WaterShader");
		shader->AddShader("Source/Laboratoare/WaterSim/Shaders/WaterVertex.glsl", GL_VERTEX_SHADER);
		shader->AddShader("Source/Laboratoare/WaterSim/Shaders/WaterFragment.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}

	// CUBEMAP
	{
		Shader* shader = new Shader("ShaderNormal");
		shader->AddShader("Source/Laboratoare/WaterSim/Shaders/NormalVS.glsl", GL_VERTEX_SHADER);
		shader->AddShader("Source/Laboratoare/WaterSim/Shaders/NormalFS.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}

	cubeMapTextureID = UploadCubeMapTexture(
		"Source/Laboratoare/WaterSim/Media/Tema1Cube/right.jpg",
		"Source/Laboratoare/WaterSim/Media/Tema1Cube/top.jpg",
		"Source/Laboratoare/WaterSim/Media/Tema1Cube/back.jpg",
		"Source/Laboratoare/WaterSim/Media/Tema1Cube/left.jpg",
		"Source/Laboratoare/WaterSim/Media/Tema1Cube/bottom.jpg",
		"Source/Laboratoare/WaterSim/Media/Tema1Cube/front.jpg"
	);

	// MESHES
	{
		Mesh* mesh = new Mesh("cube");
		mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "sphere.obj");
		mesh->UseMaterials(false);
		meshes[mesh->GetMeshID()] = mesh;
	}

	{
		Mesh* mesh = new Mesh("sun");
		mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "sphere.obj");
		meshes[mesh->GetMeshID()] = mesh;
	}

	{
		Mesh* mesh = new Mesh("water_surface");
		mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "plane50.obj");
		mesh->UseMaterials(false);
		meshes[mesh->GetMeshID()] = mesh;
	}

	// ocean mesh
	int resolution = 200;
	int size = 200;
	{
		double posX, posZ;
		vector<VertexFormat> vertices;
		vector<unsigned short> indices;

		VertexFormat vertex = VertexFormat(glm::vec3(0, 0, 0), glm::vec3(0.1f, 0.1f, 0.3f));

		for (unsigned i = 0; i < resolution; i++) {
			for (unsigned j = 0; j < resolution; j++) {

				posX = -(double)size / 2 + (double)i / (resolution - 1) * size;
				posZ = -(double)size / 2 + (double)j / (resolution - 1) * size;

				vertex.position = glm::vec3(posX, 0.5f, posZ);
				vertex.text_coord = glm::vec2(i, j);
				vertices.push_back(vertex);
			}
		}

		for (unsigned j = 1; j < resolution; j++)
		{
			for (unsigned i = 1; i < resolution; i++)
			{
				indices.push_back(i - 1 + (j - 1) * resolution);
				indices.push_back(i + (j - 1) * resolution);
				indices.push_back(i - 1 + j * resolution);

				indices.push_back(i + (j - 1) * resolution);
				indices.push_back(i + j * resolution);
				indices.push_back(i - 1 + j * resolution);
			}
		}

		Mesh* waterMesh = new Mesh("water_surfac");
		waterMesh->InitFromData(vertices, indices);
		meshes[waterMesh->GetMeshID()] = waterMesh;
	}

	// TEXTURES
	{
		Texture2D* texture_1 = new Texture2D();
		texture_1->Load2D("Source/Laboratoare/WaterSim/Media/tex_Water.png", GL_REPEAT);
		mapTextures["water"] = texture_1;
	}

	{
		Texture2D* texture_1 = new Texture2D();
		texture_1->Load2D("Source/Laboratoare/WaterSim/Media/water_texturee.png", GL_REPEAT);
		mapTextures["water2"] = texture_1;
	}

	//Light & material properties
	{
		lightPosition = glm::vec3(-35, 35, -65);
		materialShininess = 100;
		materialKd = 0.9;
		materialKs = 0.9;
	}
}

unsigned int WaterSimulation::UploadCubeMapTexture(const std::string& posx, const std::string& posy, const std::string& posz, const std::string& negx, const std::string& negy, const std::string& negz)
{
	int width, height, chn;

	unsigned char* data_posx = stbi_load(posx.c_str(), &width, &height, &chn, 0);
	unsigned char* data_posy = stbi_load(posy.c_str(), &width, &height, &chn, 0);
	unsigned char* data_posz = stbi_load(posz.c_str(), &width, &height, &chn, 0);
	unsigned char* data_negx = stbi_load(negx.c_str(), &width, &height, &chn, 0);
	unsigned char* data_negy = stbi_load(negy.c_str(), &width, &height, &chn, 0);
	unsigned char* data_negz = stbi_load(negz.c_str(), &width, &height, &chn, 0);

	// create OpenGL texture
	unsigned int textureID = 0;
	glGenTextures(1, &textureID);

	// bind the texture
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	float maxAnisotropy;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// load texture information for each face
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_posx);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_posy);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_posz);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_negx);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_negy);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_negz);

	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	// free memory
	SAFE_FREE(data_posx);
	SAFE_FREE(data_posy);
	SAFE_FREE(data_posz);
	SAFE_FREE(data_negx);
	SAFE_FREE(data_negy);
	SAFE_FREE(data_negz);

	return textureID;
}

void WaterSimulation::DrawSky()
{
	{
		Shader* shader = shaders["ShaderNormal"];
		shader->Use();

		glm::mat4 modelMatrix = glm::scale(glm::mat4(1), glm::vec3(170));

		glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));
		glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(GetSceneCamera()->GetViewMatrix()));
		glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(GetSceneCamera()->GetProjectionMatrix()));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureID);
		int loc_texture = shader->GetUniformLocation("texture_cubemap");
		glUniform1i(loc_texture, 0);

		meshes["cube"]->Render();
	}
}

void WaterSimulation::RenderWater(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, Texture2D* texture_1, const glm::vec3& color)
{
	if (!mesh || !shader || !shader->GetProgramID())
		return;

	// render an object using the specified shader and the specified position
	glUseProgram(shader->program);

	// Bind model matrix
	GLint loc_model_matrix = glGetUniformLocation(shader->program, "Model");
	glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

	// Bind view matrix
	glm::mat4 viewMatrix = GetSceneCamera()->GetViewMatrix();
	int loc_view_matrix = glGetUniformLocation(shader->program, "View");
	glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));

	// Bind projection matrix
	glm::mat4 projectionMatrix = GetSceneCamera()->GetProjectionMatrix();
	int loc_projection_matrix = glGetUniformLocation(shader->program, "Projection");
	glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

	// time component
	int loc_time = glGetUniformLocation(shader->program, "time");
	glUniform1f(loc_time, (float)Engine::GetElapsedTime());

	// light uniforms
	int loc_light_position = glGetUniformLocation(shader->program, "light_position");
	glUniform3fv(loc_light_position, 1, glm::value_ptr(lightPosition));

	int loc_shininess = glGetUniformLocation(shader->program, "material_shininess");
	glUniform1i(loc_shininess, materialShininess);

	int loc_kd = glGetUniformLocation(shader->program, "material_kd");
	glUniform1f(loc_kd, materialKd);

	int loc_ks = glGetUniformLocation(shader->program, "material_ks");
	glUniform1f(loc_ks, materialKs);

	if (texture_1)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_1->GetTextureID());
		glUniform1i(glGetUniformLocation(shader->GetProgramID(), "texture_1"), 0);
	}

	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureID);
	int loc_texture = shader->GetUniformLocation("texture_cubemap");
	glUniform1i(loc_texture, 1);

	int loc_camera_pos = glGetUniformLocation(shader->program, "camera_pos");
	glm::vec3 camera_position = GetSceneCamera()->transform->GetWorldPosition();
	glUniform3f(loc_camera_pos, camera_position.x, camera_position.y, camera_position.z);

	// Draw the object
	glBindVertexArray(mesh->GetBuffers()->VAO);
	glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_SHORT, 0);
}

void WaterSimulation::FrameStart()
{
	// clears the color buffer (using the previously set color) and depth buffer
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::ivec2 resolution = window->GetResolution();
	// sets the screen area where to draw
	glViewport(0, 0, resolution.x, resolution.y);
}

void WaterSimulation::Update(float deltaTimeSeconds)
{
	// ClearScreen();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	DrawSky();

	{
		glm::mat4 modelMatrix = glm::mat4(1);
		modelMatrix = glm::translate(modelMatrix, lightPosition);
		modelMatrix = glm::scale(modelMatrix, glm::vec3(25.0f));
		RenderMesh(meshes["sun"], shaders["Normal"], modelMatrix);
	}

	modelMatrix = glm::mat4(1);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0,-10, 0));

	RenderWater(meshes["water_surfac"], shaders["WaterShader"], modelMatrix, mapTextures["water"], glm::vec3(0, 0, 1));
}

void WaterSimulation::FrameEnd()
{
	// DrawCoordinatSystem();
}

void WaterSimulation::OnInputUpdate(float deltaTime, int mods)
{
	// treat continuous update based on input
};

void WaterSimulation::OnKeyPress(int key, int mods)
{
	// add key press event
};

void WaterSimulation::OnKeyRelease(int key, int mods)
{
	// add key release event
};

void WaterSimulation::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
	// add mouse move event
};

void WaterSimulation::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
	// add mouse button press event
};

void WaterSimulation::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
	// add mouse button release event
}

void WaterSimulation::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
	// treat mouse scroll event
}

void WaterSimulation::OnWindowResize(int width, int height)
{
	// treat window resize event
}
