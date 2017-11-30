#ifndef L_ENGINE_RENDERABLE_OBJECT
#define L_ENGINE_RENDERABLE_OBJECT

#include "../Coordinates.h"
#include "../Defines.h"
#include "../Resources/RSC_GLShader.h"

#include <string>
#include <map>

class RenderCamera;
class RenderManager;
class RenderableObjectScreen;
class RenderableObjectWorld;

class RenderableObject {
  friend RenderManager;
  friend RenderableObjectScreen;
  friend RenderableObjectWorld;

 public:
  enum TYPE {
    Generic = 0,
    Screen = 1,
    World = 2,
    SpriteBatch = 3,
    TileLayer = 4,
    Image = 5
  };

  RenderableObject(RenderManager *rm, TYPE t = TYPE::Generic);

  // Virtual destructor; enables derived classes to be fully deleted from a base
  // RenderableObject pointer
  virtual ~RenderableObject() {}

  // All renderable objects will render using their own shader program if one is
  // not passed to them
  virtual void Render(const RenderCamera *camera,
                      const RSC_GLProgram *program) = 0;
  void Render(const RenderCamera *camera);
  void Render();

  void SetDepth(const MAP_DEPTH &d);
  MAP_DEPTH GetDepth();

  void SetRotation(const double &rot) { rotation = rot; }
  double GetRotation() { return rotation; }

  void SetScalingY(const double &s) { scaleY = s; }
  double GetScalingY() { return scaleY; }

  void SetScalingX(const double &s) { scaleX = s; }
  double GetScalingX() { return scaleX; }

  void SetPositionX(const double &x) { position.x = x; }
  double GetPositionX() { return position.x; }

  void SetPositionY(const double &y) { position.y = y; }
  double GetPositionY() { return position.y; }

  void SetRender(const bool &s) { render = s; }
  bool GetRender() { return render; }

  Color4f GetColor() { return color; }
  void SetColor(Color4f c) { color = c; }

  void SetShaderProgram(const RSC_GLProgram *p) { shaderProgram = p; }
  const RSC_GLProgram *GetShaderProgram() { return shaderProgram; }

  // void UpdateShaderProgram();

  bool render;
  Color4f color;

  TYPE type;
  std::string GetTypeString();

 protected:
  virtual void AddToRenderManager() = 0;
  RenderManager *renderManager;

 private:
  MAP_DEPTH depth;
  double scaleX, scaleY, rotation;
  Coord2df position;

  const RSC_GLProgram *shaderProgram;
  static std::map<TYPE, std::string> TYPE_STR;
};

class RenderableObjectScreen : public RenderableObject {
 public:
  RenderableObjectScreen(RenderManager *rm, TYPE t = TYPE::Screen);
  virtual ~RenderableObjectScreen();

  virtual void AddToRenderManager();
};

class RenderableObjectWorld : public RenderableObject {
 public:
  RenderableObjectWorld(RenderManager *rm, TYPE t = TYPE::World);
  virtual ~RenderableObjectWorld();

  virtual void AddToRenderManager();
};

#endif  // L_ENGINE_RENDERABLE_OBJECT
