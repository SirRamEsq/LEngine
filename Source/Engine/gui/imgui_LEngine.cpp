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

void ImGui::TextWrapper(const std::string& str){
	ImGui::Text(str.c_str());
}

void ImGui::BeginWrapper(std::string name){
	ImGui::Begin(name.c_str(), NULL);
}

void ImGui::SetNextWindowSizeWrapper(const Coord2df& size, ImGuiSetCond cond){
	ImGui::SetNextWindowSize(size, cond);
}

void ImGui::SetNextWindowPosWrapper(const Coord2df& pos, ImGuiSetCond cond){
	ImGui::SetNextWindowPos(pos, cond);
}

void ImGui::SetNextWindowSizeConstraintsWrapper(const Coord2df& size_min, const Coord2df& size_max){
	SetNextWindowSizeConstraints(size_min, size_max);
}
