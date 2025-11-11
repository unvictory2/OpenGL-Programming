// solarsystem vs 그대로 가져옴. 이게 상위호환이라
#version 330 core
layout (location = 0) in vec3 aPos;        // 구/바닥: 위치
layout (location = 1) in vec3 aNormal;     // 구/바닥: 노말
layout (location = 2) in vec2 aTexCoord;   // 구/바닥: UV

out vec3 FragPos;
out vec2 TexCoord;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal  = normalize(transpose(inverse(mat3(model))) * aNormal);
    TexCoord = aTexCoord;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
