#include "Skybox.h"

#include <iostream>
#include <stb_image.h>
#include <utility>
#include <glad/glad.h>

#include "Globals.h"

Skybox::Skybox(std::shared_ptr<Material> const& material, std::vector<std::string> face_paths) : Drawable(material), face_paths(std::move(face_paths))
{
    load_textures();
    create_cube();
}

std::string Skybox::get_name() const
{
    std::string const name = typeid(decltype(*this)).name();
    return name.substr(6);
}

void Skybox::draw() const
{
    if (texture_id == 0)
        return;

    GLint depth_func_previous_value;
    glGetIntegerv(GL_DEPTH_FUNC, &depth_func_previous_value);

    glDepthFunc(GL_LEQUAL);

    glBindVertexArray(VAO);

    glActiveTexture(GL_TEXTURE0);
    material->shader->set_int("skybox", 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);

    // Draw mesh
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glActiveTexture(GL_TEXTURE0);

    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);

    glDepthFunc(depth_func_previous_value);
}

uint32_t Skybox::get_texture_id() const
{
    return texture_id;
}

void Skybox::bind() const
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, instance->texture_id);
}

void Skybox::create_cube()
{
    setup_mesh();
}

void Skybox::load_textures()
{
    stbi_set_flip_vertically_on_load(false);
    uint32_t texture_id = 0;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);

    int32_t width = 0;
    int32_t height = 0;
    int32_t channel_count = 0;

    for (uint32_t i = 0; i < face_paths.size(); ++i)
    {
        uint8_t* data = stbi_load(face_paths[i].c_str(), &width, &height, &channel_count, 0);

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
            std::cout << "Cubemap texture failed to load at path: " << face_paths[i] << "\n";
        }

        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    this->texture_id = texture_id;
    stbi_set_flip_vertically_on_load(true);
}

void Skybox::setup_mesh()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(InternalMeshData::skybox_vertices), &InternalMeshData::skybox_vertices, GL_STATIC_DRAW);

    // Vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
