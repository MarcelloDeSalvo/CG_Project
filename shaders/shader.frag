#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform GlobalUniformBufferLight {
	vec3 DIR_light_direction;
	vec3 DIR_light_color;

	vec3 SPOT_light_pos;
	vec3 SPOT_light_direction;
	vec3 SPOT_light_color;
	vec4 SPOT_coneInOutDecayExp;

	vec3 AMB_light_color_up;
	vec3 AMB_light_color_down;

} gubo;

layout(set = 2, binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragViewDir;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragPos;

layout(location = 0) out vec4 outColor;


vec3 spot_light_dir(vec3 pos) {
	// SPOT light direction
	return normalize(gubo.SPOT_light_pos-pos);
}

vec3 spot_light_color(vec3 pos) {
	// SPOT light color
	vec3 light = gubo.SPOT_light_color;
	float clamp =  clamp( (   ( dot( normalize(gubo.SPOT_light_pos-pos), gubo.SPOT_light_direction) - gubo.SPOT_coneInOutDecayExp.x ) / (gubo.SPOT_coneInOutDecayExp.y-gubo.SPOT_coneInOutDecayExp.x)  ), 0.0f, 1.0f);
	return gubo.SPOT_light_color*clamp;
}

void main() {
	const vec3  diffColor = texture(texSampler, fragTexCoord).rgb;
	const float specPower = 150.0f;
	const float ambientFactor = 0.35f;

	vec3  LightColor = gubo.DIR_light_color;
	vec3  L = gubo.DIR_light_direction;
	vec3  SpecColor = gubo.DIR_light_color;

	vec3 N = normalize(fragNorm);
	vec3 R = -reflect(L, N);
	vec3 V = normalize(fragViewDir);
	
	vec3 diffuse = vec3(0,0,0);
	vec3 specular = vec3(0,0,0);

	//POINT
	vec3  SPOT_LightColor = spot_light_color(fragPos);
    vec3  SPOT_LightDir = spot_light_dir(fragPos);

	
	// LAMBERT DIFFUSE
	diffuse	+= LightColor * diffColor * max(dot(L,N),0);
	diffuse	+= SPOT_LightColor * diffColor * max(dot(L,N),0);

	// PHONG SPECULAR
	specular += SpecColor * pow(max(dot(R,V), 0.0f), specPower) ;
	specular += SPOT_LightColor * pow(max(dot(R,V), 0.0f), specPower);  

	// Hemispheric ambient
	vec3 ambient  = (gubo.AMB_light_color_up * (1.0f + N.y) + gubo.AMB_light_color_down  * (1.0f - N.y)) * diffColor;


	outColor = vec4(clamp(ambientFactor * ambient + diffuse + specular, vec3(0.0f), vec3(1.0f)), 1.0f);
}

