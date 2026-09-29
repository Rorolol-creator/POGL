#pragma once
#define X_ 0
#define Y_ 1
#define Z_ 2

#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <GL/freeglut.h>

namespace mygl
{
    class matrix4
    {
    public:
        matrix4();
        void operator*=(const matrix4& rhs);
        static matrix4 identity();
        std::vector<GLfloat> mat;
        void frustrum(const GLfloat& left, const GLfloat& right,
                      const GLfloat& bottom, const GLfloat& top,
                      const GLfloat& nearVal, const GLfloat& farVal);
        void lookAt(const GLfloat& eyeX, const GLfloat& eyeY,
                    const GLfloat& eyeZ, const GLfloat& centerX,
                    const GLfloat& centerY, const GLfloat& centerZ,
                    const GLfloat& upX, const GLfloat& upY, const GLfloat& upZ);
        const GLfloat* to_uniform();

    private:
        void swap();
    };

    std::vector<GLfloat> cross(std::vector<GLfloat> lhs,
                               std::vector<GLfloat> rhs);
    void normalize(std::vector<GLfloat>& vec);
}; // namespace mygl

std::ostream& operator<<(std::ostream& out, const mygl::matrix4& m);
