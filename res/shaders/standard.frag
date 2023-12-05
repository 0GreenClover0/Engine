#version 400 core

in vec2 TextureCoordinatesVertex;
in vec3 FragmentPosition;
in vec3 NormalVertex;

struct Material
{
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    vec3 color;
    float specular;
    float shininess;
};

uniform Material material;

struct PointLight
{
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

const int MAX_POINT_LIGHTS = 1;
uniform int pointLightCount;
uniform PointLight pointLights[MAX_POINT_LIGHTS];

struct DirectionalLight
{
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform bool directionalLightOn;
uniform DirectionalLight directionalLight;

struct SpotLight
{
    vec3 position;
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;

    float cutOff;
    float outerCutOff;
};

const int MAX_SPOT_LIGHTS = 4;
uniform int spotLightCount;
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];

uniform vec3 cameraPosition;

out vec4 FragColor;

vec3 diffuse_texture = vec3(0.0, 0.0, 0.0);
vec3 specular_texture = vec3(0.0, 0.0, 0.0);

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDirection)
{
    vec3 lightDirection = normalize(-light.direction);

    // Diffuse
    float diff = max(dot(normal, lightDirection), 0.0);

    // Specular
    vec3 reflectDirection = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), material.shininess);

    // Combine results
    vec3 ambient = light.ambient  * diffuse_texture;
    vec3 diffuse = light.diffuse  * diff * diffuse_texture;
    vec3 specular = light.specular * spec * specular_texture;
    return ambient + diffuse + specular;
}

vec3 CalculatePointLight(PointLight light, vec3 normal, vec3 viewDirection)
{
    vec3 lightDirection = normalize(light.position - FragmentPosition);

    // Diffuse
    float difference = max(dot(normal, lightDirection), 0.0);

    // Specular
    vec3 reflectDirection = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), material.shininess);

    // Attenuation
    float distance = length(light.position - FragmentPosition);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // Combine
    vec3 ambient = attenuation * light.ambient * diffuse_texture;
    vec3 diffuse = attenuation * difference * material.color * light.diffuse * diffuse_texture;
    vec3 specular = attenuation * spec * material.specular * light.specular * specular_texture;
    return ambient + diffuse + specular;
}

vec3 CalculateSpotLight(SpotLight light, vec3 normal, vec3 viewDirection)
{
    vec3 lightDirection = normalize(light.position - FragmentPosition);

    // Diffuse
    float difference = max(dot(normal, lightDirection), 0.0);

    // Specular
    vec3 reflectDirection = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), material.shininess);

    // Attenuation
    float distance = length(light.position - FragmentPosition);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // Combine
    vec3 ambient = attenuation * light.ambient * diffuse_texture;
    vec3 diffuse = attenuation * difference * material.color * light.diffuse * diffuse_texture;
    vec3 specular = attenuation * spec * material.specular * light.specular * specular_texture;

    // Spotlight intensity
    float theta = dot(lightDirection, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    return intensity * (ambient + diffuse + specular);
}

void main()
{
    vec3 normal = normalize(NormalVertex);
    vec3 viewDirection = normalize(cameraPosition - FragmentPosition);

    diffuse_texture = vec3(texture(material.texture_diffuse1, TextureCoordinatesVertex));
    specular_texture = vec3(texture(material.texture_specular1, TextureCoordinatesVertex));

    vec3 result = vec3(0.0, 0.0, 0.0);

    // 1. Directional lighting
    if (directionalLightOn)
    {
        result += CalculateDirectionalLight(directionalLight, normal, viewDirection);
    }

    // 2. Point lights
    for (int i = 0; i < pointLightCount; ++i)
    {
        result += CalculatePointLight(pointLights[i], normal, viewDirection);
    }

    // 3. Spot lights
    for (int i = 0; i < spotLightCount; ++i)
    {
        result += CalculateSpotLight(spotLights[i], normal, viewDirection);
    }

    FragColor = vec4(result, 1.0);
}
