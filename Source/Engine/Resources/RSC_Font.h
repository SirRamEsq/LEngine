#ifndef L_ENGINE_FONT
#define L_ENGINE_FONT

#include "../gui/imgui.h"
#include "ResourceLoading.h"
#include <memory>
#include <string>
#include <map>

class RSC_Font{
    public:
        RSC_Font(const std::string& sname, std::unique_ptr<FileData> d);
        ~RSC_Font();

        const std::string name;
		ImFont* GetFont(int size) const;

        static std::unique_ptr<RSC_Font> LoadResource(const std::string& fname);

    private:
		///Maps font sizes to usable ImGui Fonts
		mutable std::map<int, ImFont*> fonts;
		std::unique_ptr<FileData> data;
};

#endif
