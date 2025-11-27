#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    // 큐브맵의 좌표는 바로 방향 벡터로 사용 가능함. 큐브맵이 원점에 있기 때문.
    TexCoords = aPos;
    vec4 pos = projection * view * vec4(aPos, 1.0);
    // 동차좌표라 xyzw에서 xyz를 각각 w로 나눠줘야 우리가 원하는 3차원 좌표가 나옴. 이 나누기는 resterizer가 알아서 나눔
    // xyww는 z에다가 w를 넣는다는 거임. 이러면 나중에 w로 나눌 때 z가 언제나 1이 나옴. 정규화된 view volume에서 언제나 z값이 1인 것. 언제나 가장 멀리 있다 
    gl_Position = pos.xyww;
}  