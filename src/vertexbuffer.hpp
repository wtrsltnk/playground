#include <glad/glad.h>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>

template <class TVertex>
class VertextBuffer
{
public:
    template <
        class TAttr0, unsigned int NAttr0>
    bool setup(
        const char *attrName0,
        GLsizei stride = 0)
    {
        bind();

        if (stride == 0)
        {
            ensureAttrArrays(1);

            _stride = stride = sizeof(TAttr0) * NAttr0;
        }

        return setup<TAttr0, NAttr0>(attrName0, 0, stride, 0);
    }

    template <
        class TAttr0, unsigned int NAttr0,
        class TAttr1, unsigned int NAttr1>
    bool setup(
        const char *attrName0,
        const char *attrName1,
        GLsizei stride = 0)
    {
        GLuint offset = (sizeof(TAttr0) * NAttr0); // Attribute 0

        if (stride == 0)
        {
            ensureAttrArrays(2);
            _stride = stride = offset + (sizeof(TAttr1) * NAttr1);
        }

        setup<TAttr0, NAttr0>(attrName0, stride);

        return setup<TAttr1, NAttr1>(attrName1, 1, stride, offset);
    }

    template <
        class TAttr0, unsigned int NAttr0,
        class TAttr1, unsigned int NAttr1,
        class TAttr2, unsigned int NAttr2>
    bool setup(
        const char *attrName0,
        const char *attrName1,
        const char *attrName2,
        GLsizei stride = 0)
    {
        GLuint offset = (sizeof(TAttr0) * NAttr0)    // Attribute 0
                        + (sizeof(TAttr1) * NAttr1); // Attribute 1

        if (stride == 0)
        {
            ensureAttrArrays(3);
            _stride = stride = offset + (sizeof(TAttr2) * NAttr2);
        }

        setup<TAttr0, NAttr0, TAttr1, NAttr1>(attrName0, attrName1, stride);

        return setup<TAttr2, NAttr2>(attrName2, 2, stride, offset);
    }

    template <
        class TAttr0, unsigned int NAttr0,
        class TAttr1, unsigned int NAttr1,
        class TAttr2, unsigned int NAttr2,
        class TAttr3, unsigned int NAttr3>
    bool setup(
        const char *attrName0,
        const char *attrName1,
        const char *attrName2,
        const char *attrName3,
        GLsizei stride = 0)
    {
        GLuint offset = (sizeof(TAttr0) * NAttr0)    // Attribute 0
                        + (sizeof(TAttr1) * NAttr1)  // Attribute 1
                        + (sizeof(TAttr2) * NAttr2); // Attribute 2

        if (stride == 0)
        {
            ensureAttrArrays(4);
            _stride = _stride = stride = offset + (sizeof(TAttr3) * NAttr3);
        }

        setup<TAttr0, NAttr0, TAttr1, NAttr1, TAttr2, NAttr2>(attrName0, attrName1, attrName2, stride);

        return setup<TAttr3, NAttr3>(attrName3, 3, stride, offset);
    }

    template <
        class TAttr0, unsigned int NAttr0,
        class TAttr1, unsigned int NAttr1,
        class TAttr2, unsigned int NAttr2,
        class TAttr3, unsigned int NAttr3,
        class TAttr4, unsigned int NAttr4>
    bool setup(
        const char *attrName0,
        const char *attrName1,
        const char *attrName2,
        const char *attrName3,
        const char *attrName4,
        GLsizei stride = 0)
    {
        GLuint offset = (sizeof(TAttr0) * NAttr0)    // Attribute 0
                        + (sizeof(TAttr1) * NAttr1)  // Attribute 1
                        + (sizeof(TAttr2) * NAttr2)  // Attribute 2
                        + (sizeof(TAttr3) * NAttr3); // Attribute 3

        if (stride == 0)
        {
            ensureAttrArrays(5);
            _stride = stride = offset + (sizeof(TAttr4) * NAttr4);
        }

        setup<TAttr0, NAttr0, TAttr1, NAttr1, TAttr2, NAttr2, TAttr3, NAttr3>(attrName0, attrName1, attrName2, attrName3, stride);

        return setup<TAttr4, NAttr4>(attrName4, 4, stride, offset);
    }

    void bind()
    {
        if (_vao == 0)
        {
            glGenVertexArrays(1, &_vao);
        }

        if (_vbo == 0)
        {
            glGenBuffers(1, &_vbo);
        }

        glBindVertexArray(_vao);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    }

    void upload()
    {
        bind();

        glBufferData(GL_ARRAY_BUFFER, sizeof(TVertex) * _vertices.size(), _vertices.data(), GL_STATIC_DRAW);
    }

    void resize(
        const size_t size)
    {
        _vertices.resize(size);
    }

    size_t add(
        const TVertex &vertex)
    {
        auto result = _vertices.size();

        _vertices.push_back(vertex);

        return result;
    }

private:
    GLuint _vao = 0;
    GLuint _vbo = 0;
    unsigned int _stride = 0;
    std::vector<std::string> _attrNames;
    std::vector<TVertex> _vertices;

    template <
        class TAttr, unsigned int NAttr>
    bool setup(
        const char *name,
        GLuint index,
        GLsizei stride,
        GLuint offset)
    {
        if (!checkStride(stride))
        {
            return false;
        }

        _attrNames[index] = name;

        glVertexAttribPointer(index, NAttr, getDataType<TAttr>(), GL_FALSE, stride, (const void *)static_cast<long long>(offset));
        glEnableVertexAttribArray(index);

        return true;
    }

    void ensureAttrArrays(
        size_t size)
    {
        _attrNames.resize(size);
    }

    bool checkStride(
        unsigned int stride)
    {
        if (stride != sizeof(TVertex))
        {
            spdlog::error("stride does not match: {} vs. {}", stride, sizeof(TVertex));

            return false;
        }
        return true;
    }

    template <class TAttr>
    GLenum getDataType()
    {
        if (sizeof(TAttr) == 1)
        {
            return GL_BYTE;
        }

        if (sizeof(TAttr) == 2)
        {
            return GL_SHORT;
        }

        return GL_FLOAT;
    }
};
