#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
uniform mat4 projection;
uniform mat4 view;

out vec3 fragPos;
void main()
{
	fragPos = aPos; 
	mat4 rotView = mat4(mat3(view));
	vec4 clipPos = projection * rotView *vec4(aPos, 1.0);
	gl_Position = clipPos.xyww;
}