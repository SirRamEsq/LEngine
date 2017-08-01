#ifndef L_ENGINE_IMGUI
#define L_ENGINE_IMGUI

#include "../Defines.h"
#include "imgui.h"
#include "../Resources/RSC_Sprite.h"

namespace ImGui{
	void BeginWrapper(const std::string& name);
	void BeginFlags(const std::string& name, int flags);

	//These wrappers are used primarily to replace ImVec2 with Coord2df for Lua's sake
    void SetWindowPosWrapper(const char* name, const Coord2df& pos, ImGuiSetCond cond = 0);
    void SetWindowSizeWrapper(const char* name, const Coord2df& size, ImGuiSetCond cond = 0);

    void SetNextWindowSizeWrapper(const Coord2df& size, ImGuiSetCond cond = 0);
    void SetNextWindowPosWrapper(const Coord2df& pos, ImGuiSetCond cond = 0);
	void SetNextWindowSizeConstraintsWrapper(const Coord2df& size_min, const Coord2df& size_max);

	void TextWrapper(const std::string& str);
	bool ButtonWrapper(const char* label);
	
	void CalculateUV(const RSC_Sprite* sprite, const std::string& animation,  int frame, ImTextureID& textureID, Coord2df& size, Coord2df& startUV, Coord2df& endUV);
	void Sprite(const RSC_Sprite* sprite, const std::string& animation, int frame);
	bool SpriteButton(const RSC_Sprite* sprite);
	void ProgressBarWrapper(float fraction);
	void ProgressBarText(float fraction, const std::string& text);
};

#endif
