#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
//#include <learnopengl/model.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);
void renderCube();

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "2091184 Seungeon Lee 이승언", NULL, NULL);
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

	// build and compile shaders
	// -------------------------
	Shader shader("50.4.normal_mapping.vs", "50.4.normal_mapping.fs");

	// load textures 새 텍스쳐 가져오면 여기서 바꾸기
	// -------------
	unsigned int diffuseMap = loadTexture("..\\textures\\A23DTEX_Albedo.jpg");
	unsigned int normalMap = loadTexture("..\\textures\\A23DTEX_Normal.jpg");

	// shader configuration
	// --------------------
	shader.use();
	shader.setInt("diffuseMap", 0);
	shader.setInt("normalMap", 1);

	// lighting info 월드 공간 기준 빛 위치
	// -------------
	glm::vec3 lightPos(1.5f, 2.0f, 1.3f);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic 애니메이션 위해서 시간 구하는 부분
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

		// 쉐이더한테 유니폼 변수 던져줌
		// configure view/projection matrices
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		shader.use();
		shader.setMat4("projection", projection);
		shader.setMat4("view", view);
		// render normal-mapped quad
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians((float)glfwGetTime() * -10.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0))); // rotate the quad to show normal mapping from multiple directions
		shader.setMat4("model", model);
		shader.setVec3("viewPos", camera.Position);
		shader.setVec3("lightPos", lightPos);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseMap);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, normalMap);

		// 이전 코드들과 다른 차이점 
		renderCube();

		// render light source (simply re-renders a smaller plane at the light's position for debugging/visualization) 빛 위치에 검은색 박스로 표시
		model = glm::mat4(1.0f);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.1f));
		shader.setMat4("model", model);
		renderCube();

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

// renders a 1x1 quad in NDC with manually calculated tangent vectors
// ------------------------------------------------------------------
// cube용 VAO/VBO
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;

// 각 정점: position(3) + normal(3) + texcoord(2) + tangent(3) + bitangent(3) = 14 floats
void renderCube()
{
	if (cubeVAO == 0)
	{
		float cubeVertices[] = {
			// front face (z+)
			// positions           // normals         // texcoords  // tangents           // bitangents
			-1.0f,  1.0f,  1.0f,   0.0f,  0.0f,  1.0f,   0.0f,  1.0f,   2.0f,  0.0f,  0.0f,   0.0f,  2.0f,  0.0f,
			-1.0f, -1.0f,  1.0f,   0.0f,  0.0f,  1.0f,   0.0f,  0.0f,   2.0f,  0.0f,  0.0f,   0.0f,  2.0f,  0.0f,
			 1.0f, -1.0f,  1.0f,   0.0f,  0.0f,  1.0f,   1.0f,  0.0f,   2.0f,  0.0f,  0.0f,   0.0f,  2.0f,  0.0f,
			-1.0f,  1.0f,  1.0f,   0.0f,  0.0f,  1.0f,   0.0f,  1.0f,   2.0f,  0.0f,  0.0f,   0.0f,  2.0f,  0.0f,
			 1.0f, -1.0f,  1.0f,   0.0f,  0.0f,  1.0f,   1.0f,  0.0f,   2.0f,  0.0f,  0.0f,   0.0f,  2.0f,  0.0f,
			 1.0f,  1.0f,  1.0f,   0.0f,  0.0f,  1.0f,   1.0f,  1.0f,   2.0f,  0.0f,  0.0f,   0.0f,  2.0f,  0.0f,

			 // back face (z-)
			  1.0f,  1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   0.0f,  1.0f,  -2.0f,  0.0f,  0.0f,   0.0f,  2.0f,  0.0f,
			  1.0f, -1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   0.0f,  0.0f,  -2.0f,  0.0f,  0.0f,   0.0f,  2.0f,  0.0f,
			 -1.0f, -1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   1.0f,  0.0f,  -2.0f,  0.0f,  0.0f,   0.0f,  2.0f,  0.0f,
			  1.0f,  1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   0.0f,  1.0f,  -2.0f,  0.0f,  0.0f,   0.0f,  2.0f,  0.0f,
			 -1.0f, -1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   1.0f,  0.0f,  -2.0f,  0.0f,  0.0f,   0.0f,  2.0f,  0.0f,
			 -1.0f,  1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   1.0f,  1.0f,  -2.0f,  0.0f,  0.0f,   0.0f,  2.0f,  0.0f,

			 // left face (x-)
			 -1.0f,  1.0f, -1.0f,  -1.0f,  0.0f,  0.0f,   0.0f,  1.0f,   0.0f,  0.0f,  2.0f,   0.0f,  2.0f,  0.0f,
			 -1.0f, -1.0f, -1.0f,  -1.0f,  0.0f,  0.0f,   0.0f,  0.0f,   0.0f,  0.0f,  2.0f,   0.0f,  2.0f,  0.0f,
			 -1.0f, -1.0f,  1.0f,  -1.0f,  0.0f,  0.0f,   1.0f,  0.0f,   0.0f,  0.0f,  2.0f,   0.0f,  2.0f,  0.0f,
			 -1.0f,  1.0f, -1.0f,  -1.0f,  0.0f,  0.0f,   0.0f,  1.0f,   0.0f,  0.0f,  2.0f,   0.0f,  2.0f,  0.0f,
			 -1.0f, -1.0f,  1.0f,  -1.0f,  0.0f,  0.0f,   1.0f,  0.0f,   0.0f,  0.0f,  2.0f,   0.0f,  2.0f,  0.0f,
			 -1.0f,  1.0f,  1.0f,  -1.0f,  0.0f,  0.0f,   1.0f,  1.0f,   0.0f,  0.0f,  2.0f,   0.0f,  2.0f,  0.0f,

			 // right face (x+)
			  1.0f,  1.0f,  1.0f,   1.0f,  0.0f,  0.0f,   0.0f,  1.0f,   0.0f,  0.0f, -2.0f,   0.0f,  2.0f,  0.0f,
			  1.0f, -1.0f,  1.0f,   1.0f,  0.0f,  0.0f,   0.0f,  0.0f,   0.0f,  0.0f, -2.0f,   0.0f,  2.0f,  0.0f,
			  1.0f, -1.0f, -1.0f,   1.0f,  0.0f,  0.0f,   1.0f,  0.0f,   0.0f,  0.0f, -2.0f,   0.0f,  2.0f,  0.0f,
			  1.0f,  1.0f,  1.0f,   1.0f,  0.0f,  0.0f,   0.0f,  1.0f,   0.0f,  0.0f, -2.0f,   0.0f,  2.0f,  0.0f,
			  1.0f, -1.0f, -1.0f,   1.0f,  0.0f,  0.0f,   1.0f,  0.0f,   0.0f,  0.0f, -2.0f,   0.0f,  2.0f,  0.0f,
			  1.0f,  1.0f, -1.0f,   1.0f,  0.0f,  0.0f,   1.0f,  1.0f,   0.0f,  0.0f, -2.0f,   0.0f,  2.0f,  0.0f,

			  // top face (y+)
			  -1.0f,  1.0f, -1.0f,   0.0f,  1.0f,  0.0f,   0.0f,  1.0f,   2.0f,  0.0f,  0.0f,   0.0f,  0.0f, -2.0f,
			  -1.0f,  1.0f,  1.0f,   0.0f,  1.0f,  0.0f,   0.0f,  0.0f,   2.0f,  0.0f,  0.0f,   0.0f,  0.0f, -2.0f,
			   1.0f,  1.0f,  1.0f,   0.0f,  1.0f,  0.0f,   1.0f,  0.0f,   2.0f,  0.0f,  0.0f,   0.0f,  0.0f, -2.0f,
			  -1.0f,  1.0f, -1.0f,   0.0f,  1.0f,  0.0f,   0.0f,  1.0f,   2.0f,  0.0f,  0.0f,   0.0f,  0.0f, -2.0f,
			   1.0f,  1.0f,  1.0f,   0.0f,  1.0f,  0.0f,   1.0f,  0.0f,   2.0f,  0.0f,  0.0f,   0.0f,  0.0f, -2.0f,
			   1.0f,  1.0f, -1.0f,   0.0f,  1.0f,  0.0f,   1.0f,  1.0f,   2.0f,  0.0f,  0.0f,   0.0f,  0.0f, -2.0f,

			   // bottom face (y-)
			   -1.0f, -1.0f,  1.0f,   0.0f, -1.0f,  0.0f,   0.0f,  1.0f,   2.0f,  0.0f,  0.0f,   0.0f,  0.0f,  2.0f,
			   -1.0f, -1.0f, -1.0f,   0.0f, -1.0f,  0.0f,   0.0f,  0.0f,   2.0f,  0.0f,  0.0f,   0.0f,  0.0f,  2.0f,
				1.0f, -1.0f, -1.0f,   0.0f, -1.0f,  0.0f,   1.0f,  0.0f,   2.0f,  0.0f,  0.0f,   0.0f,  0.0f,  2.0f,
			   -1.0f, -1.0f,  1.0f,   0.0f, -1.0f,  0.0f,   0.0f,  1.0f,   2.0f,  0.0f,  0.0f,   0.0f,  0.0f,  2.0f,
				1.0f, -1.0f, -1.0f,   0.0f, -1.0f,  0.0f,   1.0f,  0.0f,   2.0f,  0.0f,  0.0f,   0.0f,  0.0f,  2.0f,
				1.0f, -1.0f,  1.0f,   0.0f, -1.0f,  0.0f,   1.0f,  1.0f,   2.0f,  0.0f,  0.0f,   0.0f,  0.0f,  2.0f
		};

		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		glBindVertexArray(cubeVAO);
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));

		glBindVertexArray(0);
	}

	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);   // 6면 * 2삼각형 * 3정점
	glBindVertexArray(0);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
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

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const * path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
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

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}