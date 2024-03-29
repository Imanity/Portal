#ifndef PHYSICS_H
#define PHYSICS_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include <climits>

using namespace std;

#define MAP_WIDTH 200

const double g = 20.0;

class Physics {
private:
	vector<pair<vector<pair<double, double>>, double>> xPlanes;
	vector<pair<vector<pair<double, double>>, double>> yPlanes;
	vector<pair<vector<pair<double, double>>, double>> zPlanes;
	glm::vec3 worldUp;
	vector<pair<int, int>> whiteWalls;
	glm::vec3 winPoint;

public:
	Physics(string const &path, string const &winPath, glm::vec3 up);
	~Physics();

	void updateVerticleState(glm::vec3 &v, glm::vec3 &pos, double deltaTime, bool &isJumping);
	bool isHorizontalAvailable(glm::vec3 &pos, glm::vec3 movement);
	bool isIntersected(glm::vec3 playerPos, glm::vec3 lookat, glm::vec3 &pos, glm::vec3 &n, glm::vec3 &up);
	bool isWin(glm::vec3 &pos);

private:
	bool isInPolygon(pair<double, double> pos, vector<pair<double, double>> polygon);
	double angleBetween(double x1, double y1, double x2, double y2);
	bool isWallWhite(int x, int y);
};

#endif // PHSICS_H
