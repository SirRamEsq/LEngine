#include "imgui_LEngine.h"
#include "../Kernel.h"

void ImGui::SetWindowPosWrapper(const char *name, const Vec2 &pos,
                                ImGuiSetCond cond) {
  ImGui::SetWindowPos(name, pos, cond);
}

void ImGui::SetWindowSizeWrapper(const char *name, const Vec2 &size,
                                 ImGuiSetCond cond) {
  ImGui::SetWindowSize(name, size, cond);
}

Vec2 ImGui::GetWindowSizeWrapper() { return ImGui::GetWindowSize(); }

bool ImGui::ButtonWrapper(const char *label) { return ImGui::Button(label); }

void ImGui::TextWrapper(const std::string &str) { ImGui::Text(str.c_str()); }

void ImGui::BeginWrapper(const std::string &name) {
  ImGui::BeginFlags(name, 0);
}

void ImGui::BeginFlags(const std::string &name, int flags) {
  // Always disable saving windows
  flags = flags | ImGuiWindowFlags_NoSavedSettings;
  ImGui::Begin(name.c_str(), NULL, flags);
}

void ImGui::SetNextWindowSizeWrapper(const Vec2 &size, ImGuiSetCond cond) {
  ImGui::SetNextWindowSize(size, cond);
}

void ImGui::SetNextWindowPosWrapper(const Vec2 &pos, ImGuiSetCond cond) {
  ImGui::SetNextWindowPos(pos, cond);
}

void ImGui::SetNextWindowPosCenterWrapper(ImGuiSetCond cond) {
  ImGui::SetNextWindowPosWrapper(Vec2(-FLT_MAX, -FLT_MAX), cond);
}

void ImGui::SetNextWindowPosCenterWrapperX(float y, ImGuiSetCond cond) {
  ImGui::SetNextWindowPosWrapper(Vec2(-FLT_MAX, y), cond);
}

void ImGui::SetNextWindowPosCenterWrapperY(float x, ImGuiSetCond cond) {
  ImGui::SetNextWindowPosWrapper(Vec2(x, -FLT_MAX), cond);
}

void ImGui::SetNextWindowSizeConstraintsWrapper(const Vec2 &size_min,
                                                const Vec2 &size_max) {
  SetNextWindowSizeConstraints(size_min, size_max);
}

void ImGui::Sprite(const RSC_Sprite *sprite, const std::string &animation,
                   int frame) {
  if (sprite == NULL) {
    LOG_ERROR("Sprite is NULL");
    return;
  }
  Vec2 size(0, 0);
  Vec2 startUV(0, 0);
  Vec2 endUV(0, 0);
  ImTextureID textureID = 0;

  CalculateUV(sprite, animation, frame, textureID, size, startUV, endUV);

  ImGui::Image(textureID, size, startUV, endUV, ImColor(255, 255, 255, 255));
}

bool ImGui::SpriteButton(const RSC_Sprite *sprite, const std::string &animation,
                         int frame) {
  Vec2 size(0, 0);
  Vec2 startUV(0, 0);
  Vec2 endUV(0, 0);
  ImTextureID textureID = 0;

  CalculateUV(sprite, animation, frame, textureID, size, startUV, endUV);

  return ImGui::ImageButton(textureID, size, startUV, endUV, 0,
                            ImColor(255, 255, 255, 125));
}

void ImGui::ProgressBarWrapper(float fraction, const Vec2 &screenSize) {
  ImGui::ProgressBar(fraction, screenSize);
}

void ImGui::SameLineWrapper() { ImGui::SameLine(); }

////////////////////
// Parameter Stacks//
////////////////////
void ImGui::PushStyleColorWindowBG(const Vec4 &c) {
  ImGui::PushStyleColor(ImGuiCol_WindowBg, ImColor(c.x, c.y, c.z, c.w));
}

void ImGui::PushStyleColorButton(const Vec4 &c) {
  ImGui::PushStyleColor(ImGuiCol_Button, ImColor(c.x, c.y, c.z, c.w));
}

void ImGui::PushStyleColorButtonHovered(const Vec4 &c) {
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor(c.x, c.y, c.z, c.w));
}

void ImGui::PushStyleColorButtonActive(const Vec4 &c) {
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor(c.x, c.y, c.z, c.w));
}

void ImGui::PushStyleColorFrameBG(const Vec4 &c) {
  ImGui::PushStyleColor(ImGuiCol_FrameBg, ImColor(c.x, c.y, c.z, c.w));
}

void ImGui::PushStyleColorFrameBGActive(const Vec4 &c) {
  ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImColor(c.x, c.y, c.z, c.w));
}

void ImGui::PushStyleColorFrameBGHovered(const Vec4 &c) {
  ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImColor(c.x, c.y, c.z, c.w));
}

void ImGui::PushStyleColorPlotHistogram(const Vec4 &c) {
  ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImColor(c.x, c.y, c.z, c.w));
}

void ImGui::PushStyleColorText(const Vec4 &c) {
  ImGui::PushStyleColor(ImGuiCol_Text, ImColor(c.x, c.y, c.z, c.w));
}

bool ImGui::PushFontWrapper(const std::string &name, int size) {
  auto fontResource = K_FontMan.GetLoadItem(name, name);
  auto font = fontResource->GetFont(size);
  if (font->IsLoaded()) {
    ImGui::PushFont(font);
    return true;
  }
  return false;
}

void ImGui::PopFontWrapper() { ImGui::PopFont(); }

////////////
// INTERNAL//
////////////
void ImGui::CalculateUV(const RSC_Sprite *sprite, const std::string &animation,
                        int frame, ImTextureID &textureID, Vec2 &size,
                        Vec2 &startUV, Vec2 &endUV) {
  auto ani = sprite->GetAnimation(animation);
  if (ani != NULL) {
    auto texture = K_TextureMan.GetLoadItem(sprite->GetTextureName(),
                                            sprite->GetTextureName());
    textureID = (void *)texture->GetOpenGLID();
    startUV.x = ani->GetUVLeft(frame);
    startUV.y = ani->GetUVTop(frame);
    endUV.x = ani->GetUVRight(frame);
    endUV.y = ani->GetUVBottom(frame);
    auto rect = ani->GetRectAtIndex(frame);
    size.x = rect.w;
    size.y = rect.h;
  }

  /*try{
          auto texture = K_TextureMan.GetLoadItem(sprite->GetTextureName(),
  sprite->GetTextureName());
          textureID = (void*) texture->GetOpenGLID();
          auto animationStruct = sprite->GetAnimation(animation);
          auto frameRect = animationStruct->GetRectAtIndex(frame);

          float startUVX = ((float)frameRect.GetLeft()) / texture->GetWidth();
          float startUVY = ((float)frameRect.GetTop()) / texture->GetHeight();
          float endUVX = ((float)frameRect.GetRight())/ texture->GetWidth();
          float endUVY = ((float)frameRect.GetBottom()) / texture->GetHeight();
          size.x = frameRect.w;
          size.y = frameRect.h;
          startUV.x = startUVX;
          startUV.y = startUVY;
          endUV.x = endUVX;
          endUV.y = endUVY;
  }
  catch(LEngineException e){
          std::stringstream ss;
          ss << "Error Occured in CalculateUV " << e.what();
          K_Log.Write(ss.str(), Log::SEVERITY::ERROR, Log::typeDefault);
  }
  */
}
