#include "Render.h"
RenderMesh::~RenderMesh()
{
    if(std::exchange(VAO, 0) != 0)
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }
}

void RenderMesh::draw(TexturesDB& textures, int diffusePtr, int normalPtr, int debugFlagPtr)
{
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(diffusePtr, 0);
    glBindTexture(GL_TEXTURE_2D, textures.get(diffuse));
    glActiveTexture(GL_TEXTURE1);
    glUniform1i(normalPtr, 1);
    glBindTexture(GL_TEXTURE_2D, textures.get(normal));

    glm::vec3 debugFlag(0.f);
    if(normal < 0)
    {
        debugFlag.x = 1.f;
    }
    glUniform3fv(debugFlagPtr, 1, glm::value_ptr(debugFlag));

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, GLsizei(indicesCount), GL_UNSIGNED_INT, 0);
}