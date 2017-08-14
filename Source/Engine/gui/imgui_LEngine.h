#ifndef L_ENGINE_IMGUI
#define L_ENGINE_IMGUI

#include "../Defines.h"
#include "imgui.h"
#include "../Resources/RSC_Sprite.h"

namespace ImGui{
	///Begin new Window
	void BeginWrapper(const std::string& name);
	///Begin new Window with flags
	void BeginFlags(const std::string& name, int flags);

    void SetWindowPosWrapper(const char* name, const Coord2df& pos, ImGuiSetCond cond = 0);
    void SetWindowSizeWrapper(const char* name, const Coord2df& size, ImGuiSetCond cond = 0);
	Coord2df GetWindowSizeWrapper();

    void SetNextWindowSizeWrapper(const Coord2df& size, ImGuiSetCond cond = 0);
    void SetNextWindowPosWrapper(const Coord2df& pos, ImGuiSetCond cond = 0);
    void SetNextWindowPosCenterWrapper(ImGuiSetCond cond = 0);
	///Must pass y Value
    void SetNextWindowPosCenterWrapperX(float y, ImGuiSetCond cond = 0);
	///Must pass x Value
    void SetNextWindowPosCenterWrapperY(float x, ImGuiSetCond cond = 0);
	void SetNextWindowSizeConstraintsWrapper(const Coord2df& size_min, const Coord2df& size_max);

	///Display Text
	void TextWrapper(const std::string& str);
	///Display a Button and return whether it was pressed
	bool ButtonWrapper(const char* label);
	
	///Display a frame from a sprite
	void Sprite(const RSC_Sprite* sprite, const std::string& animation, int frame);
	///Display a button by using a frame from a sprite bool SpriteButton(const RSC_Sprite* sprite, const std::string& animation, int frame);
	bool SpriteButton(const RSC_Sprite* sprite, const std::string& animation, int frame);

	/**
	 * Create a ProgressBar
     * Use ImVec2(-1.0f,0.0f) to use all available width
	 * ImVec2(width,0.0f) for a specified width
	 * ImVec2(0.0f,0.0f) uses ItemWidth.
	 * \param fraction Amount of progress bar that is full, between 0.0f and 1.0f
	 * \param screenSize Pixels taken for each axis
	 */
	void ProgressBarWrapper(float fraction, const Coord2df& screenSize);
	
	///Display the next widget on the same line as the previous widget
	void SameLineWrapper();

	////////////////////
	//Parameter Stacks//
	////////////////////
	void PushStyleColorWindowBG(const Color4f& c);
	void PushStyleColorButton(const Color4f& c);
	void PushStyleColorButtonHovered(const Color4f& c);
	void PushStyleColorButtonActive(const Color4f& c);
	void PushStyleColorFrameBG(const Color4f& c);
	void PushStyleColorFrameBGHovered(const Color4f& c);
	void PushStyleColorFrameBGActive(const Color4f& c);
	void PushStyleColorPlotHistogram(const Color4f& c);
	void PushStyleColorText(const Color4f& c);

	//Returns true if font is pushed
	bool PushFontWrapper(const std::string& name, int size);
	void PopFontWrapper();

	////////////
	//INTERNAL//
	////////////
	void CalculateUV(const RSC_Sprite* sprite, const std::string& animation,  int frame, ImTextureID& textureID, Coord2df& size, Coord2df& startUV, Coord2df& endUV);
};

#endif
