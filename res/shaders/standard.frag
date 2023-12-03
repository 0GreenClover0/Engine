#version 400 core

in vec2 TextureCoordinatesVertex;
in vec3 FragmentPosition;
in vec3 NormalVertex;

struct Material
{
    sampler2D texture_diffuse1;
    vec3 color;
    vec3 specular;
    float shininess;
};

uniform Material material;

struct Light
{
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Light light;

uniform vec3 cameraPosition;

out vec4 FragColor;

void main()
{
    // Ambient
    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, TextureCoordinatesVertex));

    // Diffuse
    vec3 normalNormalized = normalize(NormalVertex);
    vec3 lightDirectionNormalized = normalize(light.position - FragmentPosition);
    float difference = max(dot(normalNormalized, lightDirectionNormalized), 0.0);
    vec3 diffuse = difference * material.color * light.diffuse * vec3(texture(material.texture_diffuse1, TextureCoordinatesVertex));

    // Specular
    vec3 viewDirectionNormalized = normalize(cameraPosition - FragmentPosition);
    vec3 reflectDirection = reflect(-lightDirectionNormalized, normalNormalized);
    float specular = pow(max(dot(viewDirectionNormalized, reflectDirection), 0.0), material.shininess);
    vec3 specularColor = specular * material.specular * light.specular;

    vec3 result = ambient + diffuse + specularColor;
    FragColor = vec4(result, 1.0);
}