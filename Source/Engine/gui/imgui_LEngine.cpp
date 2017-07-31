#include "imgui_LEngine.h"

void ImGui::SetWindowPosWrapper(const char* name, const Coord2df& pos, ImGuiSetCond cond){
	ImGui::SetWindowPos(name, pos, cond);
}

void ImGui::SetWindowSizeWrapper(const char* name, const Coord2df& size, ImGuiSetCond cond){
	ImGui::SetWindowSize(name, size, cond);
}

bool ImGui::ButtonWrapper(const char* label, const Coord2df& size){
	ImGui::Button(label, size);
}

void ImGui::BeginWrapper(std::string name, bool isOpen){
	ImGui::Begin(name.c_str(), &isOpen);
}
