#include "Portal.h"

Portal::Portal() {
	//
}

Portal::~Portal() {
	//
}

void Portal::initialize() {
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	vector<Texture> textures;
	bluePortals.push_back(Mesh(vertices, indices, textures));
	orangePortals.push_back(Mesh(vertices, indices, textures));
}

void Portal::setPortal(int portal_type, glm::vec3 pos, glm::vec3 n, glm::vec3 up) {
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	vector<Texture> textures;
	glm::vec3 right = glm::normalize(glm::cross(up, n));
	glm::vec3 p1 = pos + n * 0.01f + up * (portal_y / 2.0f) - right * (portal_x / 2.0f);
	glm::vec3 p2 = pos + n * 0.01f + up * (portal_y / 2.0f) + right * (portal_x / 2.0f);
	glm::vec3 p3 = pos + n * 0.01f - up * (portal_y / 2.0f) + right * (portal_x / 2.0f);
	glm::vec3 p4 = pos + n * 0.01f - up * (portal_y / 2.0f) - right * (portal_x / 2.0f);
	Vertex v[4];
	v[0].Position = p1;
	v[1].Position = p2;
	v[2].Position = p3;
	v[3].Position = p4;
	v[0].TexCoords = glm::vec2(0.0f, 0.0f);
	v[1].TexCoords = glm::vec2(1.0f, 0.0f);
	v[2].TexCoords = glm::vec2(1.0f, 1.0f);
	v[3].TexCoords = glm::vec2(0.0f, 1.0f);
	for (int i = 0; i < 4; ++i) {
		v[i].Normal = n;
		vertices.push_back(v[i]);
	}
	int order[] = { 0, 1, 2, 0, 2, 3 };
	for (int i = 0; i < 6; ++i) {
		indices.push_back(order[i]);
	}
	if (portal_type == BLUE_PORTAL) {
		bluePortalExist = true;
		bluePortalPos = pos + n * 0.01f;
		bluePortalN = n;
		Texture tmpTexture;
		tmpTexture.id = TextureFromFile("blue_portal.png", "Textures/", false);
		tmpTexture.path = "blue_portal.png";
		tmpTexture.type = "texture_diffuse";
		textures.push_back(tmpTexture);
		bluePortals[0] = Mesh(vertices, indices, textures);
	}
	else {
		orangePortalExist = true;
		orangePortalPos = pos + n * 0.01f;
		orangePortalN = n;
		Texture tmpTexture;
		tmpTexture.id = TextureFromFile("orange_portal.png", "Textures/", false);
		tmpTexture.path = "orange_portal.png";
		tmpTexture.type = "texture_diffuse";
		textures.push_back(tmpTexture);
		orangePortals[0] = Mesh(vertices, indices, textures);
	}
}

void Portal::Draw(Shader shader) {
	if (bluePortalExist) {
		bluePortals[0].Draw(shader);
	}
	if (orangePortalExist) {
		orangePortals[0].Draw(shader);
	}
}

float Portal::passPortal(glm::vec3 &pos, glm::vec3 &v, glm::vec3 keyV, glm::vec3 cameraFront, float deltaTime, bool &isPass) {
	isPass = false;
	if (!bluePortalExist || !orangePortalExist) {
		return 0.0f;
	}
	glm::vec3 pos_ = pos + v * deltaTime + keyV;
	// In
	// ------
	int whichPortal = -1;
	glm::vec3 p1, p2, p3, p4;
	glm::vec2 p1_2, p2_2, p3_2, p4_2, p;
	bool passedBlue = false, passedOrange = false;
	vector<pair<double, double>> polygon;
	// Blue
	p1 = bluePortals[0].vertices[0].Position;
	p2 = bluePortals[0].vertices[1].Position;
	p3 = bluePortals[0].vertices[2].Position;
	p4 = bluePortals[0].vertices[3].Position;
	if (bluePortalN.x == 0 && bluePortalN.y == 0) {
		if ((p1.z - pos.z) * (p1.z - pos_.z) <= 0 && pos.z != pos_.z) {
			float t = (p1.z - pos.z) / (pos_.z - pos.z);
			p = glm::vec2(pos.x + t * (pos_.x - pos.x), pos.y + t * (pos_.y - pos.y));
			p1_2 = glm::vec2(p1.x, p1.y);
			p2_2 = glm::vec2(p2.x, p2.y);
			p3_2 = glm::vec2(p3.x, p3.y);
			p4_2 = glm::vec2(p4.x, p4.y);
			passedBlue = true;
		}
	}
	else if (bluePortalN.x == 0 && bluePortalN.z == 0) {
		if ((p1.y - pos.y) * (p1.y - pos_.y) <= 0 && pos.y != pos_.y) {
			float t = (p1.y - pos.y) / (pos_.y - pos.y);
			p = glm::vec2(pos.x + t * (pos_.x - pos.x), pos.z + t * (pos_.z - pos.z));
			p1_2 = glm::vec2(p1.x, p1.z);
			p2_2 = glm::vec2(p2.x, p2.z);
			p3_2 = glm::vec2(p3.x, p3.z);
			p4_2 = glm::vec2(p4.x, p4.z);
			passedBlue = true;
		}
	}
	else {
		if ((p1.x - pos.x) * (p1.x - pos_.x) <= 0 && pos.x != pos_.x) {
			float t = (p1.x - pos.x) / (pos_.x - pos.x);
			p = glm::vec2(pos.y + t * (pos_.y - pos.y), pos.z + t * (pos_.z - pos.z));
			p1_2 = glm::vec2(p1.y, p1.z);
			p2_2 = glm::vec2(p2.y, p2.z);
			p3_2 = glm::vec2(p3.y, p3.z);
			p4_2 = glm::vec2(p4.y, p4.z);
			passedBlue = true;
		}
	}
	if (passedBlue) {
		polygon.push_back(make_pair(p1_2.x, p1_2.y));
		polygon.push_back(make_pair(p2_2.x, p2_2.y));
		polygon.push_back(make_pair(p3_2.x, p3_2.y));
		polygon.push_back(make_pair(p4_2.x, p4_2.y));
		if (isInPolygon(make_pair(p.x, p.y), polygon)) {
			whichPortal = BLUE_PORTAL;
		}
	}
	// Orange
	p1 = orangePortals[0].vertices[0].Position;
	p2 = orangePortals[0].vertices[1].Position;
	p3 = orangePortals[0].vertices[2].Position;
	p4 = orangePortals[0].vertices[3].Position;
	if (orangePortalN.x == 0 && orangePortalN.y == 0) {
		if ((p1.z - pos.z) * (p1.z - pos_.z) <= 0 && pos.z != pos_.z) {
			float t = (p1.z - pos.z) / (pos_.z - pos.z);
			p = glm::vec2(pos.x + t * (pos_.x - pos.x), pos.y + t * (pos_.y - pos.y));
			p1_2 = glm::vec2(p1.x, p1.y);
			p2_2 = glm::vec2(p2.x, p2.y);
			p3_2 = glm::vec2(p3.x, p3.y);
			p4_2 = glm::vec2(p4.x, p4.y);
			passedOrange = true;
		}
	}
	else if (orangePortalN.x == 0 && orangePortalN.z == 0) {
		if ((p1.y - pos.y) * (p1.y - pos_.y) <= 0 && pos.y != pos_.y) {
			float t = (p1.y - pos.y) / (pos_.y - pos.y);
			p = glm::vec2(pos.x + t * (pos_.x - pos.x), pos.z + t * (pos_.z - pos.z));
			p1_2 = glm::vec2(p1.x, p1.z);
			p2_2 = glm::vec2(p2.x, p2.z);
			p3_2 = glm::vec2(p3.x, p3.z);
			p4_2 = glm::vec2(p4.x, p4.z);
			passedOrange = true;
		}
	}
	else {
		if ((p1.x - pos.x) * (p1.x - pos_.x) <= 0 && pos.x != pos_.x) {
			float t = (p1.x - pos.x) / (pos_.x - pos.x);
			p = glm::vec2(pos.y + t * (pos_.y - pos.y), pos.z + t * (pos_.z - pos.z));
			p1_2 = glm::vec2(p1.y, p1.z);
			p2_2 = glm::vec2(p2.y, p2.z);
			p3_2 = glm::vec2(p3.y, p3.z);
			p4_2 = glm::vec2(p4.y, p4.z);
			passedOrange = true;
		}
	}
	polygon.clear();
	if (passedOrange) {
		polygon.push_back(make_pair(p1_2.x, p1_2.y));
		polygon.push_back(make_pair(p2_2.x, p2_2.y));
		polygon.push_back(make_pair(p3_2.x, p3_2.y));
		polygon.push_back(make_pair(p4_2.x, p4_2.y));
		if (isInPolygon(make_pair(p.x, p.y), polygon)) {
			whichPortal = ORANGE_PORTAL;
		}
	}
	// Out
	// ------
	float speedABS = v.length();
	if (whichPortal == BLUE_PORTAL) {
		pos = orangePortalPos + orangePortalN * 1.0f + glm::vec3(0.0f, 0.0f, 2.0f);
		// v = orangePortalN * speedABS;
	}
	else if (whichPortal == ORANGE_PORTAL) {
		pos = bluePortalPos + bluePortalN * 1.0f + glm::vec3(0.0f, 0.0f, 2.0f);
		// v = bluePortalN * speedABS;
	}
	if (whichPortal >= 0) {
		isPass = true;
		if (orangePortalN.z != 0.0f || bluePortalN.z != 0.0f) {
			return 0.0f;
		}
		if (whichPortal == BLUE_PORTAL) {
			cout << "into blue" << endl;
			return -angleBetween(cameraFront.x, cameraFront.y, orangePortalN.x, orangePortalN.y);
		}
		else {
			cout << "into orange" << endl;
			return -angleBetween(cameraFront.x, cameraFront.y, bluePortalN.x, bluePortalN.y);
		}
	}
	return 0.0f;
}

bool Portal::isInPolygon(pair<double, double> pos, vector<pair<double, double>> polygon) {
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

double Portal::angleBetween(double x1, double y1, double x2, double y2) {
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
