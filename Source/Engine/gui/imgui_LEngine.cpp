#include "imgui_LEngine.h"

void ImGui::SetWindowPosWrapper(const char* name, const Coord2df& pos, ImGuiSetCond cond){
	ImGui::SetWindowPos(name, pos, cond);
}

void ImGui::SetWindowSizeWrapper(const char* name, const Coord2df& size, ImGuiSetCond cond){
	ImGui::SetWindowSize(name, size, cond);
}

bool ImGui::ButtonWrapper(const char* label){
	ImGui::Button(label);
}

void ImGui::TextWrapper(const std::string& str){
	ImGui::Text(str.c_str());
}

void ImGui::BeginWrapper(const std::string& name){
	ImGui::BeginFlags(name, 0);
}

void ImGui::BeginFlags(const std::string& name, int flags){
	//Always disable saving windows
	flags = flags | ImGuiWindowFlags_NoSavedSettings;
	ImGui::Begin(name.c_str(), NULL, flags);
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
