#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

int main() {
	// 1. INIT
	glfwInit(); // initialize GLFW
	// tell GLFW that the OpenGL version is 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // configure GLFW (what we want to configure, integer that sets the value of our option)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// tell GLFW we will use core-profile explictily - get access to a smaller subset of OPENGL features without features we no longer need
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// 2. MAKE WINDOW
	GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL); // (width, height, name, ignore , for now)
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// 3. GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// 4. Tell OpenGL the size of the rendering window
	glViewport(0, 0, 800, 600); // (set the location of the, lower left corner of the window,, width, heigiht in pixels)

	// 창 크기 조정하면 콜백함수 불러서 openGL한테 크기 조정된 거 알리기
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// render loop - 프로그램 바로 종료 안 하고 유저 반응 대기
	while (!glfwWindowShouldClose(window)) {
		//input
		processInput(window);

		// rendering commands
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// check and call eevents and swap the buffers
		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {

}