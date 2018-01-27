#include "RenderPrimitives.h"
#include "../Kernel.h"
#include "../Resources/RSC_Texture.h"
//////////////
// RenderLine//
//////////////

RenderLine::RenderLine(RenderManager *rm, int x1, int y1, int x2, int y2,
                       bool absolute)
    : RenderableObjectScreen(rm) {
  mX1 = x1;
  mY1 = y1;
  mX2 = x2;
  mY2 = y2;
  color.r = 255;
  color.g = 0;
  color.b = 0;
  render = true;
  mAbsolute = absolute;
  AddToRenderManager();
}

void RenderLine::Render(const RenderCamera *camera,
                        const RSC_GLProgram *program) {
  glBindTexture(GL_TEXTURE_2D, 0);

  double R, G, B;
  R = (double(color.r)) / 255.0f;
  G = (double(color.g)) / 255.0f;
  B = (double(color.b)) / 255.0f;

  glLineWidth(1.0f);
  glColor4f(R, G, B, 1.0f);
  glBegin(GL_LINES);
  glVertex2f(mX1, mY1);
  glVertex2f(mX2, mY2);
  glEnd();
}

RenderLine::~RenderLine() {}

/////////////
// RenderBox//
/////////////

RenderBox::RenderBox(RenderManager *rm, int x1, int y1, int x2, int y2,
                     bool absolute)
    : RenderableObjectScreen(rm) {
  mX1 = x1;
  mY1 = y1;
  mX2 = x2;
  mY2 = y2;
  color.r = 255;
  color.g = 0;
  color.b = 0;
  render = true;
  mAbsolute = absolute;
  AddToRenderManager();
}
RenderBox::~RenderBox() {}

void RenderBox::Render(const RenderCamera *camera,
                       const RSC_GLProgram *program) {
  glBindTexture(GL_TEXTURE_2D, 0);

  double R, G, B;
  R = (double(color.r)) / 255.0f;
  G = (double(color.g)) / 255.0f;
  B = (double(color.b)) / 255.0f;

  glLineWidth(1.0f);
  glColor4f(R, G, B, 1.0f);
  glBegin(GL_QUADS);
  glVertex2f(mX1, mY1);
  glVertex2f(mX2, mY1);
  glVertex2f(mX2, mY2);
  glVertex2f(mX1, mY2);
  glEnd();
}
