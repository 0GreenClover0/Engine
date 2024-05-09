#include "SkyboxGL.h"

#include <glad/glad.h>

#include "Globals.h"

SkyboxGL::SkyboxGL(AK::Badge<SkyboxFactory>, std::shared_ptr<Material> const& material, std::vector<std::string> const& face_paths)
    : Skybox(material, face_paths)
{
    create_cube();
}

void SkyboxGL::bind()
{
    bind_texture();
}

void SkyboxGL::unbind()
{
    unbind_texture();
}

void SkyboxGL::draw() const
{
    if (m_texture->id == 0)
        return;

    GLint depth_func_previous_value;
    glGetIntegerv(GL_DEPTH_FUNC, &depth_func_previous_value);

    glDepthFunc(GL_LEQUAL);

    glBindVertexArray(m_VAO);

    glActiveTexture(GL_TEXTURE0);
    material->shader->set_int("skybox", 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_texture->id);

    // Draw mesh
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glActiveTexture(GL_TEXTURE0);

    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);

    glDepthFunc(depth_func_previous_value);
}

void SkyboxGL::bind_texture() const
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_texture->id);
}

void SkyboxGL::unbind_texture() const
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void SkyboxGL::create_cube()
{
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);

    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(InternalMeshData::skybox_vertices), &InternalMeshData::skybox_vertices, GL_STATIC_DRAW);

    // Vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
