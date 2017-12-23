#ifndef PHYSICS_H
#define PHYSICS_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

#define MAP_WIDTH 200

const double g = 20.0;

class Physics {
private:
	vector<pair<vector<pair<double, double>>, double>> xPlanes;
	vector<pair<vector<pair<double, double>>, double>> yPlanes;
	vector<pair<vector<pair<double, double>>, double>> zPlanes;
	glm::vec3 worldUp;

public:
	Physics(string const &path, glm::vec3 up);
	~Physics();

	void updateVerticleState(double &v, glm::vec3 &pos, double deltaTime, bool &isJumping);
	bool isHorizontalAvailable(glm::vec3 pos, glm::vec3 movement);

private:
	bool isInPolygon(pair<double, double> pos, vector<pair<double, double>> polygon);
	double angleBetween(double x1, double y1, double x2, double y2);
};

#endif // PHSICS_H
