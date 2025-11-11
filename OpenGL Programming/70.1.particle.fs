// solarsystem fs 그대로 가져옴 텍스쳐 적용하려고
#version 330 core
out vec4 FragColor;

struct Material { vec3 ambient; vec3 diffuse; vec3 specular; float shininess; };
struct Light    { vec3 position; vec3 ambient; vec3 diffuse; vec3 specular; };

in vec3 Normal;  
in vec2 TexCoord;
in vec3 FragPos;  

uniform sampler2D texture1;   // 텍스처 샘플러
uniform Material material;    // 재질
uniform Light light;          // 광원
uniform vec3 eyePos;          // 카메라 위치

void main()
{
    vec3  norm     = normalize(Normal);
    vec3  lightDir = normalize(light.position - FragPos);
    float diff     = max(dot(norm, lightDir), 0.0);

    // 텍스처 색을 diffuse에 사용
    vec3 diffuse = light.diffuse * (diff * texture(texture1, TexCoord).rgb);

    // 하이라이트
    vec3 viewDir = normalize(eyePos - FragPos);
    vec3 refl    = 2.0 * norm * dot(norm, lightDir) - lightDir;
    float spec   = pow(max(dot(refl, viewDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);

    // 주변광
    vec3 ambient = light.ambient * material.ambient;

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
