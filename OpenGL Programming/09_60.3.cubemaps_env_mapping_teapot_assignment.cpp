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

#include "teapot_loader.h"

#include <iostream>
using namespace std;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
// 구 만들기
void initSphere(float** vertices, int* nVert, int* nAttr);
unsigned int loadTexture(const char* path);
unsigned int loadCubemap(vector<std::string> faces);

// settings
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 1200;

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
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "2091184 Seungeon Lee", NULL, NULL);
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
	Shader teapotShader("60.3.teapot.vs", "60.3.teapot.fs");
	Shader skyboxShader("60.1.skybox.vs", "60.1.skybox.fs");

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// 이 skybox vertices를 VBO로 gpu한테 줌
	// 원점에 큐브 배치해야 되니까 적절히 배치 
	// ------------------------------------------------------------------
	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	// load teapot data 
	std::vector <float> data;
	// teapot.vbo에 티팟의 정점 정보 들어가있음
	Teapot teapot("teapot.vbo", data, 8);
	// teapot VAO and VBO
	unsigned int teapotVBO, teapotVAO;
	glGenVertexArrays(1, &teapotVAO);
	glGenBuffers(1, &teapotVBO);
	glBindBuffer(GL_ARRAY_BUFFER, teapotVBO);
	// gpu한테 
	glBufferData(GL_ARRAY_BUFFER, teapot.nVertNum * teapot.nVertFloats * sizeof(float), &data[0], GL_STATIC_DRAW);
	glBindVertexArray(teapotVAO);
	// teapot 데이터에서 가져온 숫자들을 규칙에 맞게 쪼개기. 앞 3개는 position, float 2개는 텍스쳐 좌표, 그 다음 float 3개는 노말 이런 식으로.
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, teapot.nVertFloats * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, teapot.nVertFloats * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, teapot.nVertFloats * sizeof(float), (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// 구 설정 
	float* sphereVerts = nullptr;
	int nSphereVert, nSphereAttr;
	initSphere(&sphereVerts, &nSphereVert, &nSphereAttr);

	unsigned int sphereVBO, sphereVAO;
	glGenVertexArrays(1, &sphereVAO);
	glGenBuffers(1, &sphereVBO);

	// VBO에 구 정점 올리기
	glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
	glBufferData(GL_ARRAY_BUFFER, nSphereVert * nSphereAttr * sizeof(float), sphereVerts, GL_STATIC_DRAW);

	// VAO 설정. teapotShader의 attribute 레이아웃 맞춰주기
	glBindVertexArray(sphereVAO);
	// position > location 0
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,	nSphereAttr * sizeof(float),(void*)0);
	glEnableVertexAttribArray(0);
	// normal > location 2
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE,	nSphereAttr * sizeof(float),(void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	// texcoord > location 1
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,	nSphereAttr * sizeof(float),(void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// CPU 메모리 해제
	free(sphereVerts);


	// skybox VAO
	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	// load textures
	// -------------
	vector<std::string> faces
	{
		"..\\textures\\skybox\\umbrella\\px_right.png",
		"..\\textures\\skybox\\umbrella\\nx_left.png",
		"..\\textures\\skybox\\umbrella\\py_top.png",
		"..\\textures\\skybox\\umbrella\\ny_bottom.png",
		"..\\textures\\skybox\\umbrella\\pz_front.png",
		"..\\textures\\skybox\\umbrella\\nz_back.png"
	};
	unsigned int cubemapTexture = loadCubemap(faces);

	// shader configuration
	// --------------------
	teapotShader.use();
	teapotShader.setInt("skybox", 0);

	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);

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
		// teapot 그리기
		teapotShader.use();
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		teapotShader.setMat4("model", model);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		//teapotShader.setMat4("model", model);
		teapotShader.setMat4("view", view);
		teapotShader.setMat4("projection", projection);
		teapotShader.setVec3("eyePos", camera.Position);
		// draw the teapot object
		glBindVertexArray(teapotVAO);
		glActiveTexture(GL_TEXTURE0); // 티팟 그릴 때 0번 텍스쳐 그린다고 설정
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);  // 0번 텍스쳐에 큐브맵 텍스쳐 연결
		glDrawArrays(GL_TRIANGLES, 0, teapot.nVertNum); // 삼각형으로 티팟 그리기
		glBindVertexArray(0);

		// 구 그리기
		teapotShader.use(); // 이미 위에서 티팟 그리느라 use 돼있긴 함
		teapotShader.setMat4("view", view);
		teapotShader.setMat4("projection", projection);
		teapotShader.setVec3("eyePos", camera.Position);

		// 큐브맵 텍스쳐 0번에 또 바인딩 (티팟과 동일 재질)
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

		// sphere 1 
		glm::mat4 sphereModel = glm::mat4(1.0f);
		sphereModel = glm::translate(sphereModel, glm::vec3(1.5f, -0.3f, 1.0f)); // 위치
		sphereModel = glm::scale(sphereModel, glm::vec3(0.3f));                  // 크기 줄이기
		teapotShader.setMat4("model", sphereModel);

		glBindVertexArray(sphereVAO);
		glDrawArrays(GL_TRIANGLES, 0, nSphereVert);

		// sphere 2 
		sphereModel = glm::mat4(1.0f);
		sphereModel = glm::translate(sphereModel, glm::vec3(-2.5f, 1.3f, 0.0f));
		sphereModel = glm::scale(sphereModel, glm::vec3(0.7f));
		teapotShader.setMat4("model", sphereModel);

		glDrawArrays(GL_TRIANGLES, 0, nSphereVert);

		// sphere 3
		sphereModel = glm::mat4(1.0f);
		sphereModel = glm::translate(sphereModel, glm::vec3(4.5f, -1.4f, -2.0f));
		sphereModel = glm::scale(sphereModel, glm::vec3(0.9f));
		teapotShader.setMat4("model", sphereModel);
		glDrawArrays(GL_TRIANGLES, 0, nSphereVert);

		// VAO 해제
		glBindVertexArray(0);

		// draw skybox as last 스카이박스 그리기 
		// glDepthFunc는 depth test 하는 방식 설명. z buffer가 처음에 1로 세팅되고, 값이 있으면 0~1미만으로 설정됨. 그래서 원래는 GL_LESS가 기본값이라 1이면 안 그렸었음
		// 근데 GL_LEQUAL은 Less Equal의 줄임말, 1이여도 그리란 뜻. skybox그릴땐 depth test에서 z buffer 값 1이여도 그려라.
		// 큐브 배경의 z값은 skybox.vs에서 언제나 1로 만들어서 가져온다. 큐브 배경의 실제 크기는 매우 작지만 이 쉐이더에서 임의로 z값 조작했음
		// 1 안 그리는 상태면 언제나 z값이 1로 오는 배경은 절대 그려지지 않을 거임
		glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
		skyboxShader.use();
		view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
		// 큐브맵은 이미 원점에 맞춰서 배치했기 때문에 model matrix는 없다. 어디 옮길 필요 없으니까
		skyboxShader.setMat4("view", view);
		skyboxShader.setMat4("projection", projection);
		// skybox cube
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS); // 이게 기본값 depth test임. skybox 다 그렸으니 다시 이걸로 돌아오기. 원래는 이게 기본값으로 있었어서 다른 set depth function back to default

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// 구 관련도 추가함
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &teapotVAO);
	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteVertexArrays(1, &sphereVAO); 
	glDeleteBuffers(1, &teapotVBO);
	glDeleteBuffers(1, &skyboxVBO);
	glDeleteBuffers(1, &sphereVBO);

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

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
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

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
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

// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front) 
// -Z (back)
// -------------------------------------------------------
unsigned int loadCubemap(vector<std::string> faces)
{
	// 사용할 텍스쳐 만들기
	unsigned int textureID;
	glGenTextures(1, &textureID); // 생성
	// 텍스쳐를 조작할 거면 바인딩을 해놔야 함. 다 쓰고 나서 끊는 작업도 해야되는데 다른 textureID 넣으면 끊김
	// 이전과 다르게 타겟 텍스쳐가 2d가 아니라 CUBE_MAP이다 
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		//  인자의 내용은 std image loader가 알아서 이미지로부터 읽어옴
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			// enum이기 때문에 +i하면 POSTIVE_X > POSITIVE_Y 뭐 이런 식으로 쭉 돌아가면서 들어옴 6개 다
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // 텍스쳐 사이즈 넘어가버리면 테두리에 있는 색 늘려서 써라(clamp to edge)
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

// 행성계 구 코드
// u는 가로로 도는 거, 경도. v는 위도, 위로 도는 거.
// 극좌표계로 u 0에서 2파이까지 돌리면서 원 만들고, v는 0~180도 혹은 -90 ~ 90도 사이 돌리면서 위 아래로 원 만듦.
void initSphere(float** vertices, int* nVert, int* nAttr)
{
	//----------------------------------------
	// sphere: set up vertex data and configure vertex attributes
	float pi = acosf(-1.0f);	// pi = 3.14152... 
	float pi2 = 2.0f * pi;
	int nu = 40, nv = 20;
	const double du = pi2 / nu;
	const double dv = pi / nv;

	*nVert = (nv - 1) * nu * 6;		// two triangles
	*nAttr = 8;
	*vertices = (float*)malloc(sizeof(float) * (*nVert) * (*nAttr));

	float u, v;
	int k = 0;

	v = 0.0f;
	u = 0.0f;
	for (v = (-0.5f) * pi + dv; v < 0.5f * pi - dv; v += dv)
	{
		for (u = 0.0f; u < pi2; u += du)
		{
			// 정점 하나당 attribute는 pos 3, normal 3, tex coords 2 해서 float 8개다.
			// 위치와 노말이 같은 이유 > 원에서 점의 위치 = 중앙에서 그 점까지 가는 벡터(노말)이기 때문
			// p(u,v)
			(*vertices)[k++] = cosf(v) * cosf(u); 	(*vertices)[k++] = cosf(v) * sinf(u);	(*vertices)[k++] = sinf(v); 	// position 
			(*vertices)[k++] = cosf(v) * cosf(u);	(*vertices)[k++] = cosf(v) * sinf(u);	(*vertices)[k++] = sinf(v);		// normal
			(*vertices)[k++] = u / pi2;				(*vertices)[k++] = (v + 0.5f * pi) / pi;	// texture coords

			// p(u+du,v)
			(*vertices)[k++] = cosf(v) * cosf(u + du);	(*vertices)[k++] = cosf(v) * sinf(u + du);	(*vertices)[k++] = sinf(v); 	// position
			(*vertices)[k++] = cosf(v) * cosf(u + du);	(*vertices)[k++] = cosf(v) * sinf(u + du);	(*vertices)[k++] = sinf(v);		// normal
			(*vertices)[k++] = (u + du) / pi2;			(*vertices)[k++] = (v + 0.5f * pi) / pi; // texture coords

			// p(u,v+dv)
			(*vertices)[k++] = cosf(v + dv) * cosf(u);	(*vertices)[k++] = cosf(v + dv) * sinf(u);	(*vertices)[k++] = sinf(v + dv);	// position
			(*vertices)[k++] = cosf(v + dv) * cosf(u);	(*vertices)[k++] = cosf(v + dv) * sinf(u);	(*vertices)[k++] = sinf(v + dv);	// normal
			(*vertices)[k++] = u / pi2;					(*vertices)[k++] = (v + dv + 0.5f * pi) / pi; // texture coords

			// p(u+du,v)
			(*vertices)[k++] = cosf(v) * cosf(u + du);	(*vertices)[k++] = cosf(v) * sinf(u + du);	(*vertices)[k++] = sinf(v); 	// position
			(*vertices)[k++] = cosf(v) * cosf(u + du);	(*vertices)[k++] = cosf(v) * sinf(u + du);	(*vertices)[k++] = sinf(v);		// normal
			(*vertices)[k++] = (u + du) / pi2;			(*vertices)[k++] = (v + 0.5f * pi) / pi; // texture coords

			// p(u+du,v+dv)
			(*vertices)[k++] = cosf(v + dv) * cosf(u + du);	(*vertices)[k++] = cosf(v + dv) * sinf(u + du);	(*vertices)[k++] = sinf(v + dv); 	// position
			(*vertices)[k++] = cosf(v + dv) * cosf(u + du);	(*vertices)[k++] = cosf(v + dv) * sinf(u + du);	(*vertices)[k++] = sinf(v + dv);	// normal
			(*vertices)[k++] = (u + du) / pi2;				(*vertices)[k++] = (v + dv + 0.5f * pi) / pi;  // texture coords

			// p(u,v+dv)
			(*vertices)[k++] = cosf(v + dv) * cosf(u);	(*vertices)[k++] = cosf(v + dv) * sinf(u);	(*vertices)[k++] = sinf(v + dv);	// position
			(*vertices)[k++] = cosf(v + dv) * cosf(u);	(*vertices)[k++] = cosf(v + dv) * sinf(u);	(*vertices)[k++] = sinf(v + dv);	// normal
			(*vertices)[k++] = u / pi2;					(*vertices)[k++] = (v + dv + 0.5f * pi) / pi; // texture coords
		}
	}
	// triangles around north pole and south pole
	for (u = 0.0f; u < pi2; u += du)
	{
		// triangles around north pole 
		// p(u,pi/2-dv)
		v = 0.5f * pi - dv;
		(*vertices)[k++] = cosf(v) * cosf(u); 	(*vertices)[k++] = cosf(v) * sinf(u);	(*vertices)[k++] = sinf(v); 	// position 
		(*vertices)[k++] = cosf(v) * cosf(u);	(*vertices)[k++] = cosf(v) * sinf(u);	(*vertices)[k++] = sinf(v);		// normal
		(*vertices)[k++] = u / pi2;				(*vertices)[k++] = (v + 0.5f * pi) / pi;	// texture coords

		// p(u+du,pi/2-dv)
		v = 0.5f * pi - dv;
		(*vertices)[k++] = cosf(v) * cosf(u + du);	(*vertices)[k++] = cosf(v) * sinf(u + du); (*vertices)[k++] = sinf(v); 	// position
		(*vertices)[k++] = cosf(v) * cosf(u + du);	(*vertices)[k++] = cosf(v) * sinf(u + du); (*vertices)[k++] = sinf(v);		// normal
		(*vertices)[k++] = (u + du) / pi2;			(*vertices)[k++] = (v + 0.5f * pi) / pi; // texture coords

		// p(u,pi/2) = (0, 1. 0)  ~ north pole
		v = 0.5f * pi;
		(*vertices)[k++] = cosf(v) * cosf(u + du);	(*vertices)[k++] = cosf(v) * sinf(u + du);	(*vertices)[k++] = sinf(v); 	 // position
		(*vertices)[k++] = cosf(v) * cosf(u + du);	(*vertices)[k++] = cosf(v) * sinf(u + du);	(*vertices)[k++] = sinf(v);		 // normal
		(*vertices)[k++] = (u + du) / pi2;			(*vertices)[k++] = 1.0f;  // texture coords

		// triangles around south pole
		// p(u,-pi/2) = (0, -1, 0)  ~ south pole
		v = (-0.5f) * pi;
		(*vertices)[k++] = cosf(v) * cosf(u); 	(*vertices)[k++] = cosf(v) * sinf(u);	(*vertices)[k++] = sinf(v); 		// position
		(*vertices)[k++] = cosf(v) * cosf(u);	(*vertices)[k++] = cosf(v) * sinf(u);	(*vertices)[k++] = sinf(v);			// normal
		(*vertices)[k++] = u / pi2;				(*vertices)[k++] = 0.0f; // texture coords

		// p(u+du,-pi/2+dv)
		v = (-0.5f) * pi + dv;
		(*vertices)[k++] = cosf(v) * cosf(u + du);	(*vertices)[k++] = cosf(v) * sinf(u + du); (*vertices)[k++] = sinf(v);	// position
		(*vertices)[k++] = cosf(v) * cosf(u + du);	(*vertices)[k++] = cosf(v) * sinf(u + du); (*vertices)[k++] = sinf(v);	// normal
		(*vertices)[k++] = (u + du) / pi2;				(*vertices)[k++] = (v + 0.5f * pi) / pi; // texture coords

		// p(u,-pi/2+dv)
		(*vertices)[k++] = cosf(v) * cosf(u);	(*vertices)[k++] = cosf(v) * sinf(u); (*vertices)[k++] = sinf(v);	// position
		(*vertices)[k++] = cosf(v) * cosf(u);	(*vertices)[k++] = cosf(v) * sinf(u); (*vertices)[k++] = sinf(v);	// normal
		(*vertices)[k++] = u / pi2;					(*vertices)[k++] = (v + 0.5f * pi) / pi; // texture coords
	}
}