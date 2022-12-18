#version 330 core

in vec3 out_Color;
in vec2 TexCoord;
in vec3 fNormal;
in vec3 FragPos;

out vec4 color; // diffuse에 곱하는 color

struct Material {
	sampler2D ambient;
	float ambientRate; // 앰비언트 적용비율
	
	sampler2D specular;
	float specularRate; // 스페큘러 적용비율
	float shininess;

	sampler2D diffuse;
};

struct DirLight {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight {
	vec3 position;

	float intencity;
	float range;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct SpotLight {
	vec3 position;
	vec3 direction;

	float intencity;
	float range;

	float cutOff;
	float outerCutOff;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform DirLight dirLight;

//포인트 라이트의 개수
#define NR_POINT_LIGHTS 10
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform int PointLightNum;

//스포트 라이트의 개수
#define NR_SPOT_LIGHTS 10
uniform SpotLight spotLights[NR_SPOT_LIGHTS];
uniform int SpotLightNum;

float Sigmoid(float x);
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

uniform vec3 ViewPos;
uniform Material material;
//uniform Light light;

float Sigmoid(float x) {
	return 1.0 / (1.0 + (exp(-x)));
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = normalize(-light.direction);
	// diffuse shading
	float diff = max(dot(normal, lightDir), 0.0);
	// specular shading
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	// combine results
	vec3 ambient = light.ambient * vec3(texture(material.ambient, TexCoord));
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoord));
	vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoord));
	return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragPos);
	// diffuse shading
	float diff = max(dot(normal, lightDir), 0.0);
	// specular shading
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	// attenuation
	float distance = length(light.position - fragPos);
	float attenuation = 4.0 * light.intencity / (2.0 / light.range + 5.0 * distance / light.range +
		6.0 * light.intencity * (distance * distance) / light.range);
	attenuation = 2.0 * (Sigmoid(attenuation) - 0.5);

	// combine results
	vec3 ambient = light.ambient * vec3(texture(material.ambient, TexCoord));
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoord));
	vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoord));
	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;
	return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
	vec3 lightDir = normalize(light.position - fragPos);
	// diffuse shading
	float diff = max(dot(normal, lightDir), 0.0);
	// specular shading
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	
	// attenuation
	float distance = length(light.position - fragPos);
	float attenuation = 4.0 * light.intencity / (2.0 / light.range + 5.0 * distance / light.range +
		6.0 * light.intencity * (distance * distance) / light.range);
	attenuation = 2.0 * (Sigmoid(attenuation) - 0.5);
	// combine results
	vec3 ambient = light.ambient * vec3(texture(material.ambient, TexCoord));
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoord));
	vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoord));

	float theta = dot(lightDir, normalize(-light.direction));
	float epsilon = (light.cutOff - light.outerCutOff);
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
	diffuse *= intensity;
	specular *= intensity;

	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;
	return (ambient + diffuse + specular);
}

void main(void) {
	int pln = PointLightNum;
	if (PointLightNum > NR_POINT_LIGHTS)
		pln = NR_POINT_LIGHTS;

	int sln = SpotLightNum;
	if (SpotLightNum > NR_SPOT_LIGHTS)
		sln = NR_SPOT_LIGHTS;

	// properties
	vec3 norm = normalize(fNormal);
	vec3 viewDir = normalize(ViewPos - FragPos);

	// phase 1: Directional lighting
	vec3 result = CalcDirLight(dirLight, norm, viewDir);
	// phase 2: Point lights
	for (int i = 0; i < pln; i++)
		result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
	// phase 3: Spot light
	for (int i = 0; i < sln; i++)
		result += CalcSpotLight(spotLights[i], norm, FragPos, viewDir);

	color = vec4(result, 1.0);
}