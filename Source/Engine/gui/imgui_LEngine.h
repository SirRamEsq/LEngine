#ifndef L_ENGINE_IMGUI
#define L_ENGINE_IMGUI

#include "../Defines.h"
#include "imgui.h"

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
};

#endif
