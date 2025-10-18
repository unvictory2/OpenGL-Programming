//////////////////////////////////////////
//		jieunlee@hansung.ac.kr			//
//		2022. 6. 1.						//
//////////////////////////////////////////
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/shader.h>
#include <learnopengl/camera.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void initSphere(float**, int*, int*);


// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 13.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

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
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "JIeun Lee @ HSU", NULL, NULL);
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
	// 어느 주석 푸느냐에 따라서 다른 쉐이더 적용됨
	//Shader sphereShader1("j.sphere.vs", "j.sphere.fs");
	//Shader sphereShader1("j.sphere.vs", "j.sphere_cartoon.fs");
	Shader sphereShader1("j.sphere.vs", "j.sphere_2lights.fs");

	// sphere VAO and VBO
	//std::vector <float> data;
	float* sphereVerts = NULL;
	int nSphereVert, nSphereAttr;
	initSphere(&sphereVerts, &nSphereVert, &nSphereAttr);

	unsigned int sphereVBO, sphereVAO;
	glGenVertexArrays(1, &sphereVAO);
	glGenBuffers(1, &sphereVBO);
	glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
	glBufferData(GL_ARRAY_BUFFER, nSphereVert * nSphereAttr * sizeof(float), sphereVerts, GL_STATIC_DRAW);

	glBindVertexArray(sphereVAO);
	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, nSphereAttr * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, nSphereAttr * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// texCoord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, nSphereAttr * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	free(sphereVerts);


	// uncomment this call to draw in wireframe polygons.
	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

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
		sphereShader1.use();
		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		sphereShader1.setMat4("projection", projection);
		sphereShader1.setMat4("view", view);
		// eye position
		sphereShader1.setVec3("eyePos", camera.Position);

		// light properties
		glm::vec3 lightPos1(1.5f, 1.0f, 2.0f);
		glm::vec3 lightColor(1.0, 1.0, 1.0);
		glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f); // decrease the influence 
		glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f); // low influenc
		sphereShader1.setVec3("light.ambient", ambientColor);
		sphereShader1.setVec3("light.diffuse", diffuseColor);
		sphereShader1.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
		sphereShader1.setVec3("light.position", lightPos1);
		// light2 properties
		glm::vec3 lightPos2(-1.5f, 1.0f, 2.0f);
		lightColor = glm::vec3(1.0, 1.0, 1.0);
		diffuseColor = lightColor * glm::vec3(0.3f); // decrease the influence 
		ambientColor = diffuseColor * glm::vec3(0.1f); // low influenc
		sphereShader1.setVec3("light2.ambient", ambientColor);
		sphereShader1.setVec3("light2.diffuse", diffuseColor);
		sphereShader1.setVec3("light2.specular", 1.0f, 0.0f, 0.0f);
		sphereShader1.setVec3("light2.position", lightPos2);

		// draw the sphere object 1
		// material properties
		sphereShader1.setVec3("material.ambient", 0.3f, 0.5f, 0.7f);
		sphereShader1.setVec3("material.diffuse", 0.3f, 0.5f, 0.9f);
		sphereShader1.setVec3("material.specular", 0.5f, 0.5f, 0.5f); // specular lighting doesn't have full effect on this object's material
		sphereShader1.setFloat("material.shininess", 20.0f);
		// world transformation
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		sphereShader1.setMat4("model", model);
		// render the sphere
		glBindVertexArray(sphereVAO);
		glDrawArrays(GL_TRIANGLES, 0, nSphereVert);

		// draw the sphere object 2
		// material properties
		sphereShader1.setVec3("material.ambient", 0.7f, 0.5f, 0.7f);
		sphereShader1.setVec3("material.diffuse", 0.7f, 0.5f, 0.7f);
		sphereShader1.setVec3("material.specular", 0.5f, 0.5f, 0.5f); // specular lighting doesn't have full effect on this object's material
		sphereShader1.setFloat("material.shininess", 2.0f);
		// world transformation
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(3.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		sphereShader1.setMat4("model", model);
		// render the sphere
		glBindVertexArray(sphereVAO);
		glDrawArrays(GL_TRIANGLES, 0, nSphereVert);

		// draw the sphere object 3
		// material properties
		sphereShader1.setVec3("material.ambient", 0.99f, 0.7f, 0.3f);
		sphereShader1.setVec3("material.diffuse", 0.99f, 0.7f, 0.3f);
		sphereShader1.setVec3("material.specular", 0.9f, 0.9f, 0.9f); // specular lighting doesn't have full effect on this object's material
		sphereShader1.setFloat("material.shininess", 100.0f);
		// world transformation
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-3.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		sphereShader1.setMat4("model", model);
		// render the sphere
		glBindVertexArray(sphereVAO);
		glDrawArrays(GL_TRIANGLES, 0, nSphereVert);

		// draw the sphere object 4
		// material properties
		sphereShader1.setVec3("material.ambient", 0.1f, 0.1f, 0.1f);
		sphereShader1.setVec3("material.diffuse", 0.1f, 0.1f, 0.1f);
		sphereShader1.setVec3("material.specular", 0.9f, 0.9f, 0.9f); // specular lighting doesn't have full effect on this object's material
		sphereShader1.setFloat("material.shininess", 100.0f);
		// world transformation
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-3.0f, 3.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		sphereShader1.setMat4("model", model);
		// render the sphere
		glBindVertexArray(sphereVAO);
		glDrawArrays(GL_TRIANGLES, 0, nSphereVert);

		// draw the sphere object 5
		// material properties
		sphereShader1.setVec3("material.ambient", 0.99f, 0.7f, 0.6f);
		sphereShader1.setVec3("material.diffuse", 0.99f, 0.7f, 0.6f);
		sphereShader1.setVec3("material.specular", 0.5f, 0.5f, 0.5f); // specular lighting doesn't have full effect on this object's material
		sphereShader1.setFloat("material.shininess", 3.0f);
		// world transformation
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 3.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		sphereShader1.setMat4("model", model);
		// render the sphere
		glBindVertexArray(sphereVAO);
		glDrawArrays(GL_TRIANGLES, 0, nSphereVert);

		// draw the sphere object 6
		// material properties
		sphereShader1.setVec3("material.ambient", 0.99f, 0.8f, 0.6f);
		sphereShader1.setVec3("material.diffuse", 0.99f, 0.8f, 0.6f);
		sphereShader1.setVec3("material.specular", 0.0f, 0.0f, 0.0f); // specular lighting doesn't have full effect on this object's material
		sphereShader1.setFloat("material.shininess", 1.0f);
		// world transformation
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(3.0f, 3.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		sphereShader1.setMat4("model", model);
		// render the sphere
		glBindVertexArray(sphereVAO);
		glDrawArrays(GL_TRIANGLES, 0, nSphereVert);

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &sphereVAO);
	glDeleteBuffers(1, &sphereVBO);

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

// initalize vertices of a sphere : position, normal, tex_coords. 
//void initSphere(std::vector <float> data, int* nVert, int* nAttr)
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