#include "Physics.h"

Physics::Physics(string const &path, glm::vec3 up) {
	worldUp = up;
	ifstream inFile(path);
	if (!inFile) {
		std::cout << "Map failed to load at path: " << path << std::endl;
		return;
	}
	while (true) {
		if (inFile.eof()) {
			break;
		}
		char type;
		inFile >> type;
		if (type == 'R') {
			double p[4][3], n[3];
			if (inFile.eof()) {
				break;
			}
			for (int i = 0; i < 4; ++i) {
				for (int j = 0; j < 3; ++j) {
					inFile >> p[i][j];
				}
			}
			char nCh;
			inFile >> nCh;
			for (int i = 0; i < 3; ++i) {
				inFile >> n[i];
			}
			char tCh;
			inFile >> tCh;
			int textureId = 0;
			inFile >> textureId;
			if (n[0] == 0 && n[1] == 0) {
				vector<pair<double, double>> plane;
				for (int i = 0; i < 4; ++i) {
					plane.push_back(make_pair(p[i][0], p[i][1]));
				}
				if (textureId == 3) {
					whiteWalls.push_back(make_pair(2, zPlanes.size()));
				}
				zPlanes.push_back(make_pair(plane, p[0][2]));
			}
			else if (n[0] == 0 && n[2] == 0) {
				vector<pair<double, double>> plane;
				for (int i = 0; i < 4; ++i) {
					plane.push_back(make_pair(p[i][0], p[i][2]));
				}
				if (textureId == 3) {
					whiteWalls.push_back(make_pair(1, yPlanes.size()));
				}
				yPlanes.push_back(make_pair(plane, p[0][1]));
			}
			else if (n[1] == 0 && n[2] == 0) {
				vector<pair<double, double>> plane;
				for (int i = 0; i < 4; ++i) {
					plane.push_back(make_pair(p[i][1], p[i][2]));
				}
				if (textureId == 3) {
					whiteWalls.push_back(make_pair(0, xPlanes.size()));
				}
				xPlanes.push_back(make_pair(plane, p[0][0]));
			}
		}
		else {
			break;
		}
	}
	inFile.close();
}

Physics::~Physics() {
}

void Physics::updateVerticleState(glm::vec3 &v, glm::vec3 &pos, double deltaTime, bool &isJumping) {
	double v_ = v.z + g * deltaTime;
	double h = (v_ * v_ - v.z * v.z) / (2 * g);
	glm::vec3 pos_ = pos - worldUp * (float)h;
	for (int i = 0; i < zPlanes.size(); ++i) {
		if (!(pos.z >= zPlanes[i].second && pos_.z <= zPlanes[i].second)) {
			continue;
		}
		vector<pair<double, double>> polygon = zPlanes[i].first;
		if (!isInPolygon(make_pair(pos.x, pos.y), polygon)) {
			continue;
		}
		isJumping = false;
		pos.z = zPlanes[i].second;
		v = glm::vec3(0.0f, 0.0f, 0.0f);
		return;
	}
	pos = pos_;
	v = glm::vec3(v.x, v.y, v_);
}

bool Physics::isHorizontalAvailable(glm::vec3 &pos, glm::vec3 movement) {
	glm::vec3 pos_ = pos + movement;
	double movementX = movement.x;
	double movementY = movement.y;
	pair<double, double> pos2;
	// X
	pos2 = make_pair(pos.y, pos.z);
	for (int i = 0; i < xPlanes.size(); ++i) {
		if (!((pos.x <= xPlanes[i].second && pos_.x >= xPlanes[i].second) || (pos.x >= xPlanes[i].second && pos_.x <= xPlanes[i].second))) {
			continue;
		}
		vector<pair<double, double>> polygon = xPlanes[i].first;
		if (!isInPolygon(pos2, polygon)) {
			continue;
		}
		return false;
	}
	// Y
	pos2 = make_pair(pos.x, pos.z);
	for (int i = 0; i < yPlanes.size(); ++i) {
		if (!((pos.y <= yPlanes[i].second && pos_.y >= yPlanes[i].second) || (pos.y >= yPlanes[i].second && pos_.y <= yPlanes[i].second))) {
			continue;
		}
		vector<pair<double, double>> polygon = yPlanes[i].first;
		if (!isInPolygon(pos2, polygon)) {
			continue;
		}
		return false;
	}
	return true;
}

bool Physics::isInPolygon(pair<double, double> pos, vector<pair<double, double>> polygon) {
	double x = pos.first, y = pos.second;
	double totalAngle = 0;
	for (int i = 0; i < polygon.size(); ++i) {
		double x1 = polygon[i].first - x;
		double y1 = polygon[i].second - y;
		double x2 = (i == polygon.size() - 1) ? (polygon[0].first - x) : (polygon[i + 1].first - x);
		double y2 = (i == polygon.size() - 1) ? (polygon[0].second - y) : (polygon[i + 1].second - y);
		double a = angleBetween(x1, y1, x2, y2);
		totalAngle += a;
	}
	return (abs(totalAngle) > 6);
}

double Physics::angleBetween(double x1, double y1, double x2, double y2) {
	double sinValue = (x1 * y2 - x2 * y1) / (sqrt(x1 * x1 + y1 * y1) * sqrt(x2 * x2 + y2 * y2));
	double cosValue = (x1 * x2 + y1 * y2) / (sqrt(x1 * x1 + y1 * y1) * sqrt(x2 * x2 + y2 * y2));
	if (cosValue >= 1.0) {
		return 0;
	}
	if (cosValue <= -1.0) {
		return 3.1416;
	}
	double a = acos(cosValue);
	if (sinValue < 0) {
		a = -a;
	}
	return a;
}

bool Physics::isIntersected(glm::vec3 playerPos, glm::vec3 lookat, glm::vec3 &pos, glm::vec3 &n, glm::vec3 &up) {
	double x = playerPos.x, y = playerPos.y, z = playerPos.z;
	double dx = lookat.x, dy = lookat.y, dz = lookat.z;
	double tMin = DBL_MAX;
	int minX = 0, minY = 0;
	for (int i = 0; i < xPlanes.size(); ++i) {
		double x0 = xPlanes[i].second;
		if (dx == 0) {
			continue;
		}
		double t = (x0 - x) / dx;
		if (t <= 0 || t > tMin) {
			continue;
		}
		pair<double, double> pos2 = make_pair(y + dy * t, z + dz * t);
		vector<pair<double, double>> polygon = xPlanes[i].first;
		if (isInPolygon(pos2, polygon)) {
			tMin = t;
			minX = 0;
			minY = i;
			pos = glm::vec3(x0, y + dy * t, z + dz * t);
			n = (x0 > x) ? glm::vec3(-1.0f, 0.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
			up = glm::vec3(0.0f, 0.0f, 1.0f);
		}
	}
	for (int i = 0; i < yPlanes.size(); ++i) {
		double y0 = yPlanes[i].second;
		if (dy == 0) {
			continue;
		}
		double t = (y0 - y) / dy;
		if (t <= 0 || t > tMin) {
			continue;
		}
		pair<double, double> pos2 = make_pair(x + dx * t, z + dz * t);
		vector<pair<double, double>> polygon = yPlanes[i].first;
		if (isInPolygon(pos2, polygon)) {
			tMin = t;
			minX = 1;
			minY = i;
			pos = glm::vec3(x + dx * t, y0, z + dz * t);
			n = (y0 > y) ? glm::vec3(0.0f, -1.0f, 0.0f) : glm::vec3(0.0f, 1.0f, 0.0f);
			up = glm::vec3(0.0f, 0.0f, 1.0f);
		}
	}
	for (int i = 0; i < zPlanes.size(); ++i) {
		double z0 = zPlanes[i].second;
		if (dz == 0) {
			continue;
		}
		double t = (z0 - z) / dz;
		if (t <= 0 || t > tMin) {
			continue;
		}
		pair<double, double> pos2 = make_pair(x + dx * t, y + dy * t);
		vector<pair<double, double>> polygon = zPlanes[i].first;
		if (isInPolygon(pos2, polygon)) {
			tMin = t;
			minX = 2;
			minY = i;
			pos = glm::vec3(x + dx * t, y + dy * t, z0);
			n = (z0 > z) ? glm::vec3(0.0f, 0.0f, -1.0f) : glm::vec3(0.0f, 0.0f, 1.0f);
			up = glm::normalize(glm::vec3(dx, dy, 0.0f));
		}
	}
	if (tMin == DBL_MAX) {
		return false;
	}
	if (isWallWhite(minX, minY)) {
		return true;
	}
	return false;
}

bool Physics::isWallWhite(int x, int y) {
	for (int i = 0; i < whiteWalls.size(); ++i) {
		if (x == whiteWalls[i].first && y == whiteWalls[i].second) {
			return true;
		}
	}
	return false;
}
