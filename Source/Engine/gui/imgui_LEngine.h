#ifndef L_ENGINE_IMGUI
#define L_ENGINE_IMGUI

#include "../Defines.h"
#include "../LuaInclude.h"
#include "../Resources/RSC_Sprite.h"
#include "imgui.h"

namespace ImGui {
/// Begin new Window
void BeginWrapper(const std::string &name);
/// Begin new Window with flags
void BeginFlags(const std::string &name, int flags);

void SetWindowPosWrapper(const char *name, const Vec2 &pos,
                         ImGuiSetCond cond = 0);
void SetWindowSizeWrapper(const char *name, const Vec2 &size,
                          ImGuiSetCond cond = 0);
Vec2 GetWindowSizeWrapper();

void SetNextWindowSizeWrapper(const Vec2 &size, ImGuiSetCond cond = 0);
void SetNextWindowPosWrapper(const Vec2 &pos, ImGuiSetCond cond = 0);
void SetNextWindowPosCenterWrapper(ImGuiSetCond cond = 0);
/// Must pass y Value
void SetNextWindowPosCenterWrapperX(float y, ImGuiSetCond cond = 0);
/// Must pass x Value
void SetNextWindowPosCenterWrapperY(float x, ImGuiSetCond cond = 0);
void SetNextWindowSizeConstraintsWrapper(const Vec2 &size_min,
                                         const Vec2 &size_max);

/// Display Text
void TextWrapper(const std::string &str);
/// Display a Button and return whether it was pressed
bool ButtonWrapper(const char *label);

/// Display a frame from a sprite
void Sprite(const RSC_Sprite *sprite, const std::string &animation, int frame);
/// Display a button by using a frame from a sprite bool SpriteButton(const
/// RSC_Sprite* sprite, const std::string& animation, int frame);
bool SpriteButton(const RSC_Sprite *sprite, const std::string &animation,
                  int frame);

/**
 * Create a ProgressBar
* Use ImVec2(-1.0f,0.0f) to use all available width
 * ImVec2(width,0.0f) for a specified width
 * ImVec2(0.0f,0.0f) uses ItemWidth.
 * \param fraction Amount of progress bar that is full, between 0.0f and 1.0f
 * \param screenSize Pixels taken for each axis
 */
void ProgressBarWrapper(float fraction, const Vec2 &screenSize);

/// Display the next widget on the same line as the previous widget
void SameLineWrapper();

void SetContext(int c);

////////////////////
// Parameter Stacks//
////////////////////
void PushStyleColorWindowBG(const Vec4 &c);
void PushStyleColorButton(const Vec4 &c);
void PushStyleColorButtonHovered(const Vec4 &c);
void PushStyleColorButtonActive(const Vec4 &c);
void PushStyleColorFrameBG(const Vec4 &c);
void PushStyleColorFrameBGHovered(const Vec4 &c);
void PushStyleColorFrameBGActive(const Vec4 &c);
void PushStyleColorPlotHistogram(const Vec4 &c);
void PushStyleColorText(const Vec4 &c);

// Returns true if font is pushed
bool PushFontWrapper(const std::string &name, int size);
void PopFontWrapper();

////////////
// INTERNAL//
////////////
void CalculateUV(const RSC_Sprite *sprite, const std::string &animation,
                 int frame, ImTextureID &textureID, Vec2 &size, Vec2 &startUV,
                 Vec2 &endUV);

void ExposeLuaInterface(lua_State *state);
};

#endif
