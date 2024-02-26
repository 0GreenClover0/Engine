#include "Skybox.h"

#include <iostream>
#include <utility>
#include <glad/glad.h>

#include "Globals.h"
#include "TextureLoader.h"

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
    TextureSettings constexpr texture_settings =
    {
        TextureWrapMode::ClampToEdge,
        TextureWrapMode::ClampToEdge,
        TextureWrapMode::ClampToEdge,
        TextureFiltering::Linear,
        TextureFiltering::Linear,
        false,
        false
    };

    Texture const texture = TextureLoader::get_instance()->load_cubemap(face_paths, TextureType::None, texture_settings);
    this->texture_id = texture.id;
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
