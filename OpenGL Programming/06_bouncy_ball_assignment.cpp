#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/shader.h>
#include <learnopengl/camera.h>

#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
// 행성 코드 텍스쳐 복붙
#define STB_IMAGE_IMPLEMENTATION // 이 줄이랑 밑에 줄 2줄 위아래 바뀌면 안 됨. 인터넷 코드엔 이 줄 없는 경우도 있는데 넣어줘야 한다.
#include <stb_image.h> // 이미지 쓸 때 (텍스쳐 매핑할 때) 필요하다

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void init_sphere(float** vertices, int* nVert, int* nAttr); // 태양계 코드의 함수 시그니처

// 간단 텍스처 로더(행성 코드 방식과 동일한 동작: stbi로 읽고 GL 텍스처 생성)
static unsigned int loadTexture(const char* path) {
    unsigned int tex; glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_set_flip_vertically_on_load(true);
    int w, h, n; unsigned char* data = stbi_load(path, &w, &h, &n, 0);
    if (data) {
        GLenum fmt = (n == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Failed to load texture: " << path << std::endl;
    }
    stbi_image_free(data);
    return tex;
}

// settings
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 1200;

// camera
Camera camera(glm::vec3(0.0f, 3.0f, 12.0f));
float lastX = (float)SCR_WIDTH / 2.0f;
float lastY = (float)SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// 물리 관련
struct RigidBall {
    float radius = 1.0f;      // 구 반지름
    float e = 0.9f;      // 반발계수 (0~1). 1이면 완전탄성, 0.7 정도면 점차 멈춤
    glm::vec3 pos{ 0.0f, 10.0f, 0.0f }; // 시작 위치
    glm::vec3 vel{ 0.0f, 0.0f, 0.0f }; // 시작 속도 0
};

const float G = 9.8f;                 // 중력 가속도
const float GROUND_Y = 0.0f;          // 지면 y 높이
const glm::vec3 GRAVITY(0.0f, -G, 0.0f);

// 멈춤 판정 임계값
const float STOP_VEL_EPS = 0.15f;     // 속도 임계값
const float STOP_POS_EPS = 1e-3f;     // 위치 보정 허용오차

int main() {
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
    // 행성 셰이더(조명/깊이 테스트 전제) 사용 
    glEnable(GL_DEPTH_TEST); // 안 키면 이상한 점 생김...
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPointSize(10.0f);

    // build and compile shaders > 둘 다 내용은 solar planet때 쉐이더랑 똑같음, 텍스쳐 적용 위해
    Shader particleShader("70.1.particle.vs", "70.1.particle.fs"); 

    // 지면 정점 데이터: pos(3) + color(3)
    // 행성 셰이더를 그대로 쓰려고 정점 포맷을 pos3, normal3, uv2(=8 floats)로 맞추기
    float groundSize = 50.0f;
    float gy = GROUND_Y;
    //   x, y, z,    nx,ny,nz,   s, t
    float ground[] = {
        -groundSize, gy, -groundSize,   0,1,0,   0,0,
         groundSize, gy, -groundSize,   0,1,0,   1,0,
         groundSize, gy,  groundSize,   0,1,0,   1,1,

        -groundSize, gy, -groundSize,   0,1,0,   0,0,
         groundSize, gy,  groundSize,   0,1,0,   1,1,
        -groundSize, gy,  groundSize,   0,1,0,   0,1,
    };

    GLuint groundVAO = 0, groundVBO = 0;
    glGenVertexArrays(1, &groundVAO);
    glGenBuffers(1, &groundVBO);
    glBindVertexArray(groundVAO);
    glBindBuffer(GL_ARRAY_BUFFER, groundVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ground), ground, GL_STATIC_DRAW);
    // layout(0): position, layout(1): RGB (vs가 aPos,aRGB 순서로 받음)
    // 행성 셰이더 용으로 (0=aPos, 1=aNormal, 2=aTexCoord) 재매핑
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    // 공 그리는 데는 태양계 코드의 init_sphere 사용(젤 밑에 복붙)
    float* sphereVerts = nullptr; int nSphereVert = 0, nSphereAttr = 0; // nSphereAttr==8: pos3,normal3,uv2
    init_sphere(&sphereVerts, &nSphereVert, &nSphereAttr);

    GLuint sphereVAO = 0, sphereVBO = 0;
    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);

    glBindVertexArray(sphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, nSphereVert * nSphereAttr * sizeof(float), sphereVerts, GL_STATIC_DRAW);

    // rigid_mesh.vs는 layout(0)=aPos, layout(1)=aRGB로 사용
    // 행성 셰이더 사용으로 변경: (0=aPos, 1=aNormal, 2=aTexCoord)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, nSphereAttr * sizeof(float), (void*)0);                 // position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, nSphereAttr * sizeof(float), (void*)(3 * sizeof(float))); // normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, nSphereAttr * sizeof(float), (void*)(6 * sizeof(float))); // uv
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    free(sphereVerts);

    // 텍스처 로드 (구=지구,땅=적당한 아무거나)
    unsigned int texSphere = loadTexture("..\\textures\\solarsystem\\2k_earth_daymap.jpg");
    unsigned int texGround = loadTexture("..\\textures\\solarsystem\\2k_venus_atmosphere.jpg");

    // 물리 초기화 
    RigidBall ball; // 기본값 사용(높이 6에서 시작)

    // ---- 루프 ----
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        // 물리 파트!!! 이 코드 핵심임 ** -------------------------------------
        if (true) {
            // 중력. 자유낙하 방정식
            ball.vel += GRAVITY * deltaTime;           // v(t+dt)
            ball.pos += ball.vel * deltaTime;          // x(t+dt)

            // 바닥 충돌: 구의 접점 y == GROUND_Y -> 중심 y == GROUND_Y + radius
            float minY = GROUND_Y + ball.radius;
            if (ball.pos.y < minY) {
                // 보정. ball.pos.y가 minY=GROUND_Y+ball.radius=1.0보다 내려가면 보정
                ball.pos.y = minY;
                // 아래로 내려오는 중이었다면 반발(ball.vel.y가 음수였으면 ball.vel.y = -ball.e*ball.vel.y(여기서 ball.e=0.7)로 반발)
                if (ball.vel.y < 0.0f) {
                    ball.vel.y = -ball.e * ball.vel.y; // v' = -e v. 
                    // 매우 느려지면 정지 처리
                    if (std::abs(ball.vel.y) < STOP_VEL_EPS) {
                        ball.vel = glm::vec3(0.0f);
                        ball.pos.y = minY;
                    }
                }
            }
        }

        // 렌더링
        glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 행성 셰이더(텍스처+Phong) 사용
        particleShader.use();
        // 공통 행렬
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        particleShader.setMat4("projection", projection);
        particleShader.setMat4("view", view);

        // 행성 셰이더 유니폼: 카메라/광원/재질
        particleShader.setVec3("eyePos", camera.Position);

        // light (태양계 코드)
        particleShader.setVec3("light.position", 21.2f, 21.0f, 12.0f);
        particleShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
        particleShader.setVec3("light.diffuse", 0.8f, 0.8f, 0.8f);
        particleShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

        // material 
        particleShader.setVec3("material.ambient", 0.5f, 0.5f, 0.5f);
        particleShader.setVec3("material.diffuse", 0.9f, 0.9f, 0.9f);
        particleShader.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
        particleShader.setFloat("material.shininess", 20.0f);

        // sampler 바인딩
        particleShader.setInt("texture1", 0);

        // --- 지면 --- (offset=0)
        particleShader.setMat4("model", glm::mat4(1.0f));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texGround);
        glBindVertexArray(groundVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // --- 공 ---
        // 메시를 원점에서 만들었으므로 평행이동 + 스케일로 배치
        glm::mat4 model(1.0f);
        model = glm::translate(model, ball.pos);
        model = glm::scale(model, glm::vec3(ball.radius));
        particleShader.setMat4("model", model);

        glBindTexture(GL_TEXTURE_2D, texSphere);
        glBindVertexArray(sphereVAO);
        glDrawArrays(GL_TRIANGLES, 0, nSphereVert);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &sphereVAO);
    glDeleteBuffers(1, &sphereVBO);
    glDeleteVertexArrays(1, &groundVAO);
    glDeleteBuffers(1, &groundVBO);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window) {
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

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) { lastX = (float)xpos; lastY = (float)ypos; firstMouse = false; }
    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos; // y: bottom->top
    lastX = (float)xpos; lastY = (float)ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll((float)yoffset);
}

// 태양계코드 복붙
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
