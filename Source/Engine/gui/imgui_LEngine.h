#ifndef L_ENGINE_IMGUI
#define L_ENGINE_IMGUI

#include "../Defines.h"
#include "imgui.h"

namespace ImGui{
	//These wrappers are used primarily to replace ImVec2 with Coord2df for Lua's sake
    void SetWindowPosWrapper(const char* name, const Coord2df& pos, ImGuiSetCond cond = 0);
    void SetWindowSizeWrapper(const char* name, const Coord2df& size, ImGuiSetCond cond = 0);
	bool ButtonWrapper(const char* label, const Coord2df& size = Coord2df(0,0));

	void BeginWrapper(std::string name, bool isOpen = true);
};

#endif
