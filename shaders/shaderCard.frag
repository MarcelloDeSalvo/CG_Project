#version 450

layout(binding = 1) uniform texture2D textures[8];
layout(binding = 2) uniform sampler samp;

layout(location = 0) in vec3 fragViewDir;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) flat in int textureID;

layout(location = 0) out vec4 outColor;

void main() {
	const vec3  diffColor = texture(sampler2D(textures[textureID], samp), fragTexCoord).rgb;
	const vec3  specColor = vec3(1.0f, 1.0f, 1.0f);
	const float specPower = 150.0f;
	const vec3  L = vec3(0.32f, 0.2f, -1.0f);
	
	vec3 N = normalize(fragNorm);
	vec3 R = -reflect(L, N);
	vec3 V = normalize(fragViewDir);
	
	// Lambert diffuse
	vec3 diffuse  = diffColor * max(dot(N,L), 0.0f);
	// Hemispheric ambient
	vec3 ambient  = (vec3(0.1f,0.1f, 0.1f) * (1.0f + N.y) + vec3(0.0f,0.0f, 0.1f) * (1.0f - N.y)) * diffColor;
	
	outColor = vec4(clamp(0.7f* ambient + 0.7f*diffuse, vec3(0.0f), vec3(1.0f)), 1.0f);
}