#version 330 core
out vec4 FragColor;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 Normal;  
in vec3 FragPos;  
  
uniform Material material;
uniform Light light;
uniform vec3 eyePos;

void main()
{

    // diffuse term
    // 음영 만들기 위함. 빛의 방향과 노말 방향으로 판단. phong 쉐이딩 중에서도 가장 중요. 
    // 둘 다 normalize 해줘야 내적해도 1을 넘지 않는 벡터가 나옴. 안 하면 기괴한 값이 나온다.
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0); // 즉 이 diff는 0~1사이 값
    // 값의 범위에 따라 값 정해버리기. 이 구간을 나눠서 그림자가 계단식으로 층져서 나타난다. 불연속적으로, 이산적으로.
    if (diff > 0.8f) diff = 0.9f;
    else if (diff > 0.6f) diff = 0.7f;
    else if (diff > 0.4f) diff = 0.5f;
    else if (diff > 0.2f) diff = 0.3f;
    else diff = 0.1f;
    vec3 diffuse = light.diffuse * (diff * material.diffuse);
    // 별개의 얘기로, diff가 0이 되는 부분은 캐릭터의 실루엣이다. 빛이 내 뒤통수로 가는 부분에서 diff는 0이고 뒤통수부턴 음수기 때문.

	// specular term 
    vec3 View = normalize(eyePos - FragPos);
	vec3 refl = 2.0 * norm * dot(norm, lightDir) - lightDir; //    vec3 reflectDir = reflect(-lightDir, norm);  
 	float spec = pow(max(dot(refl, View), 0.0), material.shininess); 
 /*
    if (spec > 0.8f) spec = 0.9f;
    else if (spec > 0.6f) spec = 0.7f;
    else if (spec > 0.4f) spec = 0.5f;
    else if (spec > 0.2f) spec = 0.3f;
    else spec = 0.1f;
    */
	vec3 specular = light.specular * (spec * material.specular);

	// ambient term
     vec3 ambient = light.ambient * material.ambient;
            
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
} 
