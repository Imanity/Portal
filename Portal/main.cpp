#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "Physics.h"
#include "Portal.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void processInput(GLFWwindow *window);
void glInitialize();

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(8.0f, 8.0f, 2.0f), glm::vec3(0.0f, 0.0f, 1.0f));

bool firstMouse = true;

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// For Physics Engine
// double verticleSpeed = 0;
glm::vec3 speed = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 keyboardSpeed = glm::vec3(0.0f, 0.0f, 0.0f);
bool isJumping = false;
Physics physics("Map2.txt", glm::vec3(0.0f, 0.0f, 1.0f));
glm::vec3 playerSize = glm::vec3(0.0f, 0.0f, 2.0f);
glm::vec3 playerPos, cameraPos;

// Portal
Portal portal;

int main() {
	glInitialize();

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Portal", NULL, NULL);
	if (window == NULL) 	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) 	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	Shader shader("shader.vs", "shader.fs");
	Shader shaderCross("shader_cross.vs", "shader_cross.fs");
	Shader shaderPortal("shader_portal.vs", "shader_portal.fs");

	Model scene("Map2.txt");
	Model crossHairs("Map_cross.txt");
	
	portal.initialize();

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window)) {
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Update state
		// ------
		playerPos = camera.Position - playerSize;
		physics.updateVerticleState(speed, playerPos, deltaTime, isJumping);
		camera.Position = playerSize + playerPos;
		
		cameraPos = camera.Position;
		bool isPass = false;
		glm::vec3 cameraFront = camera.Front;
		float rotateAngle = portal.passPortal(cameraPos, speed, keyboardSpeed, cameraFront, deltaTime, isPass);
		camera.ProcessMouseMovement(rotateAngle * 10.0f * 180.0f / 3.1416f, 0.0f);
		camera.Position = cameraPos;

		// input
		// -----
		keyboardSpeed = glm::vec3(0.0f, 0.0f, 0.0f);
		if (!isPass)
			processInput(window);

		// render
		// ------
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 model;

		// render the model
		shader.use();
		shader.setMat4("projection", projection);
		shader.setMat4("view", view);
		shader.setMat4("model", model);
		scene.Draw(shader);

		// draw cross
		shaderCross.use();
		crossHairs.Draw(shaderCross);

		// draw portal
		shaderPortal.use();
		shaderPortal.setMat4("projection", projection);
		shaderPortal.setMat4("view", view);
		shaderPortal.setMat4("model", model);
		portal.Draw(shaderPortal);

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	keyboardSpeed = glm::vec3(0.0f, 0.0f, 0.0f);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		glm::vec3 movement = glm::normalize(camera.Front - camera.WorldUp * (camera.Front * camera.WorldUp)) * camera.MovementSpeed * deltaTime * 35.0f;
		if (physics.isHorizontalAvailable(camera.Position, movement)) {
			camera.ProcessKeyboard(FORWARD, deltaTime);
			keyboardSpeed += movement;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		glm::vec3 movement = glm::normalize(camera.Front - camera.WorldUp * (camera.Front * camera.WorldUp)) * camera.MovementSpeed * deltaTime * -35.0f;
		if (physics.isHorizontalAvailable(camera.Position, movement)) {
			camera.ProcessKeyboard(BACKWARD, deltaTime);
			keyboardSpeed += movement;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		glm::vec3 movement = glm::normalize(camera.Right - camera.WorldUp * (camera.Right * camera.WorldUp)) * camera.MovementSpeed * deltaTime * -35.0f;
		if (physics.isHorizontalAvailable(camera.Position, movement)) {
			camera.ProcessKeyboard(LEFT, deltaTime);
			keyboardSpeed += movement;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		glm::vec3 movement = glm::normalize(camera.Right - camera.WorldUp * (camera.Right * camera.WorldUp)) * camera.MovementSpeed * deltaTime * 35.0f;
		if (physics.isHorizontalAvailable(camera.Position, movement)) {
			camera.ProcessKeyboard(RIGHT, deltaTime);
			keyboardSpeed += movement;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !isJumping) {
		speed.z = -8.0f;
		isJumping = true;
	}
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse) 	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse clicked, this callback is called
// -------------------------------------------------------
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (action == GLFW_PRESS) {
		int whichButton = (button == GLFW_MOUSE_BUTTON_RIGHT);
		bool isIntersected;
		glm::vec3 pos, n, up;
		isIntersected = physics.isIntersected(camera.Position, camera.Front, pos, n, up);
		// portal.setPortal(whichButton, glm::vec3(15.0f, 9.0f, 7.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		if (isIntersected) {
			portal.setPortal(whichButton, pos, n, up);
		}
	}
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

void glInitialize() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}
