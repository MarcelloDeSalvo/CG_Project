#version 450

layout(binding = 0) uniform UniformBufferObjectCard {
	mat4 model;
	mat4 view;
	mat4 proj;
	int textureID;
} ubo;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 texCoord;

layout(location = 0) out vec3 fragViewDir;
layout(location = 1) out vec3 fragNorm;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) flat out int textureID;

void main() {
	gl_Position = ubo.proj * ubo.view * ubo.model * vec4(pos, 1.0);
	fragViewDir  = (ubo.view[3]).xyz - (ubo.model * vec4(pos,  1.0)).xyz;
	fragNorm     = (ubo.model * vec4(norm, 0.0)).xyz;
	fragTexCoord = texCoord;
	textureID = ubo.textureID;
}