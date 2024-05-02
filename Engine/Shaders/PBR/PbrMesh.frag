#version 450 core

const float PI = 3.141592;
const float Epsilon = 0.00001;
const int NumLights = 3;
const vec3 Fdielectric = vec3(0.04);

layout(location=0) in Vertex
{
	vec3 position;
	vec2 texcoord;
	mat3 tangentBasis;
} vin;

struct AnalyticalLight {
	vec3 direction;
	vec3 radiance;
};

layout(location=0) out vec4 color;

layout(set=0, binding=1) uniform ShadingUniforms
{
    AnalyticalLight lights[NumLights];
	vec3 eyePosition;
}lightUniform;

layout(set=1, binding=0) uniform sampler2D albedoTexture;
layout(set=1, binding=1) uniform sampler2D normalTexture;
layout(set=1, binding=2) uniform sampler2D metalnessTexture;
layout(set=1, binding=3) uniform sampler2D roughnessTexture;
layout(set=1, binding=4) uniform samplerCube specularTexture;
layout(set=1, binding=5) uniform samplerCube irradianceTexture;
layout(set=1, binding=6) uniform sampler2D specularBRDF_LUT;

// 定义函数，没有什么好说的
float ndfGGX(float cosLh, float roughness)
{
	float alpha   = roughness * roughness;
	float alphaSq = alpha * alpha;

	float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
	return alphaSq / (PI * denom * denom);
}

float gaSchlickG1(float cosTheta, float k)
{
	return cosTheta / (cosTheta * (1.0 - k) + k);
}

float gaSchlickGGX(float cosLi, float cosLo, float roughness)
{
	float r = roughness + 1.0;
	float k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights.
	return gaSchlickG1(cosLi, k) * gaSchlickG1(cosLo, k);
}

vec3 fresnelSchlick(vec3 F0, float cosTheta)
{
	return F0 + (vec3(1.0) - F0) * pow(1.0 - cosTheta, 5.0);
}

void main()
{
    vec3 albedo = texture(albedoTexture, vin.texcoord).rgb;
	float metalness = texture(metalnessTexture, vin.texcoord).r;
	float roughness = texture(roughnessTexture, vin.texcoord).r;

    // 视角方向
	vec3 Lo = normalize(lightUniform.eyePosition - vin.position);
    // 法线
	vec3 N = normalize(2.0 * texture(normalTexture, vin.texcoord).rgb - 1.0);
	N = normalize(vin.tangentBasis * N);
	float cosLo = max(0.0, dot(N, Lo));

    // 反射方向
	vec3 Lr = 2.0 * cosLo * N - Lo;

    vec3 F0 = mix(Fdielectric, albedo, metalness);

    // 直接光
    vec3 directLighting = vec3(0);
    for(int i = 0 ; i < NumLights ; ++i)
    {
        // 灯光方向
        vec3 Li = -lightUniform.lights[i].direction;
        vec3 Lradiance = lightUniform.lights[i].radiance;

        // 半程向量
        vec3 Lh = normalize(Li + Lo);

        float cosLi = max(0.0, dot(N, Li));
		float cosLh = max(0.0, dot(N, Lh));

        // 菲涅尔
		vec3 F  = fresnelSchlick(F0, max(0.0, dot(Lh, Lo)));
		// 法线分布函数
		float D = ndfGGX(cosLh, roughness);
		// 几何遮蔽
		float G = gaSchlickGGX(cosLi, cosLo, roughness);

        vec3 kd = mix(vec3(1.0) - F, vec3(0.0), metalness);

        vec3 diffuseBRDF = kd * albedo;
        vec3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * cosLi * cosLo);

        directLighting += (diffuseBRDF + specularBRDF) * Lradiance * cosLi;
    }

    // 间接光
    vec3 ambientLighting;
    {
        //  漫反射项Irradiance
        vec3 irradiance = texture(irradianceTexture, N).rgb;

        vec3 F = fresnelSchlick(F0, cosLo);
		vec3 kd = mix(vec3(1.0) - F, vec3(0.0), metalness);

		// 漫反射项
		vec3 diffuseIBL = kd * albedo * irradiance;

        // 根据roughness采样高光项的Irradiance
		int specularTextureLevels = textureQueryLevels(specularTexture);
		vec3 specularIrradiance = textureLod(specularTexture, Lr, roughness * specularTextureLevels).rgb;

        // 查表组装高光BRDF
		vec2 specularBRDF = texture(specularBRDF_LUT, vec2(cosLo, roughness)).rg;

        // 高光项
		vec3 specularIBL = (F0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;

        // 间接光最终Radiance
		ambientLighting = diffuseIBL + specularIBL;
    }

    color = vec4(directLighting + ambientLighting, 1.0);
}