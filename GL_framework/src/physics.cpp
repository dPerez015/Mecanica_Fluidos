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
	//extern float radius;
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

glm::vec3 gravity(0,-9.8,0);

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
		distVertex =0.65;
		heightPos = 5;
		time = 0;
		position = new glm::vec3[ClothMesh::numVerts];
		originalPos = new glm::vec3[ClothMesh::numVerts];
		setInitPos();

		density = 10;
		//WAVES

		Wave wave(0.2f, 4.f, 0.5f, glm::vec3(1, 1, 0));
		Wave wave2(0.15, 3, 1, glm::vec3(-1, 0, 1));
		Wave wave3(0.25, 2,6,glm::vec3(-1.2,0,-3));
		waves.push_back(wave);
		waves.push_back(wave2);
		waves.push_back(wave3);
	}
	~Mesh() {
		delete[] position;
		delete[] originalPos;
		waves.clear();
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
		float resultSinus;
		for (int i = 0; i < ClothMesh::numVerts; i++) {
			//std::cout << waves.size();
			position[i] = glm::vec3(0);
			for (int j = 0; j < waves.size(); j++) {
				resultSinus= waves[j].amplitude*glm::sin(glm::dot(waves[j].waveVector, originalPos[i]) - waves[j].frequency*time);
				position[i].x += (waves[j].waveVector.x / waves[j].k)*resultSinus;
				position[i].z +=  (waves[j].waveVector.z / waves[j].k)*resultSinus;

				position[i].y += (waves[j].amplitude*glm::cos(glm::dot(waves[j].waveVector, originalPos[i]) - waves[j].frequency*time));
			}
			position[i].x = originalPos[i].x - position[i].x;
			position[i].y += heightPos;
			position[i].z = originalPos[i].z - position[i].z;
		}
	}

	float time;
	float distVertex;
	float heightPos;

	float density;

	std::vector<Wave> waves;
	
	glm::vec3* position;
	glm::vec3* originalPos;
};

class FloatingSphere {
public:
	FloatingSphere() {
		radius = 0.5f;
		InitRandomPos();
		density = 1;
	}
	~FloatingSphere() {
	
	}
	void InitRandomPos() {
		position.x = ((rand() / RAND_MAX) - 0.5f) * 5;
		position.y = 9;
		position.z = ((rand() / RAND_MAX) - 0.5f) * 5;
	}
	void findPointHeight(std::vector<Wave>& waves, float originalHeight, float time) {
		pointHeight = 0;
		for (int i = 0; i < waves.size(); i++) {
			pointHeight += (waves[i].amplitude*glm::cos(glm::dot(waves[i].waveVector, glm::vec3(position.x, originalHeight, position.z)) - waves[i].frequency*time));
		}
		pointHeight += originalHeight;
	}
	void findH() {
		h = pointHeight - position.y+radius;

		if (h > 2 * radius) {
			isColliding = true;
			use2Hemi = false;
			isSubmerged = true;
		}
		else if (h > radius) {
			isColliding = true;
			use2Hemi = true;
			isSubmerged = false;
		}
		else if (h > 0) {
			isColliding = true;
			use2Hemi = false;
			isSubmerged = false;
		}
		else {
			isColliding = false;
			use2Hemi = false;
			isSubmerged = false;
		}
	}

	void findA() {	
		if (use2Hemi) {
			a = glm::sqrt(glm::pow(radius, 2) - glm::pow(h-radius, 2));
		}
		else {
			a= glm::sqrt(glm::pow(radius, 2) - glm::pow(radius-h, 2));
		}
	}
	void computeVolume() {
		if (use2Hemi) {//si la esfera esta a mas de la mitad calculamos el volumen de una mitad y el resto
			h -= radius;
			volume = glm::pi<float>()*glm::two_thirds<float>()*glm::pow(radius, 3);
			volume += volume - ((glm::pi<float>()*h / 6)*(3 * glm::pow(a, 2) + glm::pow(h, 2)));
		}
		else {
			volume= ((glm::pi<float>()*h / 6)*(3 * glm::pow(a, 2) + glm::pow(h, 2)));
		}
	}
	void computeForce(float& fluidDensity) {
		if (isSubmerged) {
			buoyancyForce = fluidDensity*gravity.y*totalVolume*(-1);
		}
		else {
			buoyancyForce = fluidDensity*gravity.y*volume*(-1);
		}
	}
	void Update(float& dt, Mesh& theMesh ) {
		totalVolume = glm::pi<float>()*glm::pow(radius, 3);
		mass = density*(4.f / 3.f)*totalVolume;
		force = gravity*mass;
		findPointHeight(theMesh.waves, theMesh.heightPos, theMesh.time);
		findH();
		if (isColliding) {
			if (!isSubmerged) {
				findA();
				computeVolume();
			}
			computeForce(theMesh.density);
			force.y += buoyancyForce;
		}
		velocity += (force/mass)*dt;
		position += velocity*dt;
		Sphere::updateSphere(position, radius);
	}

	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec3 force;
	float buoyancyForce;
	float pointHeight;//altura de las olas en la posicion de la esfera
	float h, a;//height of the caps and amplitude of the base (radius of the base)
	float radius;
	float volume;
	float totalVolume;
	float density;
	float mass;
	bool use2Hemi;
	bool isColliding;
	bool isSubmerged;
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
FloatingSphere sphere;
void PhysicsInit() {
	//TODO
}
void PhysicsUpdate(float dt) {
	mesh.Update(dt);
	sphere.Update(dt, mesh);
	ClothMesh::updateClothMesh(&mesh.position[0].x);
}
void PhysicsCleanup() {
	//TODO
}