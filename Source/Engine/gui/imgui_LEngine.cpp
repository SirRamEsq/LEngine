#include "imgui_LEngine.h"
#include "../Kernel.h"

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

void ImGui::CalculateUV(const RSC_Sprite* sprite, const std::string& animation, int frame,  ImTextureID& textureID, Coord2df& size, Coord2df& startUV, Coord2df& endUV){
	try{
		auto texture = K_TextureMan.GetLoadItem(sprite->GetTextureName(), sprite->GetTextureName());
		textureID = (void*) texture->GetOpenGLID();
		auto animationStruct = sprite->GetAnimation(animation);
		auto frameRect = animationStruct->GetCRectAtIndex(frame);

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
		ErrorLog::WriteToFile(ss.str(), ErrorLog::SEVERITY::ERROR, ErrorLog::GenericLogFile);
	}
}

void ImGui::Sprite(const RSC_Sprite* sprite, const std::string& animation, int frame){
	Coord2df size(0,0);
	Coord2df startUV(0,0);
	Coord2df endUV(0,0);
	ImTextureID textureID = 0; 

	CalculateUV(sprite, animation, frame, textureID, size, startUV, endUV);

	std::stringstream ss;
	ImGui::Text("%.0f x %.0f", size.x, size.y);
	
	ImGui::Text("StartUV %f x %f", startUV.x, startUV.y);

	ImGui::Text("EndUV %f x %f", endUV.x, endUV.y);

	ImGui::Image(textureID, size,startUV,endUV, ImColor(255,255,255,255), ImColor(255,255,255,255));
}

bool ImGui::SpriteButton(const RSC_Sprite* sprite){
}

void ImGui::ProgressBarWrapper(float fraction){
        ImGui::ProgressBar(fraction, ImVec2(-1.0f,0.0f));
}
void ImGui::ProgressBarText(float fraction, const std::string& text){
        ImGui::ProgressBar(fraction, ImVec2(-1.0f,0.0f));
        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
        ImGui::Text(text.c_str());
}
