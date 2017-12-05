#include "VAOWrapper.h"

// Each vertex point consists of 2 floats                                [X,Y]
GLint vertexAttributeSize = 2;
GLenum vertexAttributeType = GL_FLOAT;

// Each texture coordinate for each vertex point consists of 2 floats    [S,T]
GLint textureAttributeSize = 2;
GLenum textureAttributeType = GL_FLOAT;

// Each normal coordinate is basically a texture lookup
GLint normalAttributeSize = 2;
GLenum normalAttributeType = GL_FLOAT;

// Each Color for each vertex point consists of 4 floats				 [RGBA]
GLint colorAttributeSize = 4;
GLenum colorAttributeType = GL_FLOAT;

// Each ScalingRotation for each vertex point consists of 3 floats [Sx, Sy, rot]
GLint scalingRotationAttributeSize = 3;
GLenum scalingRotationAttributeType = GL_FLOAT;

// Extra data to be used based on rendering context
GLint extraAttributeSize = 2;
GLenum extraAttributeType = GL_FLOAT;

const GLuint VAOWrapper2D::indexVertex = 0;
const GLuint VAOWrapper2D::indexTexture = 1;
const GLuint VAOWrapper2D::indexNormal = 2;
const GLuint VAOWrapper2D::indexColor = 3;
const GLuint VAOWrapper2D::indexScalingRotation = 4;
const GLuint VAOWrapper2D::indexExtra = 5;

VAOWrapper2D::VAOWrapper2D(unsigned int flags, unsigned int maxSize)
    : vboMaxSize(maxSize), attributeFlags(flags) {
  // Generate VAO
  glGenVertexArrays(1, &vao);

  defaultScalingRotation = Vec3(1, 1, 0);
  defaultTexture = Vec2(0, 0);
  defaultNormal = Vec2(0, 0);
  defaultColor = Vec4(1, 1, 1, 1);
  defaultExtra = Vec2(0,0);

  vboVertex = 0;
  vboTexture = 0;
  vboNormal = 0;
  vboColor = 0;
  vboScalingRotation = 0;
  vboExtra = 0;

  usingTexture = ((attributeFlags & VAO_TEXTURE) == VAO_TEXTURE);
  usingNormal = ((attributeFlags & VAO_NORMAL) == VAO_NORMAL);
  usingColor = ((attributeFlags & VAO_COLOR) == VAO_COLOR);
  usingScalingRotation =
      ((attributeFlags & VAO_SCALINGROTATION) == VAO_SCALINGROTATION);
  usingExtra = ((attributeFlags & VAO_EXTRA) == VAO_EXTRA);

  CreateVBOs();
}

VAOWrapper2D::~VAOWrapper2D() {
  glDeleteBuffers(1, &vboVertex);

  if (usingTexture) {
    glDeleteBuffers(1, &vboTexture);
  }
  if (usingNormal) {
    glDeleteBuffers(1, &vboNormal);
  }
  if (usingColor) {
    glDeleteBuffers(1, &vboColor);
  }
  if (usingScalingRotation) {
    glDeleteBuffers(1, &vboScalingRotation);
  }
  if (usingExtra) {
    glDeleteBuffers(1, &vboExtra);
  }

  glDeleteVertexArrays(1, &vao);
}

void VAOWrapper2D::CreateVBOs() {
  // vao index must already be generated
  glBindVertexArray(vao);

  sizeVertex = (vboMaxSize * sizeof(Vec2) * 4);
  vboVertexArray.reset(new Vec2[vboMaxSize * 4]);

  glGenBuffers(1, &vboVertex);
  glBindBuffer(GL_ARRAY_BUFFER, vboVertex);
  //     Size of Buffer                           Pointer to data
  glBufferData(GL_ARRAY_BUFFER, sizeVertex, vboVertexArray.get(),
               GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, vboVertex);
  glVertexAttribPointer(indexVertex, vertexAttributeSize, vertexAttributeType,
                        GL_FALSE, 0, NULL);
  glEnableVertexAttribArray(indexVertex);

  if (usingTexture) {
    sizeTexture = (vboMaxSize * sizeof(Vec2) * 4);
    vboTextureArray.reset(new Vec2[vboMaxSize * 4]);

    glGenBuffers(1, &vboTexture);
    glBindBuffer(GL_ARRAY_BUFFER, vboTexture);
    glBufferData(GL_ARRAY_BUFFER, sizeTexture, vboTextureArray.get(),
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, vboTexture);
    glVertexAttribPointer(indexTexture, textureAttributeSize,
                          textureAttributeType, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(indexTexture);
  } else {
    glDisableVertexAttribArray(indexTexture);
  }

  if (usingNormal) {
    sizeNormal = (vboMaxSize * sizeof(Vec2) * 4);
    vboNormalArray.reset(new Vec2[vboMaxSize * 4]);

    glGenBuffers(1, &vboNormal);
    glBindBuffer(GL_ARRAY_BUFFER, vboNormal);
    glBufferData(GL_ARRAY_BUFFER, sizeNormal, vboNormalArray.get(),
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, vboNormal);
    glVertexAttribPointer(indexNormal, normalAttributeSize, normalAttributeType,
                          GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(indexNormal);
  } else {
    glDisableVertexAttribArray(indexNormal);
  }

  if (usingColor) {
    sizeColor = (vboMaxSize * sizeof(Vec4) * 4);
    vboColorArray.reset(new Vec4[vboMaxSize * 4]);

    glGenBuffers(1, &vboColor);
    glBindBuffer(GL_ARRAY_BUFFER, vboColor);
    glBufferData(GL_ARRAY_BUFFER, sizeColor, vboColorArray.get(),
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, vboColor);
    glVertexAttribPointer(indexColor, colorAttributeSize, colorAttributeType,
                          GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(indexColor);
  } else {
    glDisableVertexAttribArray(indexColor);
  }

  if (usingScalingRotation) {
    sizeScalingRotation = (vboMaxSize * sizeof(Vec3) * 4);
    vboScalingRotationArray.reset(new Vec3[vboMaxSize * 4]);

    glGenBuffers(1, &vboScalingRotation);
    glBindBuffer(GL_ARRAY_BUFFER, vboScalingRotation);
    glBufferData(GL_ARRAY_BUFFER, sizeScalingRotation,
                 vboScalingRotationArray.get(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, vboScalingRotation);
    glVertexAttribPointer(indexScalingRotation, scalingRotationAttributeSize,
                          scalingRotationAttributeType, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(indexScalingRotation);
  } else {
    glDisableVertexAttribArray(indexScalingRotation);
  }

  if (usingExtra) {
    sizeExtra = (vboMaxSize * sizeof(Vec2) * 4);
    vboExtraArray.reset(new Vec2[vboMaxSize * 4]);

    glGenBuffers(1, &vboExtra);
    glBindBuffer(GL_ARRAY_BUFFER, vboExtra);
    glBufferData(GL_ARRAY_BUFFER, sizeExtra, vboExtraArray.get(),
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, vboExtra);
    glVertexAttribPointer(indexExtra, extraAttributeSize, extraAttributeType,
                          GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(indexExtra);
  } else {
    glDisableVertexAttribArray(indexExtra);
  }
}

void VAOWrapper2D::UpdateGPU() {
  // Fully update all data

  glBindBuffer(GL_ARRAY_BUFFER, vboVertex);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeVertex, vboVertexArray.get());

  if (usingTexture) {
    glBindBuffer(GL_ARRAY_BUFFER, vboTexture);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeTexture, vboTextureArray.get());
  }

  if (usingNormal) {
    glBindBuffer(GL_ARRAY_BUFFER, vboNormal);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeNormal, vboNormalArray.get());
  }

  if (usingColor) {
    glBindBuffer(GL_ARRAY_BUFFER, vboColor);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeColor, vboColorArray.get());
  }

  if (usingScalingRotation) {
    glBindBuffer(GL_ARRAY_BUFFER, vboScalingRotation);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeScalingRotation,
                    vboScalingRotationArray.get());
  }

  if (usingExtra) {
    glBindBuffer(GL_ARRAY_BUFFER, vboExtra);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeExtra, vboExtraArray.get());
  }
}

void VAOWrapper2D::SetDefaultTexture(Vec2 vec) { defaultTexture = vec; }
void VAOWrapper2D::SetDefaultNormal(Vec2 vec) { defaultNormal = vec; }
void VAOWrapper2D::SetDefaultColor(Vec4 vec) { defaultColor = vec; }
void VAOWrapper2D::SetDefaultScalingRotation(Vec3 vec) {
  defaultScalingRotation = vec;
}
void VAOWrapper2D::SetDefaultExtra(Vec2 vec) { defaultExtra = vec; }

void VAOWrapper2D::Bind() {
  glBindVertexArray(vao);

  if (!usingTexture) {
    glVertexAttrib2f(indexTexture, defaultTexture.x, defaultTexture.y);
  }
  if (!usingNormal) {
    glVertexAttrib2f(indexNormal, defaultNormal.x, defaultNormal.y);
  }
  if (!usingColor) {
    glVertexAttrib4f(indexColor, defaultColor.x, defaultColor.y, defaultColor.z,
                     defaultColor.w);
  }
  if (!usingScalingRotation) {
    glVertexAttrib3f(indexScalingRotation, defaultScalingRotation.x,
                     defaultScalingRotation.y, defaultScalingRotation.z);
  }
  if (!usingExtra) {
    glVertexAttrib2f(indexExtra, defaultExtra.x, defaultExtra.y);
  }
}
