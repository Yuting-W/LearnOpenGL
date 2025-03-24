#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexture;
//layout (location = 3) in vec3 aTangent;
//layout (location = 4) in vec3 aBitangent;
//layout (location = 5) in int am_BoneIDs;
//layout (location = 6) in float am_Weights;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 modelInvTrans;
uniform mat4 lightSpaceMatrix;//shadow------------------------------------------------------------
out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out vec4 FragPosLightSpace;
void main()
{
	vec3 FragPos = vec3(model * vec4(aPos, 1.0));
	Normal  = vec3(modelInvTrans * aNormal);
	//Normal = aNormal;
	//Normal = mat3(transpose(inverse(model))) * aNormal;  
	
	TexCoords = vec2(aTexture.x,aTexture.y);
	FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);//shadow------------------------------------------------------------

	gl_Position = projection * view * vec4(FragPos, 1.0);
}
