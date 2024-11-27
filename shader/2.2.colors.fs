#version 330 core
out vec4 FragColor;

in vec2 texCoord;
in vec3 FragPos;
in vec3 Normal;

struct Material{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};
struct Light{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 position;
};

uniform vec3 viewPos;
uniform Material material;
uniform Light light;


void main()
{
    vec3 ambient = material.ambient * light.ambient;

    float diffStrength = 1.0;
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position-FragPos);
    float diff = max(dot(norm, lightDir),0);
    vec3 diffuse = (material.diffuse * diff) *  light.diffuse;

    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(reflectDir, viewDir), 0.0),32);
    vec3 specular = (material.specular * spec) *  light.specular;
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
