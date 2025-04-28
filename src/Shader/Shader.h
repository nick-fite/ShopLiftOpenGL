#include <string>
#include <gl/glew.h>
#include <cassert>

struct ShaderProgram
{
    unsigned int id;
    static ShaderProgram FromBuffers(std::string_view vs, std::string_view fs)
    {
        unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
        const char* const vsPtr = vs.data();
        glShaderSource(vertex, 1, &vsPtr, nullptr);
        glCompileShader(vertex);
        {
            int success = -1;
            glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
            assert(success != 0);
        }

        unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
        assert(fragment != 0);
        const GLint psLength = GLint(fs.size());
        const char* const fsPtr = fs.data();
        glShaderSource(fragment, 1, &fsPtr, &psLength);
        glCompileShader(fragment);
        {
            int success = -1;
            glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
            assert(success != 0);
        }

        unsigned int ID = glCreateProgram();
        assert(ID != 0);
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        {
            int success = -1;
            glGetProgramiv(ID, GL_LINK_STATUS, &success);
            assert(success != 0);
        }

        glDeleteShader(vertex);
        glDeleteShader(fragment);

        return ShaderProgram(ID);
    }
    ~ShaderProgram() noexcept {
        if(std::exchange(id, 0) != 0)
        {
            glDeleteProgram(id);
        }

    }
    ShaderProgram(ShaderProgram&& other) noexcept
        : id(std::exchange(other.id, 0)) {}
    ShaderProgram& operator=(ShaderProgram&&) = delete;
    ShaderProgram(const ShaderProgram&) = delete;
    ShaderProgram operator=(const ShaderProgram&) = delete;

private:
    explicit ShaderProgram(unsigned int id) : id(id) {}
};