#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec3 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

uniform samplerCube diffuseTexture;
uniform sampler2D shadowMap;

uniform vec3 lightPos;
uniform vec3 viewPos;
float ShadowCalculation(vec4 fragPosLightSpace, float bias)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    float shadow;
    float samples = 7.0;
    float offset = 2;
    if(projCoords.z > 1.0)
    {
        shadow = 0.0;
    }    
    else
    {
         for(float x = -offset; x < offset; x += offset / (samples * 0.5))
        {
            for(float y = -offset; y < offset; y += offset / (samples * 0.5))
            {
                float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
                shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
            }    
        }
        shadow /= (samples * samples);
    }
    
    
    return shadow;
}

void main()
{           

    vec3 color = texture(diffuseTexture, normalize(fs_in.FragPos - viewPos)).rgb;
    //vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightColor = vec3(1.25);
    
    // ambient
    vec3 ambient = 0.35 * lightColor;
    // diffuse
    //vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    vec3 lightDir = normalize(lightPos - 0);
    float diff = 0.f;
    diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 specular = spec * lightColor;    
    // calculate shadow
    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.0005);
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace,bias);
    vec3 lighting = (ambient + (1.0 - shadow) * (1.0 - ambient)) * color;    



        // HDR tonemap and gamma correct
    lighting = lighting / (lighting + vec3(1.0));
    lighting = pow(lighting, vec3(1.0/2.2)); 
    FragColor = vec4(lighting, 1.0);
    //FragColor = vec4(fs_in.FragPosLightSpace);
}