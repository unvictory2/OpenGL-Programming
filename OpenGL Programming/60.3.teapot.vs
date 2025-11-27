#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;

out vec3 Normal;
out vec3 Position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // 언제나 하는 보정작업
    Normal = mat3(transpose(inverse(model))) * aNormal;
    // Positoion은 output 변수. pos에 모델만 곱한 거임. Position은 월드 좌표라고 생각하면 됨
    Position = vec3(model * vec4(aPos, 1.0));
    // 
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
