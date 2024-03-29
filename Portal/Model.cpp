#include "Model.h"

unsigned int TextureFromFile(const char *path, const string &directory, bool gamma) {
	string filename = string(path);
	filename = directory + '/' + filename;

	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
	if (data) {
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	} else {
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

Model::Model(string const &path, bool gamma) : gammaCorrection(gamma) {
	//loadModel(path);
	loadMap(path);
}

// draws the model, and thus all its meshes
void Model::Draw(Shader shader) {
	for (unsigned int i = 0; i < meshes.size(); i++)
		meshes[i].Draw(shader);
}

void Model::DrawExcept(Shader shader, glm::vec3 pos, glm::vec3 n) {
	for (unsigned int i = 0; i < singleMeshes.size(); i++) {
		Vertex v = singleMeshes[i].vertices[0];
		if (abs(v.Normal.x) == abs(n.x) && abs(v.Normal.y) == abs(n.y) && (
			(n.x == 1.0f && v.Position.x < pos.x + 1.0f) || 
			(n.x == -1.0f && v.Position.x > pos.x - 1.0f) ||
			(n.y == 1.0f && v.Position.y < pos.y + 1.0f) || 
			(n.y == -1.0f && v.Position.y > pos.y - 1.0f))) {
			continue;
		}
		singleMeshes[i].Draw(shader);
	}
}

void Model::loadModel(string const &path) {
	// read file via ASSIMP
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
	// check for errors
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
	{
		cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
		return;
	}
	// retrieve the directory path of the filepath
	directory = path.substr(0, path.find_last_of('/'));

	// process ASSIMP's root node recursively
	processNode(scene->mRootNode, scene);
}

// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
void Model::processNode(aiNode *node, const aiScene *scene) {
	// process each mesh located at the current node
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		// the node object only contains indices to index the actual objects in the scene. 
		// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processMesh(mesh, scene));
	}
	// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		processNode(node->mChildren[i], scene);
	}
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene) {
	// data to fill
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	vector<Texture> textures;

	// Walk through each of the mesh's vertices
	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		Vertex vertex;
		glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
						  // positions
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.Position = vector;
		// normals
		vector.x = mesh->mNormals[i].x;
		vector.y = mesh->mNormals[i].y;
		vector.z = mesh->mNormals[i].z;
		vertex.Normal = vector;
		// texture coordinates
		if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
		{
			glm::vec2 vec;
			// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
			// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = vec;
		}
		else
			vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		// tangent
		vector.x = mesh->mTangents[i].x;
		vector.y = mesh->mTangents[i].y;
		vector.z = mesh->mTangents[i].z;
		vertex.Tangent = vector;
		// bitangent
		vector.x = mesh->mBitangents[i].x;
		vector.y = mesh->mBitangents[i].y;
		vector.z = mesh->mBitangents[i].z;
		vertex.Bitangent = vector;
		vertices.push_back(vertex);
	}
	// now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		// retrieve all indices of the face and store them in the indices vector
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}
	// process materials
	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	// we assume a convention for sampler names in the shaders. Each diffuse texture should be named
	// as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
	// Same applies to other texture as the following list summarizes:
	// diffuse: texture_diffuseN
	// specular: texture_specularN
	// normal: texture_normalN

	// 1. diffuse maps
	vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
	textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
	// 2. specular maps
	vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
	textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	// 3. normal maps
	std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
	textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
	// 4. height maps
	std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
	textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

	// return a mesh object created from the extracted mesh data
	return Mesh(vertices, indices, textures);
}

// checks all material textures of a given type and loads the textures if they're not loaded yet.
// the required info is returned as a Texture struct.
vector<Texture> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName) {
	vector<Texture> textures;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		// check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
		bool skip = false;
		for (unsigned int j = 0; j < textures_loaded.size(); j++) {
			if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
			{
				textures.push_back(textures_loaded[j]);
				skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
				break;
			}
		}
		if (!skip) {   // if texture hasn't been loaded already, load it
			Texture texture;
			texture.id = TextureFromFile(str.C_Str(), this->directory);
			texture.type = typeName;
			texture.path = str.C_Str();
			textures.push_back(texture);
			textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
		}
	}
	return textures;
}

void Model::loadMap(string const &path) {
	ifstream inFile(path);
	if (!inFile) {
		std::cout << "Map failed to load at path: " << path << std::endl;
		return;
	}
	int id[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	vector<Vertex> vertices[7];
	vector<unsigned int> indices[7];
	vector<Texture> textures[7];
	float textureWidth[7] = { 650, 357, 640, 447, 429, 980, 640 };
	float textureHeight[7] = { 613, 357, 480, 783, 729, 653, 490 };
	string textureNames[7] = { "wall_v_1.png", "wall_v_2.png", "wall_w_2.jpg", "blue_portal.png", "orange_portal.png", "ceil_2.jpg", "pillar_1.png" };
	while (true) {
		if (inFile.eof()) {
			break;
		}
		char type;
		inFile >> type;
		if (type == 'R') {
			vector<Vertex> vertices_;
			vector<unsigned int> indices_;
			vector<Texture> textures_;
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
			int order[] = { 0, 1, 2, 0, 2, 3 };
			for (int i = 0; i < 4; ++i) {
				Vertex v;
				float texCorWidth = 300.0f * (textureId == 1 ? 1.0f : 1.0f) * sqrt(pow(p[1][0] - p[0][0], 2) + pow(p[1][1] - p[0][1], 2) + pow(p[1][2] - p[0][2], 2));
				float texCorHeight = 300.0f * (textureId == 1 ? 1.0f : 1.0f) * sqrt(pow(p[1][0] - p[2][0], 2) + pow(p[1][1] - p[2][1], 2) + pow(p[1][2] - p[2][2], 2));
				float texX = texCorWidth / textureWidth[textureId - 1];
				float texY = texCorHeight / textureHeight[textureId - 1];
				switch (i) {
				case 0:
					v.TexCoords = glm::vec2(0.0f, 0.0f);
					break;
				case 1:
					v.TexCoords = glm::vec2(texX, 0.0f);
					break;
				case 2:
					v.TexCoords = glm::vec2(texX, texY);
					break;
				case 3:
					v.TexCoords = glm::vec2(0.0f, texY);
					break;
				default:
					break;
				}
				glm::vec3 vector;
				vector.x = p[i][0];
				vector.y = p[i][1];
				vector.z = p[i][2];
				v.Position = vector;
				vector.x = n[0];
				vector.y = n[1];
				vector.z = n[2];
				v.Normal = vector;
				vertices[textureId - 1].push_back(v);
				vertices_.push_back(v);
			}
			for (int i = 0; i < 6; ++i) {
				indices[textureId - 1].push_back(id[textureId - 1] + order[i]);
				indices_.push_back(order[i]);
			}
			Texture tmpTexture;
			tmpTexture.id = TextureFromFile(textureNames[textureId - 1].c_str(), "Textures/");
			tmpTexture.path = textureNames[textureId - 1].c_str();
			tmpTexture.type = "texture_diffuse";
			textures_.push_back(tmpTexture);
			Mesh m(vertices_, indices_, textures_);
			singleMeshes.push_back(m);
			id[textureId - 1] += 4;
		}
		else {
			break;
		}
	}
	Texture tmpTexture;
	for (int i = 0; i < 7; ++i) {
		tmpTexture.id = TextureFromFile(textureNames[i].c_str(), "Textures/");
		tmpTexture.path = textureNames[i].c_str();
		tmpTexture.type = "texture_diffuse";
		textures[i].push_back(tmpTexture);
	}
	inFile.close();
	for (int i = 0; i < 7; ++i) {
		Mesh mesh(vertices[i], indices[i], textures[i]);
		meshes.push_back(mesh);
	}
}
