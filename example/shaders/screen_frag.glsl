#version 330 core

layout (location = 0) out vec4 OutColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 uCameraPos;
uniform vec3 uAmbientColor;

uniform float uTime;

uniform vec3 uColor;

vec3 computeLight(vec3 lightPos, vec3 color, float strength, float diffuseStrength, float specularExponent)
{
	float attenuation = strength / (1.0 + 0.09 * pow(distance(lightPos, FragPos), 2));

	// diffuse
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(lightPos - FragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diffuseStrength * diff * color;

	// specular
	float specularStrength = 1.0 - diffuseStrength;
	vec3 viewDir = normalize(uCameraPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), specularExponent);
	vec3 specular = specularStrength * spec * color;

	return (diffuse + specular) * attenuation;
}

void main()
{
	vec3 light1 = computeLight(vec3(1, 3, 3), vec3(1,0.5,0.5), 5.0, 0.5, 32.0);
	vec3 light2 = computeLight(vec3(0, -1, 10), vec3(0.5,1,0.5), 10.0, 0.1, 256.0);
	vec3 light3 = computeLight(vec3(0, 0, 0), vec3(0.5,0.5,1), 4.0, 0.8, 128.0);
	
	vec3 lights = light1 + light2 + light3;

	vec3 lightContrib = lights + uAmbientColor;

	OutColor = vec4(lightContrib * uColor, 1.0);
}