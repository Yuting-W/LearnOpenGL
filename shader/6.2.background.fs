#version 330 core
out vec4 FragColor;
in vec3 fragPos;

uniform samplerCube environmentMap;

void main()
{		
    vec3 envColor = texture(environmentMap, fragPos).rgb; 
   
    // HDR tonemap and gamma correct
    envColor = envColor / (envColor + vec3(1.0));
    envColor = pow(envColor, vec3(1.0/2.2)); 
    FragColor = vec4(envColor, 1.0);



}


