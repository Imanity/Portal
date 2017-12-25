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
const unsigned int SCR_WIDTH = 1366;
const unsigned int SCR_HEIGHT = 768;

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
	Shader shaderPortalInside("shader_portal_inside.vs", "shader_portal_inside.fs");
	Shader shaderPortalMask("shader_portal_mask.vs", "shader_portal_mask.fs");

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
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 model;

		// render the model
		/*
		shader.use();
		shader.setMat4("projection", projection);
		shader.setMat4("view", view);
		shader.setMat4("model", model);
		scene.Draw(shader);*/

		// -----------------------------------------

		glClearStencil(0);
		glClear(GL_STENCIL_BUFFER_BIT);

		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glEnable(GL_STENCIL_TEST);

		glDepthMask(GL_FALSE);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

		shaderPortalMask.use();
		shaderPortalMask.setMat4("projection", projection);
		shaderPortalMask.setMat4("view", view);
		shaderPortalMask.setMat4("model", model);
		portal.Draw(shaderPortalMask);

		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);

		glStencilFunc(GL_EQUAL, 0, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

		shader.use();
		shader.setMat4("projection", projection);
		shader.setMat4("view", view);
		shader.setMat4("model", model);
		scene.Draw(shader);

		glDisable(GL_STENCIL_TEST);

		// ----------------------------------------

		// draw cross
		shaderCross.use();
		crossHairs.Draw(shaderCross);

		// draw portal
		shaderPortal.use();
		shaderPortal.setMat4("projection", projection);
		shaderPortal.setMat4("view", view);
		shaderPortal.setMat4("model", model);
		portal.Draw(shaderPortal);

		// draw scene inside portal
		if (portal.bluePortalExist && portal.orangePortalExist) {
			glm::vec3 Front = camera.Front;
			glm::vec3 Up = camera.Up;
			glm::vec3 pos = camera.Position;

			for (int i = 0; i < 2; ++i) {
				glm::vec3 PortalN, PortalPos, PortalUp, PortalRight, PortalN_, PortalPos_, PortalUp_, PortalRight_;
				if (i == 0) {
					PortalN = portal.bluePortalN;
					PortalPos = portal.bluePortalPos;
					PortalN_ = portal.orangePortalN;
					PortalPos_ = portal.orangePortalPos;
				}
				else {
					PortalN_ = portal.bluePortalN;
					PortalPos_ = portal.bluePortalPos;
					PortalN = portal.orangePortalN;
					PortalPos = portal.orangePortalPos;
				}

				PortalUp = glm::vec3(0.0f, 0.0f, 1.0f);
				PortalUp_ = glm::vec3(0.0f, 0.0f, 1.0f);
				PortalRight = glm::normalize(glm::cross(PortalUp, PortalN));
				PortalRight_ = glm::normalize(glm::cross(PortalUp_, PortalN_));

				glm::vec3 Front_ = PortalUp_ * glm::dot(Front, PortalUp) - PortalRight_ * glm::dot(Front, PortalRight) - PortalN_ *  glm::dot(Front, PortalN);
				glm::vec3 Up_ = PortalUp_ * glm::dot(Up, PortalUp) - PortalRight_ * glm::dot(Up, PortalRight) - PortalN_ *  glm::dot(Up, PortalN);
				Front_ = glm::normalize(Front_);
				Up_ = glm::normalize(Up_);
				if (glm::dot(Front, PortalN) == 0) {
					continue;
				}
				float t = glm::dot(PortalPos - pos, PortalN) / glm::dot(Front, PortalN);
				if (t <= 0.0f) {
					continue;
				}
				glm::vec3 pos1 = pos + Front * t;
				glm::vec3 pa = pos1 - PortalPos;
				glm::vec3 pb = PortalUp * glm::dot(pa, PortalUp) - PortalRight * glm::dot(pa, PortalRight);
				glm::vec3 pos2 = PortalPos_ + pb;
				glm::vec3 pos_ = pos2 - Front_ * t;

				glm::mat4 insideView = glm::lookAt(pos_, pos_ + Front_ * t, Up_);
				

				// Mask
				
				glClearStencil(0);
				glClear(GL_STENCIL_BUFFER_BIT);

				glStencilFunc(GL_ALWAYS, 1, 0xFF);
				glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
				glEnable(GL_STENCIL_TEST);

				glDepthMask(GL_FALSE);
				glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

				shaderPortalMask.use();
				shaderPortalMask.setMat4("projection", projection);
				shaderPortalMask.setMat4("view", view);
				shaderPortalMask.setMat4("model", model);
				portal.DrawSingle(shaderPortalMask, i);

				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
				glDepthMask(GL_TRUE);

				glStencilFunc(GL_EQUAL, 1, 0xFF);
				glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
				
				shaderPortalInside.use();
				shaderPortalInside.setMat4("projection", projection);
				shaderPortalInside.setMat4("view", insideView);
				shaderPortalInside.setMat4("model", model);
				// scene.Draw(shaderPortalInside);
				scene.DrawExcept(shaderPortalInside, PortalPos_, PortalN_);

				glDisable(GL_STENCIL_TEST);
			}
		}

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
