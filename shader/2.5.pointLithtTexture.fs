#version 330 core
out vec4 FragColor;

in vec2 texCoord;
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
struct Material{
    //vec3 ambient;
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};
struct Light{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 position;
    float constant;
    float linear;
    float quadratic;
};

uniform vec3 viewPos;
uniform Material material;
uniform Light light;


void main()
{
    //vec3 ambient = material.ambient * light.ambient;
    float distance = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);
    vec3 ambient = texture(material.diffuse,TexCoords).rgb * light.ambient;
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position-FragPos);
    float diff = max(dot(norm, lightDir),0);
    vec3 diffuse = texture(material.diffuse,TexCoords).rgb * diff *  light.diffuse;

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(reflectDir, viewDir), 0.0),material.shininess);
    //vec3 specular = (material.specular * spec) *  light.specular;
    vec3 specular = texture(material.specular,TexCoords).rgb * spec *  light.specular;
    vec3 result = attenuation * (ambient + diffuse + specular);
    FragColor = vec4(result, 1.0);
}
