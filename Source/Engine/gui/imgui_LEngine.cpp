#include "imgui_LEngine.h"
#include "../Kernel.h"

void ImGui::SetWindowPosWrapper(const char* name, const Coord2df& pos, ImGuiSetCond cond){
	ImGui::SetWindowPos(name, pos, cond);
}

void ImGui::SetWindowSizeWrapper(const char* name, const Coord2df& size, ImGuiSetCond cond){
	ImGui::SetWindowSize(name, size, cond);
}

Coord2df ImGui::GetWindowSizeWrapper(){
	return ImGui::GetWindowSize();
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

void ImGui::SetNextWindowPosCenterWrapper(ImGuiSetCond cond){
	ImGui::SetNextWindowPosWrapper(Coord2df(-FLT_MAX, -FLT_MAX), cond);
}

void ImGui::SetNextWindowPosCenterWrapperX(float y, ImGuiSetCond cond){
	ImGui::SetNextWindowPosWrapper(Coord2df(-FLT_MAX, y), cond);
}

void ImGui::SetNextWindowPosCenterWrapperY(float x, ImGuiSetCond cond){
	ImGui::SetNextWindowPosWrapper(Coord2df(x, -FLT_MAX), cond);
}

void ImGui::SetNextWindowSizeConstraintsWrapper(const Coord2df& size_min, const Coord2df& size_max){
	SetNextWindowSizeConstraints(size_min, size_max);
}

void ImGui::Sprite(const RSC_Sprite* sprite, const std::string& animation, int frame){
	Coord2df size(0,0);
	Coord2df startUV(0,0);
	Coord2df endUV(0,0);
	ImTextureID textureID = 0; 

	CalculateUV(sprite, animation, frame, textureID, size, startUV, endUV);

	ImGui::Image(textureID, size,startUV,endUV, ImColor(255,255,255,255));
}

bool ImGui::SpriteButton(const RSC_Sprite* sprite, const std::string& animation, int frame){
	Coord2df size(0,0);
	Coord2df startUV(0,0);
	Coord2df endUV(0,0);
	ImTextureID textureID = 0; 

	CalculateUV(sprite, animation, frame, textureID, size, startUV, endUV);

	ImGui::ImageButton(textureID, size,startUV,endUV, 0, ImColor(255,255,255,125));
}

void ImGui::ProgressBarWrapper(float fraction, const Coord2df& screenSize){
		ImGui::ProgressBar(fraction,screenSize);
}

void ImGui::SameLineWrapper(){
		ImGui::SameLine();
}

////////////////////
//Parameter Stacks//
////////////////////
void ImGui::PushStyleColorWindowBG(const Color4f& c){
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(c.r,c.g,c.b,c.a));
}

void ImGui::PushStyleColorButton(const Color4f& c){
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(c.r,c.g,c.b,c.a));
}

void ImGui::PushStyleColorButtonHovered(const Color4f& c){
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(c.r,c.g,c.b,c.a));
}

void ImGui::PushStyleColorButtonActive(const Color4f& c){
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(c.r,c.g,c.b,c.a));
}

void ImGui::PushStyleColorFrameBG(const Color4f& c){
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(c.r,c.g,c.b,c.a));
}

void ImGui::PushStyleColorFrameBGActive(const Color4f& c){
	ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(c.r,c.g,c.b,c.a));
}

void ImGui::PushStyleColorFrameBGHovered(const Color4f& c){
	ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(c.r,c.g,c.b,c.a));
}

void ImGui::PushStyleColorPlotHistogram(const Color4f& c){
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(c.r,c.g,c.b,c.a));
}

void ImGui::PushStyleColorText(const Color4f& c){
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(c.r,c.g,c.b,c.a));
}

bool ImGui::PushFontWrapper(const std::string& name, int size){
	auto fontResource = K_FontMan.GetLoadItem(name, name);
	auto font = fontResource->GetFont(size);
	if(font->IsLoaded()){
			ImGui::PushFont(font);
			return true;
	}
	return false;
}

void ImGui::PopFontWrapper(){
	ImGui::PopFont();
}

////////////
//INTERNAL//
////////////
void ImGui::CalculateUV(const RSC_Sprite* sprite, const std::string& animation, int frame,	ImTextureID& textureID, Coord2df& size, Coord2df& startUV, Coord2df& endUV){
	auto ani = sprite->GetAnimation(animation);
	if(ani != NULL){
		auto texture = K_TextureMan.GetLoadItem(sprite->GetTextureName(), sprite->GetTextureName());
		textureID = (void*) texture->GetOpenGLID();
		startUV.x = ani->GetUVLeft(frame);
		startUV.y = ani->GetUVTop(frame);
		endUV.x = ani->GetUVRight(frame);
		endUV.y = ani->GetUVBottom(frame);
		auto rect =  ani->GetCRectAtIndex(frame);
		size.x = rect.w;
		size.y = rect.h;
	}

	/*try{
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
		K_Log.Write(ss.str(), Log::SEVERITY::ERROR, Log::typeDefault);
	}
	*/
}
