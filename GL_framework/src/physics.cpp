#include <imgui\imgui.h>
#include <imgui\imgui_impl_glfw_gl3.h>
#include <glm\gtx\rotate_vector.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\constants.hpp>
#include <glm/gtx/norm.hpp>
#include <vector>
#include <iostream>

bool show_test_window = false;
namespace Sphere {
	extern void setupSphere(glm::vec3 pos = glm::vec3(0.f, 1.f, 0.f), float radius = 1.f);
	extern void cleanupSphere();
	extern void updateSphere(glm::vec3 pos, float radius = 1.f);
	extern void drawSphere();
}
namespace Capsule {
	extern void setupCapsule(glm::vec3 posA = glm::vec3(-3.f, 2.f, -2.f), glm::vec3 posB = glm::vec3(-4.f, 2.f, 2.f), float radius = 1.f);
	extern void cleanupCapsule();
	extern void updateCapsule(glm::vec3 posA, glm::vec3 posB, float radius = 1.f);
	extern void drawCapsule();
}
namespace LilSpheres {
	extern const int maxParticles;
	extern void setupParticles(int numTotalParticles, float radius = 0.05f);
	extern void cleanupParticles();
	extern void updateParticles(int startIdx, int count, float* array_data);
	extern void drawParticles(int startIdx, int count);
}
namespace ClothMesh {
	extern const int numCols;
	extern const int numRows;
	extern const int numVerts;
	extern void setupClothMesh();
	extern void cleanupClothMesh();
	extern void updateClothMesh(float* array_data);
	extern void drawClothMesh();
}

struct Wave{
	float amplitude;
	float frequency;
	glm::vec3 waveVector;
	float waveLength;
	float k;
	Wave(float a, float freq,float length ,glm::vec3 vec){
		amplitude = a;
		frequency = freq;
		waveLength = length;
		waveVector = glm::normalize(vec);
		k = glm::two_pi<float>() / waveLength;
	}
};

class Mesh {
public:
	Mesh() {
		distVertex = 0.5;
		heightPos = 5;
		time = 0;
		position = new glm::vec3[ClothMesh::numVerts];
		originalPos = new glm::vec3[ClothMesh::numVerts];
		setInitPos();

		//WAVES

		Wave wave(0.5f, 2.f, 5.f, glm::vec3(1, 0, 0));
		Wave wave2(0.2, 3, 10, glm::vec3(0, 0, 1));
		waves.push_back(wave);
		waves.push_back(wave2);

	}
	~Mesh() {

	}
	void setInitPos() {
		for (int i = 0; i < ClothMesh::numRows; i++) {
			for (int j = 0; j < ClothMesh::numCols; j++) {
				position[j + i*ClothMesh::numCols] = glm::vec3(i*distVertex - (distVertex*ClothMesh::numRows / 2), heightPos, j*distVertex - (distVertex*ClothMesh::numCols / 2));
				originalPos[j + i*ClothMesh::numCols] = position[j + i*ClothMesh::numCols];
			}
		}
	}

	void Update(float& dt) {
		time += dt;
		for (int i = 0; i < ClothMesh::numVerts; i++) {
			//std::cout << waves.size();
			position[i] = glm::vec3(0);
			for (int j = 0; j < waves.size(); j++) {
				position[i] += originalPos[i] - (waves[j].waveVector / waves[j].k)*waves[j].amplitude*glm::sin(glm::dot(waves[j].waveVector, originalPos[i]) - waves[j].frequency*time);
				position[i].y += (waves[j].amplitude*glm::cos(glm::dot(waves[j].waveVector, originalPos[i]) - waves[j].frequency*time));
			}
			//position[i] += heightPos;
		}
	}

	float time;
	float distVertex;
	float heightPos;
	std::vector<Wave> waves;
	
	glm::vec3* position;
	glm::vec3* originalPos;
};



void GUI() {
	{	//FrameRate
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		//TODO
	}

	// ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
	if(show_test_window) {
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
	}
}

Mesh mesh;
void PhysicsInit() {
	//TODO
}
void PhysicsUpdate(float dt) {
	mesh.Update(dt);

	ClothMesh::updateClothMesh(&mesh.position[0].x);
}
void PhysicsCleanup() {
	//TODO
}