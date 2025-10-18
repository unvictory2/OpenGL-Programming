//////////////////////////////////////////
//		jieunlee@hansung.ac.kr			//
//		Solar system (Earth)			//
//		2023. 5. 11						//
//////////////////////////////////////////
// 행성 간 거리는 실제비율 반영보단 간편함을 목적으로 적당한 값을 줌
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION // 이 줄이랑 밑에 줄 2줄 위아래 바뀌면 안 됨. 인터넷 코드엔 이 줄 없는 경우도 있는데 넣어줘야 한다.
#include <stb_image.h> // 이미지 쓸 때 (텍스쳐 매핑할 때) 필요하다

#include <learnopengl/shader.h>
#include <learnopengl/camera.h>

//framebuffer size = 창 조절. 마우스로 창 테두리 잡아서 크기 바꾸는 거
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos); // 마우스 눌렀을 때 뭐 할 건지 (현재는 카메라 기능과 연결)
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset); // 마우스 스크롤 했을 때 뭐 할 건지 (현재 카메라 기능과 연결)
void processInput(GLFWwindow* window); // 키보드 눌렀을 때 뭐 할 건지
void init_sphere(float**, int*, int*); // 구 정점 계산
void init_textures(); // 텍스쳐 관련 초기화

// settings 창 크기
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 50.0f));
// 처음 마우스 위치. 이거 기준으로 마우스 얼마나 움직였는지로 회전시키고 함.
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f; // 한 프레임 그리고 다음 프레임 그리기 전까지의 시간.
float lastFrame = 0.0f;

// lighting
glm::vec3 lightPos(21.2f, 21.0f, 12.0f); // 빛이 있는 위치. (0,0,0)으로 해서 태양계 행성들이 태양 빛을 받게 해야되지만 이러면 태양 빛에 문제 발생.
// 아마 태양에는 다른 쉐이더 써서 해결하지 않나 싶다.

// the sun, planets, and moon
	// 자전주기(일): 태수금지(달) ... �¾� 25.38��, ���� 58.6462, �ݼ�	-243.0185, ����	0.99726963,	(�� 27.3216) ȭ���� 1.02595676, ���� 0.410��, �伺�� 0.426��, õ�ռ��� 0.718��, �ؿռ��� 0.669��
	// 공전주기(일): 수금지(달)화목 ... ���� 87.97, �ݼ� 224.7, ���� 365.26, (�� 27.322) ȭ�� 686.96, �� 4333.29, �伺 10,756.20,	 õ�ռ� 30,707.49, 	�ؿռ� 60,223.35

	// rotation periods of the sun, planets, and moon 자전주기
const float rotp_sun = 25.38f;
const float rotp_mercury = 58.6462f;
const float rotp_venus = 243.0185f;
const float rotp_earth = 0.99726963f;
const float rotp_moon = 27.3216f;
const float rotp_mars = 1.02595676f;
const float rotp_jupiter = 0.410f;
const float rotp_saturn = 0.426f;
const float rotp_uranus = 0.718f;
const float rotp_neptune = 0.669;

// revolution periods of the planets, and moon 공전주기
const float revp_mercury = 87.97f;
const float revp_venus = 224.7f;
const float revp_earth = 365.26f;
const float revp_moon = 27.322f;
const float revp_mars = 686.96f;
const float revp_jupiter = 4333.29f;
const float revp_saturn = 10756.20f;
const float revp_uranus = 30707.49f;
const float revp_neptune = 60223.35;

// speed of rotation and revolution
const float rot_speed = 20.0f;

// scales of the planets, and moon 행성들의 반지름
const float radi_sun = 6.0f; // 696340.0f;
const float radi_mercury = 0.24f;	//2439.7f;
const float radi_venus = 0.60f;	//6051.8f;
const float radi_earth = 0.63f;	//6371.0f;
const float radi_moon = 0.17f;	//1737.4f;
const float radi_mars = 0.3389f;	//3389.5f;
const float radi_jupiter = 1.0f; // 69911.0f;
const float radi_saturn = 0.9f; // 58232.0f;
const float radi_uranus = 0.43f; // 25362.0f;
const float radi_neptune = 0.4f; // 24622.0f;

// textures 텍스쳐 사용하려면 텍스쳐 변수 만들어놔야 한다.
unsigned int texture_sun, texture_mercury, texture_venus, texture_earth, texture_moon;
unsigned int texture_mars, texture_jupiter, texture_saturn, texture_uranus, texture_neptune;

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	// opengl 버전 알려줌. 3.3 core여서 알려줌.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

	// glfw window creation
	// --------------------
	// 윈도우 생성
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Seungeon Lee @ 2091184", NULL, NULL);
	if (window == NULL) // 윈도우 생성 실패시 에러 메시지
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback); // glfw에게 마우스 스크롤이 일어나면 scroll_callback 함수 찾아가라고 말함. scroll_callback은 밑에 만들어둠.

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// glad 로딩 실패하면 에러 메시지
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
	// 쉐이더 2개 씀. 행성 그릴 땐 vertex shader 뭐 쓰고 fragment shader 뭐 쓴다. 태양 그릴 땐 뭐 쓴다 라고 말해줌.
	// 쉐이더 연결은 learnoepgnl 저자가 만든 shader.h 가 알아서 해줌, 우리는 파일 명시만 해주면 됨
	// 지금은 starShader 부분 파일들이 planet이랑 똑같아서 태양이 이상해 보이는 것
	// ------------------------------------
	Shader planetShader("solarsystem_planet.vs", "solarsystem_planet.fs");
	//Shader planetShader("solarsystem_color.vs", "solarsystem_color.fs");
	Shader starShader("solarsystem_star.vs", "solarsystem_star.fs");

	// sphere VAO and VBO
	//std::vector <float> data;
	float* sphereVerts = NULL;
	int nSphereVert, nSphereAttr;
	// 교수님이 짠 부분. 정점이 만들어지고 정점의 개수 등도 만들어진다
	// Attr는 float 8개. pos 3, normal 3, texture coords 2개 
	init_sphere(&sphereVerts, &nSphereVert, &nSphereAttr);

	// VAO : Attribute로 온 정보들은 그냥 숫자 덩어리다. 뭐가 포지션이고 뭐가 노말인지 GPU가 이해 못 한다. VAO가 해석해준다.
	unsigned int sphereVBO, sphereVAO;
	glGenVertexArrays(1, &sphereVAO);
	glGenBuffers(1, &sphereVBO);
	glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
	glBufferData(GL_ARRAY_BUFFER, nSphereVert * nSphereAttr * sizeof(float), sphereVerts, GL_STATIC_DRAW);

	glBindVertexArray(sphereVAO);
	// position attribute
	// 정보를 해석할 수 있게 라벨링 해주는 거.  
	//         (void*)0부터 3개 읽어. 난 0번 특징이야. 그 다음부턴 8개 = nSphereAttr * sizeof(float) 건너뛰고 읽어.
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, nSphereAttr * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// normal attribute
	// 1번 attribute, 나도 float 3개야. 시작은 (void*)(3 * sizeof(float))
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, nSphereAttr * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// texCoord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, nSphereAttr * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	free(sphereVerts);


	// init textures
	init_textures();


	// uncomment this call to draw in wireframe polygons.
	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// render loop
	// 가장 중요!
	// -----------
	while (!glfwWindowShouldClose(window)) // 윈도우 꺼질 때까지 while 
	{
		// per-frame time logic 이 부분은 이해하려면 learnopengl갈것
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // rgba. 배경색. a=alpha, 투명값
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // frame buffer, depth buffer. 그래픽스 내용.


		// sun 	
		// -----------
		// be sure to activate shader when setting uniforms/drawing objects. star shader를 쓰겠다.
		starShader.use();
		// view/projection transformations 원근투영 평행투영. glm에 내 의도만 넣으면 알아서 해준다.
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
		// learnopengl 작가가 짠 함수
		glm::mat4 view = camera.GetViewMatrix();
		starShader.setMat4("projection", projection);
		starShader.setMat4("view", view);
		// eye position 눈 위치를 카메라 위치로 설정
		starShader.setVec3("eyePos", camera.Position);

		// phong 쉐이딩 관련 내용 - fragment 쉐이더에 정의돼있다!
		// draw the sphere object
		// light properties
		glm::vec3 lightColor(1.0, 1.0, 1.0);;
		glm::vec3 diffuseColor = lightColor * glm::vec3(0.8f); // decrease the influence 
		glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f); // low influenc
		// 쉐이더에서 이런 변수를 찾아서 이 값으로 설정해라. light.ambient 변수 찾아서 ambientColor로 설정
		starShader.setVec3("light.ambient", ambientColor);
		starShader.setVec3("light.diffuse", diffuseColor);
		starShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
		starShader.setVec3("light.position", lightPos);
		// material properties. light color는 빛이 가지고 있는 것, object color(밑에꺼)는 물체의 색상
		starShader.setVec3("material.ambient", 0.5f, 0.5f, 0.5f);
		starShader.setVec3("material.diffuse", 0.9f, 0.9f, 0.9f);
		starShader.setVec3("material.specular", 0.5f, 0.5f, 0.5f); // specular lighting doesn't have full effect on this object's material
		starShader.setFloat("material.shininess", 20.0f);


		// 곱하기는 역순으로 된다고 이해하면 됨. Scale(Sun) * 원래 행렬부터. 원래 행렬은 코드에서 언급x 엄청 많으니까.
		// 실제 계산은 A*B*C*행렬에서 ABC부터 한다. 행렬 곱셈은 교환법칙 성립 안 하지만 순서는 안 바꾸고 계산 먼저 하는 건 ㄱㅊ
		// A*B*C 하면 Model Matrix 나온다. 여기다 원래 행렬 곱한다. 원래 행렬을 중간 과정에서 곱해대면 계산 수 너무 많아진다.
		// // world transformation, 물체 배치 > 회전 스케일 등
		// I * Rotation(x-40도) * Rotation(Sun) * Rotation(x-90도) * Scale(Sun) * 원래 행렬
		// 원래 행렬은 Vertex/Sphere 전체
		// 태양은 그냥 그리면 북극이 카메라 보게 돼있어서 한번 회전시킴(위쪽보게) > Rotation(x-90도)
		glm::mat4 model = glm::mat4(1.0f); // 4x4 매트릭스 선언. 매트릭스의 모든 원소에 1.0f 넣어라 > 이 매트릭스가 맨 앞이고 밑 줄들의 매트릭스는 이 뒤에 곱해진다. 
		model = glm::rotate(model, glm::radians(40.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // 40도 돌림(중요x, 잘 보려고)
		glm::mat4 sun_model = model; // I * R(x-40도). 중간값 저장.
		// 시간에 따른 태양의 자전
		model = glm::rotate(model, (float)glfwGetTime() * rot_speed / rotp_sun, glm::vec3(0.0f, 1.0f, 0.0f));	// the rotation of the sun
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // 
		model = glm::scale(model, glm::vec3(radi_sun, radi_sun, radi_sun)); // 태양 크기로 scale
		starShader.setMat4("model", model);

		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_sun);

		// render the sphere
		glBindVertexArray(sphereVAO);
		glDrawArrays(GL_TRIANGLES, 0, nSphereVert);

		// Planets, moon 	
		// -----------
		// be sure to activate shader when setting uniforms/drawing objects
		planetShader.use();
		// view/projection transformations
		planetShader.setMat4("projection", projection);
		planetShader.setMat4("view", view);
		// eye position
		planetShader.setVec3("eyePos", camera.Position);

		// draw the sphere object
		// light properties
		//glm::vec3 lightColor(1.0, 1.0, 1.0);;
		//glm::vec3 diffuseColor = lightColor * glm::vec3(0.8f); // decrease the influence 
		//glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f); // low influenc
		planetShader.setVec3("light.ambient", ambientColor);
		planetShader.setVec3("light.diffuse", diffuseColor);
		planetShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
		planetShader.setVec3("light.position", lightPos);
		// material properties
		planetShader.setVec3("material.ambient", 0.5f, 0.5f, 0.5f);
		planetShader.setVec3("material.diffuse", 0.9f, 0.9f, 0.9f);
		planetShader.setVec3("material.specular", 0.5f, 0.5f, 0.5f); // specular lighting doesn't have full effect on this object's material
		planetShader.setFloat("material.shininess", 20.0f);

		// mercury 	
		// -----------
		// world transformation
		// 북극 위로 돌리고 / 크기만큼 키우고 / 자전하고 / 위치까지 x축으로 평행이동하고 / 공전하고 / 기울이고
		// 윗줄에서  기울이고의 뜻 > 태양계 좀 삐딱하게 봐야 잘 보여서 태양계를 약간 회전시켜둠
		float dist = radi_sun + 3 * radi_mercury; // 태양 반지름보다 커야 태양 밖에 있음 + 적당한 거리. 행성 간 거리는 실제비율 반영보단 간편함을 목적으로 적당한 값을 줌, 실제 비율을 쓰려면 dist = 2080.0f * radi_mercury;여야 해서 보기 힘듬.
		model = sun_model;
		model = glm::rotate(model, (float)glfwGetTime() * rot_speed / revp_mercury, glm::vec3(0.0f, 1.0f, 0.0f));	// mercury 공전, y축
		model = glm::translate(model, glm::vec3(dist, 0.0f, 0.0f));						// the translation of the mercury from the sun
		model = glm::rotate(model, (float)glfwGetTime() * rot_speed / rotp_mercury, glm::vec3(0.0f, 1.0f, 0.0f));	// mercury 자전, y축
		model = glm::scale(model, glm::vec3(radi_mercury, radi_mercury, radi_mercury)); // 행성 크기로 scale
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // 행성 누워있어서 북극을 위쪽으로 회전
		planetShader.setMat4("model", model);

		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_mercury);

		// render the sphere
		glBindVertexArray(sphereVAO);
		glDrawArrays(GL_TRIANGLES, 0, nSphereVert);

		// venus 	
		// -----------
		// world transformation
		dist = dist + 3 * radi_venus;
		model = sun_model;
		model = glm::rotate(model, (float)glfwGetTime() * rot_speed / revp_venus, glm::vec3(0.0f, 1.0f, 0.0f));	// the revolution of the earth
		model = glm::translate(model, glm::vec3(dist, 0.0f, 0.0f));						// the translation of the earth from the sun
		model = glm::rotate(model, (float)glfwGetTime() * rot_speed / rotp_venus, glm::vec3(0.0f, 1.0f, 0.0f));	// the rotation of the earth
		model = glm::scale(model, glm::vec3(radi_venus, radi_venus, radi_venus));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		planetShader.setMat4("model", model);

		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_venus);

		// render the sphere
		glBindVertexArray(sphereVAO);
		glDrawArrays(GL_TRIANGLES, 0, nSphereVert);

		// earth 	
		// -----------
				// world transformation
		dist = dist + 3 * radi_earth;
		model = sun_model; // sun model 써줘야 sun이 영향 받을 때 같이 영향 받음(?)
		model = glm::rotate(model, (float)glfwGetTime() * rot_speed / revp_earth, glm::vec3(0.0f, 1.0f, 0.0f));	// the revolution of the earth
		model = glm::translate(model, glm::vec3(dist, 0.0f, 0.0f));						// the translation of the earth from the sun
		// 다른 태양계 행성들이 sun_model 갖다 쓰듯 earth_model을 달이 가져다 쓰기 위해 저장. 지구의 윗 2줄 움직임 쫓아가기 위해. 교수님은 지구 자전 밑줄에 하셨는데 지구 자전은 빠졌어야 됐다고 하셨으니까 윗줄로.
		glm::mat4 earth_model = model;
		model = glm::rotate(model, (float)glfwGetTime() * rot_speed / rotp_earth, glm::vec3(0.0f, 1.0f, 0.0f));	// the rotation of the earth
		model = glm::scale(model, glm::vec3(radi_earth, radi_earth, radi_earth));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		planetShader.setMat4("model", model);

		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_earth);

		// render the sphere
		glBindVertexArray(sphereVAO);
		glDrawArrays(GL_TRIANGLES, 0, nSphereVert);

		// moon 	
		// -----------
		// world transformation
		model = earth_model;
		// glm::rotate는 model 행렬에 회전 변환을 곱하는 함수. glfwGetTime()은 프로그램 시작 후 경과한 초 반환, 애니메이션의 시간 기반 움직임에 사용
		// 
		// glm::vec3(0.0f, 1.0f, 0.0f)은 회전축 의미, y축 회전 
		model = glm::rotate(model, (float)glfwGetTime() * rot_speed / revp_moon, glm::vec3(0.0f, 1.0f, 0.0f));	// 공전
		model = glm::translate(model, glm::vec3(1.5f * radi_earth, 0.0f, 0.0f));						// the translation of the moon from the earth
		// model = glm::rotate(model, (float)glfwGetTime() * rot_speed / rotp_moon, glm::vec3(0.0f, 1.0f, 0.0f));// the rotation of the moon
		model = glm::rotate(model, ((float)glfwGetTime() * rot_speed / rotp_moon) - (float)glfwGetTime() * rot_speed / revp_moon, glm::vec3(0.0f, 1.0f, 0.0f));	// 달의 자전. 자전과 공전의 각도 차이만큼 
		model = glm::scale(model, glm::vec3(radi_moon, radi_moon, radi_moon));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); 
		planetShader.setMat4("model", model);

		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_moon);

		// render the sphere
		glBindVertexArray(sphereVAO);
		glDrawArrays(GL_TRIANGLES, 0, nSphereVert);

		// mars 	
		// -----------
		// world transformation
		dist = dist + 3 * radi_mars;
		model = sun_model;
		model = glm::rotate(model, (float)glfwGetTime() * rot_speed / revp_mars, glm::vec3(0.0f, 1.0f, 0.0f));	// the revolution of the earth
		model = glm::translate(model, glm::vec3(dist, 0.0f, 0.0f));						// the translation of the earth from the sun
		model = glm::rotate(model, (float)glfwGetTime() * rot_speed / rotp_mars, glm::vec3(0.0f, 1.0f, 0.0f));	// the rotation of the earth
		model = glm::scale(model, glm::vec3(radi_mars, radi_mars, radi_mars));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		planetShader.setMat4("model", model);

		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_mars);

		// render the sphere
		glBindVertexArray(sphereVAO);
		glDrawArrays(GL_TRIANGLES, 0, nSphereVert);

		// jupiter 	
		// -----------
		// world transformation
		dist = dist + 6 * radi_jupiter;
		model = sun_model;
		model = glm::rotate(model, (float)glfwGetTime() * rot_speed / revp_jupiter, glm::vec3(0.0f, 1.0f, 0.0f));	// the revolution of the earth
		model = glm::translate(model, glm::vec3(dist, 0.0f, 0.0f));						// the translation of the earth from the sun
		model = glm::rotate(model, (float)glfwGetTime() * rot_speed / rotp_jupiter, glm::vec3(0.0f, 1.0f, 0.0f));	// the rotation of the earth
		model = glm::scale(model, glm::vec3(radi_jupiter, radi_jupiter, radi_jupiter));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		planetShader.setMat4("model", model);

		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_jupiter);

		// render the sphere
		glBindVertexArray(sphereVAO);
		glDrawArrays(GL_TRIANGLES, 0, nSphereVert);

		// saturn 	
		// -----------
		// world transformation
		dist = dist + 6 * radi_saturn;
		model = sun_model;
		model = glm::rotate(model, (float)glfwGetTime() * rot_speed / revp_saturn, glm::vec3(0.0f, 1.0f, 0.0f));	// the revolution of the earth
		model = glm::translate(model, glm::vec3(dist, 0.0f, 0.0f));						// the translation of the earth from the sun
		model = glm::rotate(model, (float)glfwGetTime() * rot_speed / rotp_saturn, glm::vec3(0.0f, 1.0f, 0.0f));	// the rotation of the earth
		model = glm::scale(model, glm::vec3(radi_saturn, radi_saturn, radi_saturn));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		planetShader.setMat4("model", model);

		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_saturn);

		// render the sphere
		glBindVertexArray(sphereVAO);
		glDrawArrays(GL_TRIANGLES, 0, nSphereVert);

		// uranus 	
		// world transformation
		dist = dist + 6 * radi_uranus;
		model = sun_model;
		model = glm::rotate(model, (float)glfwGetTime() * rot_speed / revp_uranus, glm::vec3(0.0f, 1.0f, 0.0f));	// the revolution of the earth
		model = glm::translate(model, glm::vec3(dist, 0.0f, 0.0f));						// the translation of the earth from the sun
		model = glm::rotate(model, (float)glfwGetTime() * rot_speed / rotp_uranus, glm::vec3(0.0f, 1.0f, 0.0f));	// the rotation of the earth
		model = glm::scale(model, glm::vec3(radi_uranus, radi_uranus, radi_uranus));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		planetShader.setMat4("model", model);

		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_uranus);

		// render the sphere
		glBindVertexArray(sphereVAO);
		glDrawArrays(GL_TRIANGLES, 0, nSphereVert);

		// neptune 	
		// world transformation
		dist = dist + 6 * radi_neptune;
		model = sun_model;
		model = glm::rotate(model, (float)glfwGetTime() * rot_speed / revp_neptune, glm::vec3(0.0f, 1.0f, 0.0f));	// the revolution of the earth
		model = glm::translate(model, glm::vec3(dist, 0.0f, 0.0f));						// the translation of the earth from the sun
		model = glm::rotate(model, (float)glfwGetTime() * rot_speed / rotp_neptune, glm::vec3(0.0f, 1.0f, 0.0f));	// the rotation of the earth
		model = glm::scale(model, glm::vec3(radi_neptune, radi_neptune, radi_neptune));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		planetShader.setMat4("model", model);

		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_neptune);

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
// 원을 그릴 때는 정점을 그리는 순서도 중요하다. 인근 점들이 나오게 구해야함.
// 일반적인 원의 방정식인 y = += 루트 r^2 - x^2를 풀어서 그리면 안 됨. 
// 이 방식대로 하면 x가 1일 때 위 아래 대칭으로 점 하나씩 나오는데 이러면 인근 점이 아니라 y축 대칭으로 위아래 점 하나씩 구하게 됨
// 그래서 (x,y) = (r cos 세타, r sin세타)의 극좌표계로 그린다.
// 세타값을 0부터 2파이(라디안)까지 범위 바꾸면 인접점으로 구해짐


// u는 가로로 도는 거, 경도. v는 위도, 위로 도는 거.
// 극좌표계로 u 0에서 2파이까지 돌리면서 원 만들고, v는 0~180도 혹은 -90 ~ 90도 사이 돌리면서 위 아래로 원 만듦.
void init_sphere(float** vertices, int* nVert, int* nAttr)
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

void init_textures()
{
	// load and create a texture 
	// -------------------------
	int width, height, nrChannels;
	unsigned char* data;

	// texture_sun 
	// ---------
	glGenTextures(1, &texture_sun);
	glBindTexture(GL_TEXTURE_2D, texture_sun);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	data = stbi_load("..\\textures\\solarsystem\\2k_sun.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	// texture_earth 
	// ---------
	glGenTextures(1, &texture_earth);
	glBindTexture(GL_TEXTURE_2D, texture_earth);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	data = stbi_load("..\\textures\\solarsystem\\2k_earth_daymap.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	// texture_moon 
	// ---------
	glGenTextures(1, &texture_moon);
	glBindTexture(GL_TEXTURE_2D, texture_moon);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	data = stbi_load("..\\textures\\solarsystem\\2k_moon.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	// texture_mercury 
	// ---------
	glGenTextures(1, &texture_mercury);
	glBindTexture(GL_TEXTURE_2D, texture_mercury);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	data = stbi_load("..\\textures\\solarsystem\\2k_mercury.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	// texture_venus 
	// ---------
	glGenTextures(1, &texture_venus);
	glBindTexture(GL_TEXTURE_2D, texture_venus);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	data = stbi_load("..\\textures\\solarsystem\\2k_venus_surface.jpg", &width, &height, &nrChannels, 0);
	//data = stbi_load("..\\textures\\solarsystem\\2k_venus_atmosphere.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	// texture_mars 
	// ---------
	glGenTextures(1, &texture_mars);
	glBindTexture(GL_TEXTURE_2D, texture_mars);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	data = stbi_load("..\\textures\\solarsystem\\2k_mars.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	// texture_mars 
	// ---------
	glGenTextures(1, &texture_jupiter);
	glBindTexture(GL_TEXTURE_2D, texture_jupiter);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	data = stbi_load("..\\textures\\solarsystem\\2k_jupiter.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	// texture_saturn 
	// ---------
	glGenTextures(1, &texture_saturn);
	glBindTexture(GL_TEXTURE_2D, texture_saturn);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	data = stbi_load("..\\textures\\solarsystem\\2k_saturn.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	// texture_uranus 
	// ---------
	glGenTextures(1, &texture_uranus);
	glBindTexture(GL_TEXTURE_2D, texture_uranus);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	data = stbi_load("..\\textures\\solarsystem\\2k_uranus.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	// texture_neptune 
	// ---------
	glGenTextures(1, &texture_neptune);
	glBindTexture(GL_TEXTURE_2D, texture_neptune);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	data = stbi_load("..\\textures\\solarsystem\\2k_neptune.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
}