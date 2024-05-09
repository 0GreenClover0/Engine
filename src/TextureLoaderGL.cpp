#include "TextureLoaderGL.h"

#include <iostream>
#include <stb_image.h>

std::shared_ptr<TextureLoaderGL> TextureLoaderGL::create()
{
    std::shared_ptr<TextureLoaderGL> texture_loader = std::make_shared<TextureLoaderGL>();
    set_instance(texture_loader);
    return texture_loader;
}

TextureData TextureLoaderGL::texture_from_file(std::string const& path, TextureSettings const settings)
{
    u32 texture_id;
    glGenTextures(1, &texture_id);

    stbi_set_flip_vertically_on_load(settings.flip_vertically);

    i32 width, height, number_of_components;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &number_of_components, 0);

    if (data == nullptr)
    {
        std::cout << "Texture failed to load at path: " << path << '\n';
        stbi_image_free(data);
        return {};
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

    if (settings.generate_mipmaps)
    {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    // NOTE: When sampling textures at their borders, OpenGL interpolates the border values with the next repeated value
    // of the texture (because we set its wrapping parameters to GL_REPEAT by default).
    // This is usually okay, but since we're using transparent values, the top of the texture image gets
    // its transparent value interpolated with the bottom border's solid color value.
    // The result is then a slightly semi-transparent colored border you may see wrapped around your textured quad.
    // To prevent this, set the texture wrapping method to GL_CLAMP_TO_EDGE whenever you use alpha textures
    // that you don't want to repeat.

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, convert_wrap_mode(settings.wrap_mode_x));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, convert_wrap_mode(settings.wrap_mode_y));

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, convert_filtering_mode(settings.filtering_min, settings.filtering_mipmap));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, convert_filtering_mode(settings.filtering_max, settings.filtering_mipmap));

    glBindTexture(GL_TEXTURE_2D, 0);
    
    stbi_image_free(data);
    return { texture_id, static_cast<u32>(width), static_cast<u32>(height), static_cast<u32>(number_of_components) };
}

TextureData TextureLoaderGL::cubemap_from_files(std::vector<std::string> const& paths, TextureSettings const settings)
{
    u32 texture_id;
    glGenTextures(1, &texture_id);

    stbi_set_flip_vertically_on_load(settings.flip_vertically);

    glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);

    i32 width = 0;
    i32 height = 0;
    i32 channel_count = 0;

    for (u32 i = 0; i < paths.size(); ++i)
    {
        u8* data = stbi_load(paths[i].c_str(), &width, &height, &channel_count, 0);

        if (data != nullptr)
        {
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0,
                GL_RGB,
                width,
                height,
                0,
                GL_RGB,
                GL_UNSIGNED_BYTE,
                data
            );
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << paths[i] << "\n";
        }

        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, convert_filtering_mode(settings.filtering_min, settings.filtering_mipmap));
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, convert_filtering_mode(settings.filtering_max, settings.filtering_mipmap));
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, convert_wrap_mode(settings.wrap_mode_x));
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, convert_wrap_mode(settings.wrap_mode_y));
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, convert_wrap_mode(settings.wrap_mode_z));

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return { texture_id, static_cast<u32>(width), static_cast<u32>(height), static_cast<u32>(channel_count) };
}

TextureData TextureLoaderGL::cubemap_from_file(std::string const& path, TextureSettings const settings)
{
    return {};
}

GLint TextureLoaderGL::convert_wrap_mode(TextureWrapMode const wrap_mode)
{
    switch (wrap_mode)
    {
    case TextureWrapMode::Repeat:
        return GL_REPEAT;
    case TextureWrapMode::ClampToEdge:
        return GL_CLAMP_TO_EDGE;
    case TextureWrapMode::ClampToBorder:
        return GL_CLAMP_TO_BORDER;
    case TextureWrapMode::MirroredRepeat:
        return GL_MIRRORED_REPEAT;
    default:
        std::unreachable();
    }
}

GLint TextureLoaderGL::convert_filtering_mode(TextureFiltering const texture_filtering, TextureFiltering const mipmap_filtering)
{
    switch (texture_filtering)
    {
    case TextureFiltering::Linear:
        if (mipmap_filtering == TextureFiltering::None)
        {
            return GL_LINEAR;
        }

        if (mipmap_filtering == TextureFiltering::Nearest)
        {
            return GL_LINEAR_MIPMAP_NEAREST;
        }

        if (mipmap_filtering == TextureFiltering::Linear)
        {
            return GL_LINEAR_MIPMAP_LINEAR;
        }

        std::unreachable();

    case TextureFiltering::Nearest:
        if (mipmap_filtering == TextureFiltering::None)
        {
            return GL_NEAREST;
        }

        if (mipmap_filtering == TextureFiltering::Nearest)
        {
            return GL_NEAREST_MIPMAP_NEAREST;
        }

        if (mipmap_filtering == TextureFiltering::Linear)
        {
            return  GL_NEAREST_MIPMAP_LINEAR;
        }

        std::unreachable();

    case TextureFiltering::None:
    default:
        std::unreachable();
    }
}
