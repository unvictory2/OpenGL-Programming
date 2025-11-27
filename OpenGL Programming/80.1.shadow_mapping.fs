// 메인 쉐이더
#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;

uniform vec3 lightPos;
uniform vec3 viewPos;

//shadow에 들어가면 1, 아니면 0 반환
float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide - z값(쉐도우 맵의 값)은 직접 잴 필요 없고 밑의 1줄로 얻어낼 수 있다
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range 얻어온 z값을 0~1 사이 범위로
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    // 이 부분 수정하면 자글자글한 현상 수정 가능(bias 추가)
    // float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005); 
    float bias = 0.05;  
    float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;  

    return shadow;
}

void main()
{           
    vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightColor = vec3(0.8);
    // ambient
    vec3 ambient = 0.3 * lightColor;
    // diffuse
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;    
    // calculate shadow
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace);            
    // 그림자 안에 들어가면 위의 ShadowCalculation 함수에서 shadow에 1이 들어옴. 그럼 1.0 - 1 해서 0이 되니까 여기 곱해지는 diffuse와 specular는 날리는 거임(어두우니까). ambient는 남기는데, 어두운 곳에서도 조금은 보이는 효과 위해서임.
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;    
    
    FragColor = vec4(lighting, 1.0);
}