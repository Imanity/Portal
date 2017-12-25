#ifndef PORTAL_H
#define PORTAL_H

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>

#include "Mesh.h"
#include "Shader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

using namespace std;

const float portal_x = 1.5f;
const float portal_y = 2.7f;

#define BLUE_PORTAL 0
#define ORANGE_PORTAL 1

extern unsigned int TextureFromFile(const char *path, const string &directory, bool gamma);

class Portal {
public:
	vector<Mesh> bluePortals;
	vector<Mesh> orangePortals;
	bool bluePortalExist = false;
	bool orangePortalExist = false;
	glm::vec3 bluePortalPos, orangePortalPos;
	glm::vec3 bluePortalN, orangePortalN;

public:
	Portal();
	~Portal();

	void initialize();
	void setPortal(int portal_type, glm::vec3 pos, glm::vec3 n, glm::vec3 up);
	void Draw(Shader shader);
	void DrawSingle(Shader shader, int id);

	float passPortal(glm::vec3 &pos, glm::vec3 &v, glm::vec3 keyV, glm::vec3 cameraFront, float deltaTime, bool &isPass);

private:
	bool isInPolygon(pair<double, double> pos, vector<pair<double, double>> polygon);
	double angleBetween(double x1, double y1, double x2, double y2);
};

#endif
