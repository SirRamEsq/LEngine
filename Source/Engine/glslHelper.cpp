#include "glslHelper.h"
#include "math.h"

////////
//Vec2//
////////

Vec2::Vec2 () {x=0.0f; y=0.0f;}
Vec2::Vec2 (float xVal, float yVal) : x(xVal), y(yVal) {}


////////
//Vec3//
////////

Vec3::Vec3 () {x=0.0f; y=0.0f; z=0.0f;}
Vec3::Vec3 (float xVal, float yVal, float zVal)  : x(xVal), y(yVal), z(zVal) {}
Vec3::Vec3 (const Vec2& vv, float zVal) {
	x = vv.x;
	y = vv.y;
	z = zVal;
}

Vec3::Vec3 (const Vec4& vv) {
	x = vv.x;
	y = vv.y;
	z = vv.z;
}

float Vec3::GetMagnitude () {
	return sqrt ( (x * x) + (y * y) + (z * z) );
}

Vec3 Vec3::GetNormalizedVector () {
	float mag = GetMagnitude();
	if (0.0f == mag) {
		return Vec3 (0.0f, 0.0f, 0.0f);
	}
	Vec3 vb;
	vb.x = x / mag;
	vb.y = y / mag;
	vb.z = z / mag;

	return vb;
}

float Vec3::DotProduct(const Vec3& vec){
    return (x * vec.x) + (y * vec.y) + (z * vec.z);
}

Vec3 Vec3::CrossProduct(const Vec3& vec){
    float x = y * vec.z - z * vec.y;
	float y = z * vec.x - x * vec.z;
	float z = x * vec.y - y * vec.x;
	return Vec3 (x, y, z);
}

float Vec3::SquaredDistance(Vec3 to) {
	float x = (to.x - x) * (to.x - x);
	float y = (to.y - y) * (to.y - y);
	float z = (to.z - z) * (to.z - z);
	return x + y + z;
}

Vec3 Vec3::operator+ (const Vec3& rhs) {
	Vec3 vc;
	vc.x = x + rhs.x;
	vc.y = y + rhs.y;
	vc.z = z + rhs.z;
	return vc;
}

Vec3& Vec3::operator+= (const Vec3& rhs) {
	x += rhs.x;
	y += rhs.y;
	z += rhs.z;
	return *this; // return self
}

Vec3 Vec3::operator- (const Vec3& rhs) {
	Vec3 vc;
	vc.x = x - rhs.x;
	vc.y = y - rhs.y;
	vc.z = z - rhs.z;
	return vc;
}

Vec3& Vec3::operator-= (const Vec3& rhs) {
	x -= rhs.x;
	y -= rhs.y;
	z -= rhs.z;
	return *this;
}

Vec3 Vec3::operator+ (float rhs) {
	Vec3 vc;
	vc.x = x + rhs;
	vc.y = y + rhs;
	vc.z = z + rhs;
	return vc;
}

Vec3 Vec3::operator- (float rhs) {
	Vec3 vc;
	vc.x = x - rhs;
	vc.y = y - rhs;
	vc.z = z - rhs;
	return vc;
}

Vec3 Vec3::operator* (float rhs) {
	Vec3 vc;
	vc.x = x * rhs;
	vc.y = y * rhs;
	vc.z = z * rhs;
	return vc;
}

Vec3 Vec3::operator/ (float rhs) {
	Vec3 vc;
	vc.x = x / rhs;
	vc.y = y / rhs;
	vc.z = z / rhs;
	return vc;
}

Vec3& Vec3::operator*= (float rhs) {
	x = x * rhs;
	y = y * rhs;
	z = z * rhs;
	return *this;
}

Vec3& Vec3::operator= (const Vec3& rhs) {
	x = rhs.x;
	y = rhs.y;
	z = rhs.z;
	return *this;
}


////////
//Vec4//
////////

Vec4::Vec4 () {}

Vec4::Vec4 (float xVal, float yVal, float zVal, float wVal) {
	x = xVal;
	y = yVal;
	z = zVal;
	w = wVal;
}

Vec4::Vec4 (const Vec2& vv, float zVal, float wVal) {
	x = vv.x;
	y = vv.y;
	z = z;
	w = w;
}

Vec4::Vec4 (const Vec3& vv, float wVal) {
	x = vv.x;
	y = vv.y;
	z = vv.z;
	w = wVal;
}


///////////
//Matrix4//
///////////

Matrix4::Matrix4 () {}

/* note: entered in COLUMNS */
Matrix4::Matrix4 (float a, float b, float c, float d,
						float e, float f, float g, float h,
						float i, float j, float k, float l,
						float mm, float n, float o, float p) {
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

Matrix4 Matrix4::ZeroMatrix () {
	return Matrix4 (
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f
	);
}

Matrix4 Matrix4::IdentityMatrix() {
	return Matrix4 (
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
}

Matrix4 Matrix4::OrthoGraphicProjectionMatrix(const Coord2df& displaySize){
	return Matrix4 (
        2.0f/displaySize.x,		0.0f,                   0.0f, 0.0f ,
        0.0f,                 	2.0f/-displaySize.y, 	0.0f, 0.0f ,
        0.0f,                 	0.0f,                  -1.0f, 0.0f ,
        -1.0f,                 	1.0f,                   0.0f, 1.0f
	);
}

Vec4 Matrix4::operator* (const Vec4& rhs) {
	// 0x + 4y + 8z + 12w
	float x =
		m[0] * rhs.x +
		m[4] * rhs.y +
		m[8] * rhs.z +
		m[12] * rhs.w;
	// 1x + 5y + 9z + 13w
	float y = m[1] * rhs.x +
		m[5] * rhs.y +
		m[9] * rhs.z +
		m[13] * rhs.w;
	// 2x + 6y + 10z + 14w
	float z = m[2] * rhs.x +
		m[6] * rhs.y +
		m[10] * rhs.z +
		m[14] * rhs.w;
	// 3x + 7y + 11z + 15w
	float w = m[3] * rhs.x +
		m[7] * rhs.y +
		m[11] * rhs.z +
		m[15] * rhs.w;
	return Vec4 (x, y, z, w);
}

Matrix4 Matrix4::operator* (const Matrix4& rhs) {
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

Matrix4& Matrix4::operator= (const Matrix4& rhs) {
	for (int i = 0; i < 16; i++) {
		m[i] = rhs.m[i];
	}
	return *this;
}

// returns a scalar value with the determinant for a 4x4 matrix
// see http://www.euclideanspace.com/maths/algebra/matrix/functions/determinant/fourD/index.htm
float Matrix4::Determinant() {
	return
		m[12] * m[ 9] * m[ 6] * m[ 3] -
		m[ 8] * m[13] * m[ 6] * m[ 3] -
		m[12] * m[ 5] * m[10] * m[ 3] +
		m[ 4] * m[13] * m[10] * m[ 3] +
		m[ 8] * m[ 5] * m[14] * m[ 3] -
		m[ 4] * m[ 9] * m[14] * m[ 3] -
		m[12] * m[ 9] * m[ 2] * m[ 7] +
		m[ 8] * m[13] * m[ 2] * m[ 7] +
		m[12] * m[ 1] * m[10] * m[ 7] -
		m[ 0] * m[13] * m[10] * m[ 7] -
		m[ 8] * m[ 1] * m[14] * m[ 7] +
		m[ 0] * m[ 9] * m[14] * m[ 7] +
		m[12] * m[ 5] * m[ 2] * m[11] -
		m[ 4] * m[13] * m[ 2] * m[11] -
		m[12] * m[ 1] * m[ 6] * m[11] +
		m[ 0] * m[13] * m[ 6] * m[11] +
		m[ 4] * m[ 1] * m[14] * m[11] -
		m[ 0] * m[ 5] * m[14] * m[11] -
		m[ 8] * m[ 5] * m[ 2] * m[15] +
		m[ 4] * m[ 9] * m[ 2] * m[15] +
		m[ 8] * m[ 1] * m[ 6] * m[15] -
		m[ 0] * m[ 9] * m[ 6] * m[15] -
		m[ 4] * m[ 1] * m[10] * m[15] +
		m[ 0] * m[ 5] * m[10] * m[15];
}

/* returns a 16-element array that is the inverse of a 16-element array (4x4
matrix). see http://www.euclideanspace.com/maths/algebra/matrix/functions/inverse/fourD/index.htm */
Matrix4 Matrix4::Inverse () {
	float det = Determinant();
	/* there is no inverse if determinant is zero (not likely unless scale is
	broken) */
	if (0.0f == det) {
		//"WARNING. matrix has no determinant. can not invert
		return IdentityMatrix();
	}
	float inv_det = 1.0f / det;

	return Matrix4 (
		inv_det * (
			m[9] * m[14] * m[7] - m[13] * m[10] * m[7] +
			m[13] * m[6] * m[11] - m[5] * m[14] * m[11] -
			m[9] * m[6] * m[15] + m[5] * m[10] * m[15]
		),
		inv_det * (
			m[13] * m[10] * m[3] - m[9] * m[14] * m[3] -
			m[13] * m[2] * m[11] + m[1] * m[14] * m[11] +
			m[9] * m[2] * m[15] - m[1] * m[10] * m[15]
		),
		inv_det * (
			m[5] * m[14] * m[3] - m[13] * m[6] * m[3] +
			m[13] * m[2] * m[7] - m[1] * m[14] * m[7] -
			m[5] * m[2] * m[15] + m[1] * m[6] * m[15]
		),
		inv_det * (
			m[9] * m[6] * m[3] - m[5] * m[10] * m[3] -
			m[9] * m[2] * m[7] + m[1] * m[10] * m[7] +
			m[5] * m[2] * m[11] - m[1] * m[6] * m[11]
		),
		inv_det * (
			m[12] * m[10] * m[7] - m[8] * m[14] * m[7] -
			m[12] * m[6] * m[11] + m[4] * m[14] * m[11] +
			m[8] * m[6] * m[15] - m[4] * m[10] * m[15]
		),
		inv_det * (
			m[8] * m[14] * m[3] - m[12] * m[10] * m[3] +
			m[12] * m[2] * m[11] - m[0] * m[14] * m[11] -
			m[8] * m[2] * m[15] + m[0] * m[10] * m[15]
		),
		inv_det * (
			m[12] * m[6] * m[3] - m[4] * m[14] * m[3] -
			m[12] * m[2] * m[7] + m[0] * m[14] * m[7] +
			m[4] * m[2] * m[15] - m[0] * m[6] * m[15]
		),
		inv_det * (
			m[4] * m[10] * m[3] - m[8] * m[6] * m[3] +
			m[8] * m[2] * m[7] - m[0] * m[10] * m[7] -
			m[4] * m[2] * m[11] + m[0] * m[6] * m[11]
		),
		inv_det * (
			m[8] * m[13] * m[7] - m[12] * m[9] * m[7] +
			m[12] * m[5] * m[11] - m[4] * m[13] * m[11] -
			m[8] * m[5] * m[15] + m[4] * m[9] * m[15]
		),
		inv_det * (
			m[12] * m[9] * m[3] - m[8] * m[13] * m[3] -
			m[12] * m[1] * m[11] + m[0] * m[13] * m[11] +
			m[8] * m[1] * m[15] - m[0] * m[9] * m[15]
		),
		inv_det * (
			m[4] * m[13] * m[3] - m[12] * m[5] * m[3] +
			m[12] * m[1] * m[7] - m[0] * m[13] * m[7] -
			m[4] * m[1] * m[15] + m[0] * m[5] * m[15]
		),
		inv_det * (
			m[8] * m[5] * m[3] - m[4] * m[9] * m[3] -
			m[8] * m[1] * m[7] + m[0] * m[9] * m[7] +
			m[4] * m[1] * m[11] - m[0] * m[5] * m[11]
		),
		inv_det * (
			m[12] * m[9] * m[6] - m[8] * m[13] * m[6] -
			m[12] * m[5] * m[10] + m[4] * m[13] * m[10] +
			m[8] * m[5] * m[14] - m[4] * m[9] * m[14]
		),
		inv_det * (
			m[8] * m[13] * m[2] - m[12] * m[9] * m[2] +
			m[12] * m[1] * m[10] - m[0] * m[13] * m[10] -
			m[8] * m[1] * m[14] + m[0] * m[9] * m[14]
		),
		inv_det * (
			m[12] * m[5] * m[2] - m[4] * m[13] * m[2] -
			m[12] * m[1] * m[6] + m[0] * m[13] * m[6] +
			m[4] * m[1] * m[14] - m[0] * m[5] * m[14]
		),
		inv_det * (
			m[4] * m[9] * m[2] - m[8] * m[5] * m[2] +
			m[8] * m[1] * m[6] - m[0] * m[9] * m[6] -
			m[4] * m[1] * m[10] + m[0] * m[5] * m[10]
		)
	);
}

// returns a 16-element array flipped on the main diagonal
Matrix4 Matrix4::Transpose() {
	return Matrix4 (
		m[0], m[4], m[8], m[12],
		m[1], m[5], m[9], m[13],
		m[2], m[6], m[10], m[14],
		m[3], m[7], m[11], m[15]
	);
}

// translate a 4d matrix with xyz array
Matrix4 Matrix4::Translate (const Vec3& v) {
	Matrix4 m_t = Matrix4::IdentityMatrix();
	m_t.m[12] = v.x;
	m_t.m[13] = v.y;
	m_t.m[14] = v.z;
	return m_t * (*this);
}

// rotate around z axis by an angle in degrees
Matrix4 Matrix4::RotateZ(const float& deg) {
	// convert to radians
	float rad = deg * ONE_DEG_IN_RAD;
	Matrix4 m_r = Matrix4::IdentityMatrix();
	m_r.m[0] = cos (rad);
	m_r.m[4] = -sin (rad);
	m_r.m[1] = sin (rad);
	m_r.m[5] = cos (rad);
	return m_r * (*this);
}

Matrix4 Matrix4::Scale (const Vec3& v) {
	Matrix4 a = Matrix4::IdentityMatrix();
	a.m[0] = v.x;
	a.m[5] = v.y;
	a.m[10] = v.z;
	return a * (*this);
}
