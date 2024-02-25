#include "TextureLoaderGL.h"

#include <iostream>
#include <stb_image.h>
#include <glad/glad.h>

std::shared_ptr<TextureLoaderGL> TextureLoaderGL::create()
{
    std::shared_ptr<TextureLoaderGL> texture_loader = std::make_shared<TextureLoaderGL>();
    set_instance(texture_loader);
    return texture_loader;
}

std::uint32_t TextureLoaderGL::texture_from_file(std::string const& path)
{
    std::uint32_t texture_id;
    glGenTextures(1, &texture_id);

    stbi_set_flip_vertically_on_load(true);

    std::int32_t width, height, number_of_components;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &number_of_components, 0);

    if (data == nullptr)
    {
        std::cout << "Texture failed to load at path: " << path << '\n';
        stbi_image_free(data);
        return texture_id;
    }

    GLint format;
    if (number_of_components == 1)
    {
        format = GL_RED;
    }
    else if (number_of_components == 3)
    {
        format = GL_RGB;
    }
    else if (number_of_components == 4)
    {
        format = GL_RGBA;
    }
    else
    {
        std::cout << "Unknown texture format. Assuming RGBA." << '\n';
        format = GL_RGBA;
    }

    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    if (format == GL_RGBA)
    {
        // NOTE: When sampling textures at their borders, OpenGL interpolates the border values with the next repeated value
        // of the texture (because we set its wrapping parameters to GL_REPEAT by default).
        // This is usually okay, but since we're using transparent values, the top of the texture image gets
        // its transparent value interpolated with the bottom border's solid color value.
        // The result is then a slightly semi-transparent colored border you may see wrapped around your textured quad.
        // To prevent this, set the texture wrapping method to GL_CLAMP_TO_EDGE whenever you use alpha textures
        // that you don't want to repeat.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);
    return texture_id;
}
