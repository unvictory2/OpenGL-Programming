#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;

// 카메라나 빛 위치는 전부 월드 공간 위치
uniform vec3 eyePos;
uniform samplerCube skybox;

void main()
{    
    // 카메라에서 물체 보는 벡터. 노말이 반드시 unit 벡터여야 하기 때문에 정규화 해줘야 함
    vec3 I = normalize(Position - eyePos);
    // I가 물체에 튕겨서 나가는 벡터. 입사각 + 물체의 노말 넣으면 반사각 줌.
    vec3 R = reflect(I, normalize(Normal));
    // 스카이박스에서 해당하는 배경 가져오기. 마지막의 1.0은 알파값.
    FragColor = vec4(texture(skybox, R).rgb, 1.0);
}