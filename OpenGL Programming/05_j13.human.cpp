//////////////////////////////////////////
//		jieunlee@hansung.ac.kr			//
//		2020. 10. 12					//
//////////////////////////////////////////

//세트: j13human.h, j13human.vs, j13human.fs
//헤더는 include폴더에, vs fs 쉐이더는 이 코드랑 같은 디렉토리에

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/shader.h>
#include <learnopengl/camera.h>

#include <iostream>

#include "j13.human.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 20.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting
glm::vec3 lightPos(1.2f, 5.0f, 12.0f);

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Jieun Lee", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// build and compile our shader zprogram
	// ------------------------------------
	Shader boneShader("j13.human.vs", "j13.human.fs");
	Shader lampShader("13.2.lamp.vs", "13.2.lamp.fs");

	// 육면체(네모) 그리기
	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float vertices[] = {
		-0.5f,  0.0f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f,  0.0f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f,  1.0f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f,  1.0f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f,  1.0f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f,  0.0f, -0.5f,  0.0f,  0.0f, -1.0f,

		-0.5f,  0.0f,  0.5f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.0f,  0.5f,  0.0f,  0.0f,  1.0f,
		 0.5f,  1.0f,  0.5f,  0.0f,  0.0f,  1.0f,
		 0.5f,  1.0f,  0.5f,  0.0f,  0.0f,  1.0f,
		-0.5f,  1.0f,  0.5f,  0.0f,  0.0f,  1.0f,
		-0.5f,  0.0f,  0.5f,  0.0f,  0.0f,  1.0f,

		-0.5f,  1.0f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  1.0f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.0f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.0f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.0f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  1.0f,  0.5f, -1.0f,  0.0f,  0.0f,

		 0.5f,  1.0f,  0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f,  1.0f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f,  0.0f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f,  0.0f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f,  0.0f,  0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f,  1.0f,  0.5f,  1.0f,  0.0f,  0.0f,

		-0.5f,  0.0f, -0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f,  0.0f, -0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f,  0.0f,  0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f,  0.0f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f,  0.0f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f,  0.0f, -0.5f,  0.0f, -1.0f,  0.0f,

		-0.5f,  1.0f, -0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  1.0f, -0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  1.0f,  0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  1.0f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  1.0f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  1.0f, -0.5f,  0.0f,  1.0f,  0.0f
	};
	// first, configure the cube's VAO (and VBO)
	unsigned int VBO, cubeVAO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindVertexArray(cubeVAO);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);


	// second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
	unsigned int lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// note that we update the lamp's position attribute's stride to reflect the updated buffer data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);


	// Human
	Human human;

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// be sure to activate shader when setting uniforms/drawing objects
		boneShader.use();
		boneShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
		boneShader.setVec3("lightPos", lightPos);
		boneShader.setVec3("viewPos", camera.Position);

		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		boneShader.setMat4("projection", projection);
		boneShader.setMat4("view", view);

		// world transformation. 월드에서 걷게 하는 과정 여기서 수정
		glm::mat4 model = glm::mat4(1.0f);
		static float s = 0.0f;
		s += deltaTime;
		//model = glm::translate(model, glm::vec3(0.0f, 0.0f, s));
		boneShader.setMat4("model", model);

		// render a human
		//human.SetBoneRotation(upperarmL, glm::angleAxis(glm::radians(30.f), glm::vec3(0.f, 0.f, 1.f)));
		//human.SetBoneRotation(forearmL, glm::angleAxis(glm::radians(60.f), glm::vec3(0.f, 0.f, 1.f)));
		//human.SetPose(armLeftUp);
		static float t = 0.0f;
		float dt = deltaTime;
		human.MixPose(base, armLeftUp, t); // 움직이는 과정
		human.DrawHuman(boneShader, cubeVAO, model);
		t = t + dt;
		if (t > 1.0f) t = 0.0f;

		// also draw the lamp object
		/*/
		lampShader.use();
		lampShader.setMat4("projection", projection);
		lampShader.setMat4("view", view);
		model = glm::mat4(1.0f);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.2f)); // a smaller cube
		lampShader.setMat4("model", model);

		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		*/

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteVertexArrays(1, &lightVAO);
	glDeleteBuffers(1, &VBO);

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
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

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}