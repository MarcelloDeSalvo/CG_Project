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


vec3 Oren_Nayar_Diffuse_BRDF(vec3 L, vec3 N, vec3 V, vec3 C, float sigma) {
	// Directional light direction
	// additional parameter:
	// float sigma : roughness of the material
	float teta_i = acos(dot(L, N));
	float teta_r = acos(dot(V, N));
	float alpha = max ( teta_i, teta_r);
	float beta = min ( teta_i, teta_r);

	float sigma_squared = pow (sigma, 2);
	float A = 1.0f - 0.5f * ( sigma_squared / (sigma_squared + 0.33f) );
	float B = 0.45f * ( sigma_squared / (sigma_squared + 0.09f) );

	vec3 vi = normalize ( L - dot(L,N)*N );
	vec3 vr = normalize ( V - dot(V,N)*N );
	float G = max (0.0f, dot (vi,vr));
	vec3 clamp = C * clamp (dot (L, N), 0.0f, 1.0f);

	return clamp*(A + B*G*sin(alpha)*tan(beta));
}


vec3 spot_light_dir(vec3 pos) {
	// SPOT light direction
	return normalize(gubo.SPOT_light_pos-pos);
}

vec3 spot_light_color(vec3 pos) {
	// SPOT light color
	return  gubo.SPOT_light_color*pow(gubo.SPOT_coneInOutDecayExp.z/length(gubo.SPOT_light_pos-pos), gubo.SPOT_coneInOutDecayExp.w);
}

void main() {
	const vec3  diffColor = texture(texSampler, fragTexCoord).rgb;
	const float ambientFactor = 0.69f;
	const float roughness = 1.5f;
	const float specPower = 5.0f;

	vec3  LightColor = gubo.DIR_light_color;
	vec3  L = gubo.DIR_light_direction;

	vec3 N = normalize(fragNorm);
	vec3 R = -reflect(L, N);
	vec3 V = normalize(fragViewDir);

	vec3 diffuse = vec3(0,0,0);
	
	//POINT
	vec3  SPOT_LightColor = spot_light_color(fragPos);
    vec3  SPOT_LightDir = spot_light_dir(fragPos);

	//OREN DIFFUSE
	diffuse += LightColor * Oren_Nayar_Diffuse_BRDF(L, N, V, diffColor, roughness) ;
	diffuse += SPOT_LightColor * Oren_Nayar_Diffuse_BRDF(L, N, V, diffColor, roughness);
	
	// PHONG SPECULAR
	vec3 specular = LightColor * pow(max(dot(R,V), 0.0f), specPower) ;
	specular += SPOT_LightColor * pow(max(dot(R,V), 0.0f), specPower);  

	// Hemispheric ambient
	vec3 ambient  = (gubo.AMB_light_color_up * (1.0f + N.y) + gubo.AMB_light_color_down  * (1.0f - N.y)) * diffColor;


	outColor = vec4(clamp(ambientFactor * ambient + diffuse + specular, vec3(0.0f), vec3(1.0f)), 1.0f);
}

