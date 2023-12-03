#version 400 core

in vec2 TextureCoordinatesVertex;
in vec3 FragmentPosition;
in vec3 NormalVertex;

out vec4 FragColor;

uniform vec4 objectColor;
uniform vec4 lightColor;
uniform vec3 lightPosition;
uniform vec3 cameraPosition;

uniform sampler2D texture_diffuse1;

const float ambientStrength = 0.2;
const float specularStrength = 0.8;
const int shininess = 32;

void main()
{
    vec3 normalNormalized = normalize(NormalVertex);
    vec3 lightDirectionNormalized = normalize(lightPosition - FragmentPosition);

    vec3 viewDirectionNormalized = normalize(cameraPosition - FragmentPosition);
    vec3 reflectDirection = reflect(-lightDirectionNormalized, normalNormalized);

    float difference = max(dot(normalNormalized, lightDirectionNormalized), 0.0);
    vec3 diffuse = difference * lightColor.xyz;
    vec3 ambient = ambientStrength * lightColor.xyz;

    float specular = pow(max(dot(viewDirectionNormalized, reflectDirection), 0.0), shininess);
    vec3 specularColor = specularStrength * specular * lightColor.xyz;

    vec3 result = (ambient + diffuse + specularColor) * objectColor.xyz;
    FragColor = vec4(result, 1.0) * texture(texture_diffuse1, TextureCoordinatesVertex);
}