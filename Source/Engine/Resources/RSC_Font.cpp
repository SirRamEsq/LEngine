#include "RSC_Font.h"
#include "../Kernel.h"

RSC_Font::RSC_Font(const std::string& sname, std::unique_ptr<FileData> d)
	: name(sname), data(std::move(d)){

}
RSC_Font::~RSC_Font(){

}

ImFont* RSC_Font::GetFont(int size) const {
    ImGuiIO& io = ImGui::GetIO();
	auto fontIterator = fonts.find(size);
	if(fontIterator == fonts.end()){
		ImFontConfig config;
		//config.OversampleH = 3;
		//config.OversampleV = 1;
		//ensure that the data isn't erased by ImGui
		//config.GlyphExtraSpacing.x =2.0f;
		config.FontDataOwnedByAtlas = false;
		
		auto font = io.Fonts->AddFontFromMemoryTTF(data->GetData(), data->length, size, &config);
		fonts[size] = font;

		//Have the kernel generate a new font texture
		Kernel::ImGuiInvalidateFontTexture();
		return font;
	}

	return fontIterator->second;
}

std::unique_ptr<RSC_Font> RSC_Font::LoadResource(const std::string& fname){
    std::unique_ptr<RSC_Font> font = NULL;
    try{
        std::string fullPath = "Resources/Fonts/"+fname;
        auto data=LoadGenericFile(fullPath);
        if(data->GetData()==NULL){
            throw LEngineException("RSC_Font::LoadResource; data is NULL!");
        }
        font = make_unique<RSC_Font>(fname, std::move(data));
    }
    catch(LEngineFileException e){
        ErrorLog::WriteToFile(e.what(), ErrorLog::GenericLogFile);
    }

    return font;
}
