#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform GlobalUniformBufferLight {
	vec3 DIR_light_direction;
	vec3 DIR_light_color;

	vec3 POINT_light_pos;
	vec3 POINT_light_direction;
	vec3 POINT_light_color;
	vec4 POINT_coneInOutDecayExp;
} gubo;

layout(set = 1, binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragViewDir;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragPos;

layout(location = 0) out vec4 outColor;

vec3 Toon_Diffuse_BRDF(vec3 L, vec3 N, vec3 V, vec3 C, vec3 Cd, float thr) {
	// Toon Diffuse Brdf
	// additional parameters:
	// vec3 Cd : color to be used in dark areas
	// float thr : color threshold
	if(dot(L,N)< thr){
		C = Cd;
		if(dot(L,N)< thr/5){
			C = Cd/8;
		}

	}
	return C;
}

vec3 Toon_Specular_BRDF(vec3 L, vec3 N, vec3 V, vec3 C, float thr)  {
	// Directional light direction
	// additional parameter:
	// float thr : color threshold
	if(dot(V,2*N*dot(L,N)-L) < thr){
		C = vec3 (0.0f,0.0f, 0.0f);
	}
	return C;
}


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


vec3 point_light_dir(vec3 pos) {
	// Point light direction
	return normalize(gubo.POINT_light_pos-pos);
}

vec3 point_light_color(vec3 pos) {
	// Point light color
	return  gubo.POINT_light_color*pow(gubo.POINT_coneInOutDecayExp.z/length(gubo.POINT_light_pos-pos), gubo.POINT_coneInOutDecayExp.w);
}

void main() {
	const vec3  diffColor = texture(texSampler, fragTexCoord).rgb;
	const float specPower = 150.0f;
	const float ambientFactor = 0.65f;

	vec3  LightColor = gubo.DIR_light_color;
	vec3  L = gubo.DIR_light_direction;
	vec3  SpecColor = gubo.DIR_light_color;

	vec3 N = normalize(fragNorm);
	vec3 R = -reflect(L, N);
	vec3 V = normalize(fragViewDir);
	
	//POINT
	vec3  POINT_LightColor = point_light_color(fragPos);
    vec3  POINT_LightDir = point_light_dir(fragPos);

	
	// LAMBERT DIFFUSE
	//vec3 diffuse  = Toon_Diffuse_BRDF(L, N, V, diffColor, 0.2f * diffColor, 0.5f);

	//OREN DIFFUSE
	vec3 diffuse  = LightColor * Oren_Nayar_Diffuse_BRDF(L, N, V, diffColor, 1.5f) ;
	diffuse += POINT_LightColor * Oren_Nayar_Diffuse_BRDF(L, N, V, diffColor, 1.5f);


	// PHONG SPECULAR
	vec3 specular = SpecColor * pow(max(dot(R,V), 0.0f), specPower) ;
	specular += POINT_LightColor * pow(max(dot(R,V), 0.0f), specPower);  

	//TOON SPECULAR 
	//vec3 specular = Toon_Specular_BRDF(L, N, V, vec3(1,1,1), 0.97f) * LightColor;


	// Hemispheric ambient
	vec3 ambient  = (vec3(0.725f,0.403f, 1.0f) * (1.0f + N.y) + vec3(0.003f,0.803f, 0.996f) * (1.0f - N.y)) * diffColor;


	outColor = vec4(clamp(ambientFactor * ambient + diffuse + specular, vec3(0.0f), vec3(1.0f)), 1.0f);
}

