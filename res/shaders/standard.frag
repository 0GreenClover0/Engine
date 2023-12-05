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
};

const int MAX_POINT_LIGHTS = 1;
uniform PointLight pointLights[MAX_POINT_LIGHTS];

struct DirectionalLight
{
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform DirectionalLight directionalLight;

uniform vec3 cameraPosition;

out vec4 FragColor;

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDirection)
{
    vec3 lightDirection = normalize(-light.direction);

    // Diffuse
    float diff = max(dot(normal, lightDirection), 0.0);

    // Specular
    vec3 reflectDirection = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), material.shininess);

    // Combine results
    vec3 ambient = light.ambient  * vec3(texture(material.texture_diffuse1, TextureCoordinatesVertex));
    vec3 diffuse = light.diffuse  * diff * vec3(texture(material.texture_diffuse1, TextureCoordinatesVertex));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, TextureCoordinatesVertex));
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

    // Combine
    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, TextureCoordinatesVertex));
    vec3 diffuse = difference * material.color * light.diffuse * vec3(texture(material.texture_diffuse1, TextureCoordinatesVertex));
    vec3 specular = spec * material.specular * light.specular * vec3(texture(material.texture_specular1, TextureCoordinatesVertex));
    return ambient + diffuse + specular;
}

void main()
{
    vec3 normal = normalize(NormalVertex);
    vec3 viewDirection = normalize(cameraPosition - FragmentPosition);

    // 1. Directional lighting
    vec3 result = CalculateDirectionalLight(directionalLight, normal, viewDirection);

    // 2. Point lights
    for (int i = 0; i < MAX_POINT_LIGHTS; ++i)
    {
        result += CalculatePointLight(pointLights[i], normal, viewDirection);
    }

    FragColor = vec4(result, 1.0);
}
