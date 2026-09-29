#include "matrix4.hh"
#include <math.h>

mygl::matrix4::matrix4()
{
    mat = std::vector<GLfloat>();
    mat.resize(16);
    for (int i = 0; i < 16; ++i)
        mat[i] = 0;
}

void mygl::matrix4::operator*=(const matrix4& rhs)
{
    auto mati = mygl::matrix4();

    for (short i = 0; i < 4; ++i)
    {
        for (short j = 0; j < 4; ++j)
        {
            for (short k = 0; k < 4; ++k)
            {
                mati.mat[i * 4 + j] += mat[i * 4 + k] * rhs.mat[k * 4 + j];
            }
        }
    }
    for (short i = 0; i < 16; ++i)
        mat[i] = mati.mat[i];
}

mygl::matrix4 mygl::matrix4::identity()
{
    auto mati = mygl::matrix4();
    mati.mat[0] = mati.mat[5] = mati.mat[10] = mati.mat[15] = 1;
    return mati;
}

std::ostream& operator<<(std::ostream& out, const mygl::matrix4& m)
{
    out << "---------------\n";
    for (int i = 0; i < 16; i += 4)
        out << "[ " << m.mat[i] << " " << m.mat[i + 1] << " " << m.mat[i + 2]
            << " " << m.mat[i + 3] << " ]\n";
    return out;
}

void mygl::matrix4::frustrum(const GLfloat& left, const GLfloat& right,
                             const GLfloat& bottom, const GLfloat& top,
                             const GLfloat& nearVal, const GLfloat& farVal)
{
    const GLfloat& A = (right + left) / (right - left);
    const GLfloat& B = (top + bottom) / (top - bottom);
    const GLfloat& C = -(farVal + nearVal) / (farVal - nearVal);
    const GLfloat& D = -(2 * farVal * nearVal) / (farVal - nearVal);
    auto mati = mygl::matrix4();
    mati.mat[0] = (2 * nearVal) / (right - left);
    mati.mat[2] = A;
    mati.mat[5] = (2 * nearVal) / (top - bottom);
    mati.mat[6] = B;
    mati.mat[10] = C;
    mati.mat[11] = D;
    mati.mat[14] = -1;
    mati.swap();
    *this *= mati;
}

std::vector<GLfloat> mygl::cross(const std::vector<GLfloat> lhs,
                                 const std::vector<GLfloat> rhs)
{
    auto res = std::vector<GLfloat>();
    res.push_back(lhs[Y_] * rhs[Z_] - lhs[Z_] * rhs[Y_]);
    res.push_back(lhs[Z_] * rhs[X_] - lhs[X_] * rhs[Z_]);
    res.push_back(lhs[X_] * rhs[Y_] - lhs[Y_] * rhs[X_]);
    return res;
}

void mygl::normalize(std::vector<GLfloat>& vec)
{
    GLfloat norm =
        std::sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
    vec[0] /= norm;
    vec[1] /= norm;
    vec[2] /= norm;
}

void mygl::matrix4::lookAt(const GLfloat& eyeX, const GLfloat& eyeY,
                           const GLfloat& eyeZ, const GLfloat& centerX,
                           const GLfloat& centerY, const GLfloat& centerZ,
                           const GLfloat& upX, const GLfloat& upY,
                           const GLfloat& upZ)
{
    auto f = std::vector<GLfloat>();
    f.push_back(centerX - eyeX);
    f.push_back(centerY - eyeY);
    f.push_back(centerZ - eyeZ);
    // normalize
    normalize(f);
    // correcting up
    auto upi = std::vector<GLfloat>();
    upi.push_back(upX);
    upi.push_back(upY);
    upi.push_back(upZ);
    normalize(upi);
    auto s = cross(f, upi);
    auto up = cross(s, f);

    normalize(s);
    normalize(up);
    auto O = mygl::matrix4::identity();
    // M
    mat[0] = s[X_];
    mat[1] = s[Y_];
    mat[2] = s[Z_];
    mat[4] = up[X_];
    mat[5] = up[Y_];
    mat[6] = up[Z_];
    mat[8] = -f[X_];
    mat[9] = -f[Y_];
    mat[10] = -f[Z_];
    mat[15] = 1;
    swap();

    // O
    O.mat[3] = -eyeX;
    O.mat[7] = -eyeY;
    O.mat[11] = -eyeZ;
    O.swap();

    *this *= O;
}

void mygl::matrix4::swap()
{
    for (short i = 0; i < 4; i++)
    {
        for (short j = 0; j < i; j++)
        {
            GLfloat tmp = mat[i * 4 + j];
            mat[i * 4 + j] = mat[j * 4 + i];
            mat[j * 4 + i] = tmp;
        }
    }
}

const GLfloat* mygl::matrix4::to_uniform()
{
    GLfloat* mati = new GLfloat[16];
    for (short i = 0; i < 4; i++)
    {
        for (short j = 0; j < 4; j++)
        {
            mati[i * 4 + j] = mat[i * 4 + j];
        }
    }
    return mati;
}
