#ifndef LENGINE_RENDER_VAOWRAPPER
#define LENGINE_RENDER_VAOWRAPPER

#include "../glslHelper.h"

enum VAO_ATTRIBUTE_ARRAY_FLAGS {
  VAO_VERTEX = 1,
  VAO_TEXTURE = 2,
  VAO_NORMAL = 4,
  VAO_COLOR = 8,
  VAO_SCALINGROTATION = 16
};

class VAOWrapper2D {
 public:
  /**
   * \param flags The attribute arrays that this VAO will use
   * (rest have a default value)
   * \param maxSize Number of objects with 4 Verticies, not the number of
   * verticies.
   */
  VAOWrapper2D(unsigned int flags, unsigned int maxSize);
  ~VAOWrapper2D();

  /// Push data contained in arrays to GPU
  void UpdateGPU();

  Vec2 *GetVertexArray() { return vboVertexArray.get(); }
  Vec2 *GetTextureArray() { return vboTextureArray.get(); }
  Vec2 *GetNormalArray() { return vboNormalArray.get(); }
  Vec4 *GetColorArray() { return vboColorArray.get(); }
  Vec3 *GetScalingRotationArray() { return vboScalingRotationArray.get(); }

  GLuint GetVAOID() { return vao; }

  // Sets default values for when an array is not in use
  void SetDefaultTexture(Vec2 vec);
  void SetDefaultNormal(Vec2 vec);
  void SetDefaultColor(Vec4 vec);
  void SetDefaultScalingRotation(Vec3 vec);

  // Total Number of Objects passed at Wrapper construction
  const unsigned int vboMaxSize;
  // Flags dictating what VBOs to create passed at wrapper construction
  const unsigned int attributeFlags;

  // Indicies for each type of data passed to the shader
  static const GLuint indexVertex;
  static const GLuint indexTexture;
  static const GLuint indexNormal;
  static const GLuint indexColor;
  static const GLuint indexScalingRotation;

  bool UsingTexture() const { return usingTexture; }
  bool UsingNormal() const { return usingNormal; }
  bool UsingColor() const { return usingColor; }
  bool UsingScalingRotation() const { return usingScalingRotation; }

  /// Will bind Vertex Array and set default values
  void Bind();

 private:
  void CreateVBOs();

  // OpenGL generated VBO ID values
  GLuint vboVertex;
  GLuint vboTexture;
  GLuint vboNormal;
  GLuint vboColor;
  GLuint vboScalingRotation;

  // OpenGL generated VAO ID value
  GLuint vao;

  // Actual Arrays containing data that will be passed to the GPU
  std::unique_ptr<Vec2[]> vboVertexArray;
  std::unique_ptr<Vec2[]> vboTextureArray;
  std::unique_ptr<Vec2[]> vboNormalArray;
  std::unique_ptr<Vec4[]> vboColorArray;
  std::unique_ptr<Vec3[]> vboScalingRotationArray;

  // Default values for when an array is not in use
  Vec2 defaultTexture;
  Vec2 defaultNormal;
  Vec4 defaultColor;
  Vec3 defaultScalingRotation;

  // Sizes of the various buffers in bytes
  unsigned int sizeVertex;
  unsigned int sizeTexture;
  unsigned int sizeNormal;
  unsigned int sizeColor;
  unsigned int sizeScalingRotation;

  bool usingTexture;
  bool usingNormal;
  bool usingColor;
  bool usingScalingRotation;
};

#endif
