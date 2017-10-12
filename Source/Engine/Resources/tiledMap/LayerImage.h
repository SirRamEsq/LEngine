#ifndef LENGINE_TILED_LAYER_IMAGE
#define LENGINE_TILED_LAYER_IMAGE

#include "../../Defines.h"
#include "../../Coordinates.h"
#include "LayerGeneric.h"

#include "../RSC_Texture.h"

class TiledImageLayer : public TiledLayerGeneric{
    public:
        TiledImageLayer(const unsigned int& tileW, const unsigned int& tileH, const std::string& name, const MAP_DEPTH& depth, const GIDManager* g, const RSC_Texture* tex);
        TiledImageLayer(const TiledImageLayer& rhs, const GIDManager* g);

        void SetTexture (const RSC_Texture* tex);
        const RSC_Texture* GetTexture() const;

        void SetOffset (const Coord2df& off);
		Coord2df GetOffset() const;

        void SetParallax (const Coord2df& para);
		Coord2df GetParallax() const;

		bool GetRepeatX()const;
		void SetRepeatX(bool val);
		bool GetRepeatY()const;
		void SetRepeatY(bool val);

		bool GetStretchToMapX()const;
		void SetStretchToMapX(bool val);
		bool GetStretchToMapY()const;
		void SetStretchToMapY(bool val);

    private:
		///If true, image is repeated, if false, image is strectched
		bool repeatX, repeatY;
		bool stretchToMapX, stretchToMapY;
        Coord2df    offset;
        Coord2df    parallax;
        const RSC_Texture*   texture;
};

#endif
