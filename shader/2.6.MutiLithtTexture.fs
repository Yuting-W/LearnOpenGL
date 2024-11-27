#version 330 core
out vec4 FragColor;


struct Material{
    //vec3 ambient;
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};
struct DirLight {
    vec3 direction;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};


struct SpotLight{
    vec3 position;
    vec3 direction; //slot 

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    
    
    float cutOff; //cos of cutting angle 外光切cos
    float outerCutOff; //cos of cutting angle 内光切cos

    float constant;
    float linear;
    float quadratic;
};
struct PointLight{
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    
    float constant;
    float linear;
    float quadratic;
};
#define NUM_POINTLIGHT 4

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
uniform vec3 viewPos;
uniform Material material;
uniform DirLight dirLight;
uniform PointLight pointLights[NUM_POINTLIGHT];
uniform SpotLight spotLight;
// function prototypes
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 result = CalcDirLight(dirLight, norm, viewDir);
    result = result + CalcSpotLight(spotLight, norm, FragPos, viewDir);
    for(int i = 0; i < NUM_POINTLIGHT; i++)
    {
        result = result + CalcPointLight(pointLights[i], norm, FragPos, viewDir);
    }

    FragColor = vec4(result, 1.0); 
   
}


vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize( - light.direction);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 reflectDir = reflect( - lightDir, normal);
    float spec = pow(max(dot(reflectDir, viewDir), 0.0), material.shininess);
    vec3 ambient = texture(material.diffuse, TexCoords).rgb * light.ambient;
    vec3 diffuse = texture(material.diffuse, TexCoords).rgb * light.diffuse * diff;
    vec3 specular = texture(material.specular, TexCoords).rgb * light.specular * spec;

    return ambient + diffuse + specular;
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 reflectDir = reflect( - lightDir, normal);
    float spec = pow(max(dot(reflectDir, viewDir), 0.0), material.shininess);
    vec3 ambient = texture(material.diffuse, TexCoords).rgb * light.ambient;
    vec3 diffuse = texture(material.diffuse, TexCoords).rgb * light.diffuse * diff;
    vec3 specular = texture(material.specular, TexCoords).rgb * light.specular * spec;
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + distance * light.linear + distance * distance * light.quadratic);
    return attenuation * (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{

    vec3 lightDir = normalize(light.position - FragPos);
    float theta = dot(lightDir, normalize(-light.direction));
    float instensity = clamp((theta - light.outerCutOff)/(light.cutOff - light.outerCutOff), 0.0, 1.0);

    float diff = max(dot(lightDir, normal), 0.0);
    vec3 reflectDir = reflect( - lightDir, normal);
    float spec = pow(max(dot(reflectDir, viewDir), 0.0), material.shininess);
    vec3 ambient = texture(material.diffuse, TexCoords).rgb * light.ambient;
    vec3 diffuse = texture(material.diffuse, TexCoords).rgb * light.diffuse * diff;
    vec3 specular = texture(material.specular, TexCoords).rgb * light.specular * spec;
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + distance * light.linear + distance * distance * light.quadratic);
    return instensity * attenuation * (ambient + diffuse + specular);
}