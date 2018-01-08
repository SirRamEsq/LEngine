#include "Matrix.h"
#include "math.h"

///////////
// Matrix4//
///////////

Matrix4::Matrix4() {}

/* note: entered in COLUMNS */
Matrix4::Matrix4(float a, float b, float c, float d, float e, float f, float g,
                 float h, float i, float j, float k, float l, float mm, float n,
                 float o, float p) {
  m[0] = a;
  m[1] = b;
  m[2] = c;
  m[3] = d;
  m[4] = e;
  m[5] = f;
  m[6] = g;
  m[7] = h;
  m[8] = i;
  m[9] = j;
  m[10] = k;
  m[11] = l;
  m[12] = mm;
  m[13] = n;
  m[14] = o;
  m[15] = p;
}

Matrix4 Matrix4::ZeroMatrix() {
  return Matrix4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
}

Matrix4 Matrix4::IdentityMatrix() {
  return Matrix4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
}

Matrix4 Matrix4::OrthoGraphicProjectionMatrix(const Vec2 &displaySize) {
  return Matrix4(2.0f / displaySize.x, 0.0f, 0.0f, 0.0f, 0.0f,
                 2.0f / -displaySize.y, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
                 -1.0f, 1.0f, 0.0f, 1.0f);
}
Matrix4 Matrix4::OrthoGraphicProjectionMatrix(const Vec2 &displaySize,
                                              int near, int far) {
  //auto n1 = -1.0f * (far / float(far - near));
  //auto n2 = -1.0f * ((far * near) / float(far - near));
  auto fnDiff = (float(far - near));
  auto n3 = -2.0f / (fnDiff);
  return Matrix4(2.0f / displaySize.x, 0.0f, 0.0f, 0.0f, 0.0f,
                 2.0f / -displaySize.y, 0.0f, 0.0f, 0.0f, 0.0f, n3, 0.0f, -1.0f,
                 1.0f, 0.0f, 1.0f);
}

Matrix4 Matrix4::OrthoGraphicProjectionMatrix(const Rect &displayArea) {
  float rightSide = displayArea.GetRight();
  float leftSide = displayArea.GetLeft();
  float bottomSide = displayArea.GetBottom();
  float topSide = displayArea.GetTop();
  float normailizedX = 2.0f / (rightSide - leftSide);
  float normailizedY = 2.0f / (topSide - bottomSide);
  return Matrix4(normailizedX, 0.0f, 0.0f, 0.0f, 0.0f, normailizedY, 0.0f, 0.0f,
                 0.0f, 0.0f, -1.0f, 0.0f, -1.0f, 1.0f, 0.0f, 1.0f);
}

Vec4 Matrix4::operator*(const Vec4 &rhs) {
  // 0x + 4y + 8z + 12w
  float x = m[0] * rhs.x + m[4] * rhs.y + m[8] * rhs.z + m[12] * rhs.w;
  // 1x + 5y + 9z + 13w
  float y = m[1] * rhs.x + m[5] * rhs.y + m[9] * rhs.z + m[13] * rhs.w;
  // 2x + 6y + 10z + 14w
  float z = m[2] * rhs.x + m[6] * rhs.y + m[10] * rhs.z + m[14] * rhs.w;
  // 3x + 7y + 11z + 15w
  float w = m[3] * rhs.x + m[7] * rhs.y + m[11] * rhs.z + m[15] * rhs.w;
  return Vec4(x, y, z, w);
}

Matrix4 Matrix4::operator*(const Matrix4 &rhs) {
  Matrix4 r = Matrix4::ZeroMatrix();
  int r_index = 0;
  for (int col = 0; col < 4; col++) {
    for (int row = 0; row < 4; row++) {
      float sum = 0.0f;
      for (int i = 0; i < 4; i++) {
        sum += rhs.m[i + col * 4] * m[row + i * 4];
      }
      r.m[r_index] = sum;
      r_index++;
    }
  }
  return r;
}

Matrix4 &Matrix4::operator=(const Matrix4 &rhs) {
  for (int i = 0; i < 16; i++) {
    m[i] = rhs.m[i];
  }
  return *this;
}

// returns a scalar value with the determinant for a 4x4 matrix
// see
// http://www.euclideanspace.com/maths/algebra/matrix/functions/determinant/fourD/index.htm
float Matrix4::Determinant() {
  return m[12] * m[9] * m[6] * m[3] - m[8] * m[13] * m[6] * m[3] -
         m[12] * m[5] * m[10] * m[3] + m[4] * m[13] * m[10] * m[3] +
         m[8] * m[5] * m[14] * m[3] - m[4] * m[9] * m[14] * m[3] -
         m[12] * m[9] * m[2] * m[7] + m[8] * m[13] * m[2] * m[7] +
         m[12] * m[1] * m[10] * m[7] - m[0] * m[13] * m[10] * m[7] -
         m[8] * m[1] * m[14] * m[7] + m[0] * m[9] * m[14] * m[7] +
         m[12] * m[5] * m[2] * m[11] - m[4] * m[13] * m[2] * m[11] -
         m[12] * m[1] * m[6] * m[11] + m[0] * m[13] * m[6] * m[11] +
         m[4] * m[1] * m[14] * m[11] - m[0] * m[5] * m[14] * m[11] -
         m[8] * m[5] * m[2] * m[15] + m[4] * m[9] * m[2] * m[15] +
         m[8] * m[1] * m[6] * m[15] - m[0] * m[9] * m[6] * m[15] -
         m[4] * m[1] * m[10] * m[15] + m[0] * m[5] * m[10] * m[15];
}

/* returns a 16-element array that is the inverse of a 16-element array (4x4
matrix). see
http://www.euclideanspace.com/maths/algebra/matrix/functions/inverse/fourD/index.htm
*/
Matrix4 Matrix4::Inverse() {
  float det = Determinant();
  /* there is no inverse if determinant is zero (not likely unless scale is
  broken) */
  if (0.0f == det) {
    //"WARNING. matrix has no determinant. can not invert
    return IdentityMatrix();
  }
  float inv_det = 1.0f / det;

  return Matrix4(
      inv_det *
          (m[9] * m[14] * m[7] - m[13] * m[10] * m[7] + m[13] * m[6] * m[11] -
           m[5] * m[14] * m[11] - m[9] * m[6] * m[15] + m[5] * m[10] * m[15]),
      inv_det *
          (m[13] * m[10] * m[3] - m[9] * m[14] * m[3] - m[13] * m[2] * m[11] +
           m[1] * m[14] * m[11] + m[9] * m[2] * m[15] - m[1] * m[10] * m[15]),
      inv_det *
          (m[5] * m[14] * m[3] - m[13] * m[6] * m[3] + m[13] * m[2] * m[7] -
           m[1] * m[14] * m[7] - m[5] * m[2] * m[15] + m[1] * m[6] * m[15]),
      inv_det *
          (m[9] * m[6] * m[3] - m[5] * m[10] * m[3] - m[9] * m[2] * m[7] +
           m[1] * m[10] * m[7] + m[5] * m[2] * m[11] - m[1] * m[6] * m[11]),
      inv_det *
          (m[12] * m[10] * m[7] - m[8] * m[14] * m[7] - m[12] * m[6] * m[11] +
           m[4] * m[14] * m[11] + m[8] * m[6] * m[15] - m[4] * m[10] * m[15]),
      inv_det *
          (m[8] * m[14] * m[3] - m[12] * m[10] * m[3] + m[12] * m[2] * m[11] -
           m[0] * m[14] * m[11] - m[8] * m[2] * m[15] + m[0] * m[10] * m[15]),
      inv_det *
          (m[12] * m[6] * m[3] - m[4] * m[14] * m[3] - m[12] * m[2] * m[7] +
           m[0] * m[14] * m[7] + m[4] * m[2] * m[15] - m[0] * m[6] * m[15]),
      inv_det *
          (m[4] * m[10] * m[3] - m[8] * m[6] * m[3] + m[8] * m[2] * m[7] -
           m[0] * m[10] * m[7] - m[4] * m[2] * m[11] + m[0] * m[6] * m[11]),
      inv_det *
          (m[8] * m[13] * m[7] - m[12] * m[9] * m[7] + m[12] * m[5] * m[11] -
           m[4] * m[13] * m[11] - m[8] * m[5] * m[15] + m[4] * m[9] * m[15]),
      inv_det *
          (m[12] * m[9] * m[3] - m[8] * m[13] * m[3] - m[12] * m[1] * m[11] +
           m[0] * m[13] * m[11] + m[8] * m[1] * m[15] - m[0] * m[9] * m[15]),
      inv_det *
          (m[4] * m[13] * m[3] - m[12] * m[5] * m[3] + m[12] * m[1] * m[7] -
           m[0] * m[13] * m[7] - m[4] * m[1] * m[15] + m[0] * m[5] * m[15]),
      inv_det *
          (m[8] * m[5] * m[3] - m[4] * m[9] * m[3] - m[8] * m[1] * m[7] +
           m[0] * m[9] * m[7] + m[4] * m[1] * m[11] - m[0] * m[5] * m[11]),
      inv_det *
          (m[12] * m[9] * m[6] - m[8] * m[13] * m[6] - m[12] * m[5] * m[10] +
           m[4] * m[13] * m[10] + m[8] * m[5] * m[14] - m[4] * m[9] * m[14]),
      inv_det *
          (m[8] * m[13] * m[2] - m[12] * m[9] * m[2] + m[12] * m[1] * m[10] -
           m[0] * m[13] * m[10] - m[8] * m[1] * m[14] + m[0] * m[9] * m[14]),
      inv_det *
          (m[12] * m[5] * m[2] - m[4] * m[13] * m[2] - m[12] * m[1] * m[6] +
           m[0] * m[13] * m[6] + m[4] * m[1] * m[14] - m[0] * m[5] * m[14]),
      inv_det *
          (m[4] * m[9] * m[2] - m[8] * m[5] * m[2] + m[8] * m[1] * m[6] -
           m[0] * m[9] * m[6] - m[4] * m[1] * m[10] + m[0] * m[5] * m[10]));
}

// returns a 16-element array flipped on the main diagonal
Matrix4 Matrix4::Transpose() {
  return Matrix4(m[0], m[4], m[8], m[12], m[1], m[5], m[9], m[13], m[2], m[6],
                 m[10], m[14], m[3], m[7], m[11], m[15]);
}

// translate a 4d matrix with xyz array
Matrix4 Matrix4::Translate(const Vec3 &v) {
  Matrix4 m_t = Matrix4::IdentityMatrix();
  m_t.m[12] = v.x;
  m_t.m[13] = v.y;
  m_t.m[14] = v.z;
  return m_t * (*this);
}

// rotate around z axis by an angle in degrees
Matrix4 Matrix4::RotateZ(const float &deg) {
  // convert to radians
  float rad = deg * ONE_DEG_IN_RAD;
  Matrix4 m_r = Matrix4::IdentityMatrix();
  m_r.m[0] = cos(rad);
  m_r.m[4] = -sin(rad);
  m_r.m[1] = sin(rad);
  m_r.m[5] = cos(rad);
  return m_r * (*this);
}

Matrix4 Matrix4::Scale(const Vec3 &v) {
  Matrix4 a = Matrix4::IdentityMatrix();
  a.m[0] = v.x;
  a.m[5] = v.y;
  a.m[10] = v.z;
  return a * (*this);
}
