#include "RenderManager.h"

#include "Components/CompSprite.h"
#include "Kernel.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"

#include "Resources/LMap.h"

std::string ROTypeRenderableObject          = "RenderableObject";
std::string ROTypeRenderableObjectScreen    = "RenderableObjectScreen";
std::string ROTypeRenderableObjectWorld     = "RenderableObjectWorld";
std::string ROTypeSpriteBatch               = "SpriteBatch";
std::string ROTypeTileLayer                 = "TileLayer";
std::string ROTypeImage                     = "Image";


////////////////////
//RenderableObject//
////////////////////
RenderableObject::RenderableObject() : type(ROTypeRenderableObject){
    depth=0;
    scaleX=1.0f;
    scaleY=1.0f;
    rotation=0.0f;
    render=true;
    shaderProgram=NULL;
}

void RenderableObject::SetDepth(const int& i){
    depth=i;
    Kernel::stateMan.GetCurrentState()->renderMan.OrderOBJs();
}

int RenderableObject::GetDepth(){return depth;}

//////////////////////////
//RenderableObjectScreen//
//////////////////////////
RenderableObjectScreen::RenderableObjectScreen() :  RenderableObject(){
    type=ROTypeRenderableObjectScreen;
}

void RenderableObjectScreen::AddToRenderManager(){
    Kernel::stateMan.GetCurrentState()->renderMan.AddObjectScreen(this);
}

RenderableObjectScreen::~RenderableObjectScreen(){
    Kernel::stateMan.GetCurrentState()->renderMan.RemoveObjectScreen(this);
}

/////////////////////////
//RenderableObjectWorld//
/////////////////////////
RenderableObjectWorld::RenderableObjectWorld() : RenderableObject(){
    type=ROTypeRenderableObjectWorld;
}

void RenderableObjectWorld::AddToRenderManager(){
    Kernel::stateMan.GetCurrentState()->renderMan.AddObjectWorld(this);
}

RenderableObjectWorld::~RenderableObjectWorld(){
    Kernel::stateMan.GetCurrentState()->renderMan.RemoveObjectWorld(this);
}


////////////////////
//RenderableSprite//
////////////////////

RenderableSprite::RenderableSprite(const std::string& texture, const unsigned int& w, const unsigned int& h, const MAP_DEPTH& d, const Vec2& off)
: textureName(texture), textureWidth(w), textureHeight(h), depth(d), offset(off) {
    spriteBatch= Kernel::stateMan.GetCurrentState()->renderMan.GetSpriteBatch(textureName, depth, 1);
    spriteBatch->AddSprite(this);

    data.color.x = 1.0f;
    data.color.y = 1.0f;
    data.color.z = 1.0f;
    data.color.w = 1.0f;

    scaleX   = 1.0f;
    scaleY   = 1.0f;
    rotation = 0.0f;

    data.scalingRotation.x = scaleX;
    data.scalingRotation.y = scaleY;
    data.scalingRotation.z = rotation;

    data.translate.x = 0;
    data.translate.y = 0;

    isActive = true;
}

RenderableSprite::~RenderableSprite(){
    spriteBatch->DeleteSprite(this);
}

//////////////
//VAOWrapper//
//////////////
VAOWrapper::VAOWrapper(const unsigned int& maxSize)
    : vboMaxSize(maxSize),
    vboVertexSize         (maxSize * sizeof(Vec2)  * 4),//4 verticies per object
    vboTextureSize        (maxSize * sizeof(Vec2)  * 4),
    vboColorSize          (maxSize * sizeof(Vec4)  * 4),
    //vboTranslateSize      (maxSize * sizeof(Translate)         * 4),
    //vboScalingRotationSize(maxSize * sizeof(ScalingRotation)   * 4),

    vboVertexArray            ( new Vec2           [maxSize * 4] ),
	vboTextureArray           ( new Vec2           [maxSize * 4] ),
	vboColorArray             ( new Vec4           [maxSize * 4] ){
	//vboTranslateArray         ( new Translate        [maxSize * 4] ),
	//vboScalingRotationArray   ( new ScalingRotation  [maxSize * 4] ){

    //Vertex VBO
	glGenBuffers (1, &vboVertex);
	glBindBuffer (GL_ARRAY_BUFFER, vboVertex);
                            //     Size of Buffer                           Pointer to data
	glBufferData (GL_ARRAY_BUFFER, vboVertexSize,    vboVertexArray.get(), GL_STATIC_DRAW);

	//Texture VBO
	glGenBuffers (1, &vboTexture);
	glBindBuffer (GL_ARRAY_BUFFER, vboTexture);
	glBufferData (GL_ARRAY_BUFFER, vboTextureSize,   vboTextureArray.get(), GL_STATIC_DRAW);

	//Color VBO
	glGenBuffers (1, &vboColor);
	glBindBuffer (GL_ARRAY_BUFFER, vboColor);
	glBufferData (GL_ARRAY_BUFFER, vboColorSize,     vboColorArray.get(), GL_STATIC_DRAW);

    //Generate VAO
	glGenVertexArrays (1, &vao);
	glBindVertexArray (vao);

	//Bind Vertex to 0
	glBindBuffer (GL_ARRAY_BUFFER, vboVertex);
	glVertexAttribPointer (0, vertexAttributeSize, vertexAttributeType, GL_FALSE, 0, NULL);

	//Bind Texture to 1
	glBindBuffer (GL_ARRAY_BUFFER, vboTexture);
	glVertexAttribPointer (1, textureAttributeSize, textureAttributeType, GL_FALSE, 0, NULL);

	//Bind Color to 2
	glBindBuffer (GL_ARRAY_BUFFER, vboColor);
	glVertexAttribPointer (2, colorAttributeSize, colorAttributeType, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray (0);
	glEnableVertexAttribArray (1);
	glEnableVertexAttribArray (2);
}

void VAOWrapper::UpdateGPU(){
    //upload vertexTexture array along with any changed data
    glBindBuffer (GL_ARRAY_BUFFER, vboVertex);
    glBufferSubData(GL_ARRAY_BUFFER,    0,      vboVertexSize, vboVertexArray.get());

    glBindBuffer (GL_ARRAY_BUFFER, vboTexture);
    glBufferSubData(GL_ARRAY_BUFFER,    0,      vboTextureSize, vboTextureArray.get());

    glBindBuffer (GL_ARRAY_BUFFER, vboColor);
    glBufferSubData(GL_ARRAY_BUFFER,    0,      vboColorSize, vboColorArray.get());
}

VAOWrapper::~VAOWrapper(){
    glDeleteVertexArrays(1, &vboVertex );
    glDeleteVertexArrays(1, &vboColor  );
    glDeleteVertexArrays(1, &vboTexture);
}
//////////////
//VAOWrapperTile//
//////////////
VAOWrapperTile::VAOWrapperTile(const unsigned int& maxSize)
    : vboMaxSize(maxSize),
    vboVertexSize         (maxSize * sizeof(Vec2)  * 4),//4 verticies per object
    vboTextureSize        (maxSize * sizeof(Vec2)  * 4),
    vboColorSize          (maxSize * sizeof(Vec4)  * 4),
    vboAnimationSize      (maxSize * sizeof(Vec2)  * 4),

    vboVertexArray            ( new Vec2           [maxSize * 4] ),
	vboTextureArray           ( new Vec2           [maxSize * 4] ),
	vboColorArray             ( new Vec4           [maxSize * 4] ),
	vboAnimationArray         ( new Vec2           [maxSize * 4] ){

    //Vertex VBO
	glGenBuffers (1, &vboVertex);
	glBindBuffer (GL_ARRAY_BUFFER, vboVertex);
                            //     Size of Buffer                           Pointer to data
	glBufferData (GL_ARRAY_BUFFER, vboVertexSize,    vboVertexArray.get(), GL_STATIC_DRAW);

	//Texture VBO
	glGenBuffers (1, &vboTexture);
	glBindBuffer (GL_ARRAY_BUFFER, vboTexture);
	glBufferData (GL_ARRAY_BUFFER, vboTextureSize,   vboTextureArray.get(), GL_STATIC_DRAW);

	//Color VBO
	glGenBuffers (1, &vboColor);
	glBindBuffer (GL_ARRAY_BUFFER, vboColor);
	glBufferData (GL_ARRAY_BUFFER, vboColorSize,     vboColorArray.get(), GL_STATIC_DRAW);

	//Animation VBO
	glGenBuffers (1, &vboAnimation);
	glBindBuffer (GL_ARRAY_BUFFER, vboAnimation);
	glBufferData (GL_ARRAY_BUFFER, vboAnimationSize,     vboAnimationArray.get(), GL_STATIC_DRAW);

    //Generate VAO
	glGenVertexArrays (1, &vao);
	glBindVertexArray (vao);

	//Bind Vertex to 0
	glBindBuffer (GL_ARRAY_BUFFER, vboVertex);
	glVertexAttribPointer (0, vertexAttributeSize, vertexAttributeType, GL_FALSE, 0, NULL);

	//Bind Texture to 1
	glBindBuffer (GL_ARRAY_BUFFER, vboTexture);
	glVertexAttribPointer (1, textureAttributeSize, textureAttributeType, GL_FALSE, 0, NULL);

	//Bind Color to 2
	glBindBuffer (GL_ARRAY_BUFFER, vboColor);
	glVertexAttribPointer (2, colorAttributeSize, colorAttributeType, GL_FALSE, 0, NULL);

	//Bind Animation to 3
	glBindBuffer (GL_ARRAY_BUFFER, vboAnimation);
	glVertexAttribPointer (3, animationAttributeSize, animationAttributeType, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray (0);
	glEnableVertexAttribArray (1);
	glEnableVertexAttribArray (2);
	glEnableVertexAttribArray (3);
}

void VAOWrapperTile::UpdateGPU(){
    //upload vertexTexture array along with any changed data
    glBindBuffer (GL_ARRAY_BUFFER, vboVertex);
    glBufferSubData(GL_ARRAY_BUFFER,    0,      vboVertexSize, vboVertexArray.get());

    glBindBuffer (GL_ARRAY_BUFFER, vboTexture);
    glBufferSubData(GL_ARRAY_BUFFER,    0,      vboTextureSize, vboTextureArray.get());

    glBindBuffer (GL_ARRAY_BUFFER, vboColor);
    glBufferSubData(GL_ARRAY_BUFFER,    0,      vboColorSize, vboColorArray.get());

    glBindBuffer (GL_ARRAY_BUFFER, vboAnimation);
    glBufferSubData(GL_ARRAY_BUFFER,    0,      vboAnimationSize, vboAnimationArray.get());
}

VAOWrapperTile::~VAOWrapperTile(){
    glDeleteVertexArrays(1, &vboVertex );
    glDeleteVertexArrays(1, &vboColor  );
    glDeleteVertexArrays(1, &vboTexture);
    glDeleteVertexArrays(1, &vboAnimation);
}

////////////////////
//VAOWrapperSprite//
////////////////////
VAOWrapperSprite::VAOWrapperSprite(const unsigned int& maxSize)
    : vboMaxSize(maxSize),
    vboVertexSize         (maxSize * sizeof(Vec2)  * 4),//4 verticies per object
    vboTextureSize        (maxSize * sizeof(Vec2)  * 4),
    vboColorSize          (maxSize * sizeof(Vec4)  * 4),
    vboTranslateSize      (maxSize * sizeof(Vec2)  * 4),
    vboScalingRotationSize(maxSize * sizeof(Vec3)  * 4),

    vboVertexArray            ( new Vec2  [maxSize * 4] ),
	vboTextureArray           ( new Vec2  [maxSize * 4] ),
	vboColorArray             ( new Vec4  [maxSize * 4] ),
	vboTranslateArray         ( new Vec2  [maxSize * 4] ),
	vboScalingRotationArray   ( new Vec3  [maxSize * 4] ){

    //Vertex VBO
	glGenBuffers (1, &vboVertex);
	glBindBuffer (GL_ARRAY_BUFFER, vboVertex);
                            //     Size of Buffer                           Pointer to data
	glBufferData (GL_ARRAY_BUFFER, vboVertexSize,    vboVertexArray.get(), GL_STATIC_DRAW);

	//Texture VBO
	glGenBuffers (1, &vboTexture);
	glBindBuffer (GL_ARRAY_BUFFER, vboTexture);
	glBufferData (GL_ARRAY_BUFFER, vboTextureSize,   vboTextureArray.get(), GL_STATIC_DRAW);

	//Color VBO
	glGenBuffers (1, &vboColor);
	glBindBuffer (GL_ARRAY_BUFFER, vboColor);
	glBufferData (GL_ARRAY_BUFFER, vboColorSize,     vboColorArray.get(), GL_STATIC_DRAW);

	//ScalingRotation VBO
	glGenBuffers (1, &vboScalingRotation);
	glBindBuffer (GL_ARRAY_BUFFER, vboScalingRotation);
	glBufferData (GL_ARRAY_BUFFER, vboScalingRotationSize, vboScalingRotationArray.get(), GL_STATIC_DRAW);

	//Translate VBO
	glGenBuffers (1, &vboTranslate);
	glBindBuffer (GL_ARRAY_BUFFER, vboTranslate);
	glBufferData (GL_ARRAY_BUFFER, vboTranslateSize, vboTranslateArray.get(), GL_STATIC_DRAW);

    //Generate VAO
	glGenVertexArrays (1, &vao);
	glBindVertexArray (vao);

	//Bind Vertex to 0
	glBindBuffer (GL_ARRAY_BUFFER, vboVertex);
	glVertexAttribPointer (0, vertexAttributeSize, vertexAttributeType, GL_FALSE, 0, NULL);

	//Bind Texture to 1
	glBindBuffer (GL_ARRAY_BUFFER, vboTexture);
	glVertexAttribPointer (1, textureAttributeSize, textureAttributeType, GL_FALSE, 0, NULL);

	//Bind Color to 2
	glBindBuffer (GL_ARRAY_BUFFER, vboColor);
	glVertexAttribPointer (2, colorAttributeSize, colorAttributeType, GL_FALSE, 0, NULL);

	//Bind ScalingRotation to 3
	glBindBuffer (GL_ARRAY_BUFFER, vboScalingRotation);
	glVertexAttribPointer (3, scalingRotationAttributeSize, scalingRotationAttributeType, GL_FALSE, 0, NULL);

	//Bind Translate to 4
	glBindBuffer (GL_ARRAY_BUFFER, vboTranslate);
	glVertexAttribPointer (4, translateAttributeSize, translateAttributeType, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray (0);
	glEnableVertexAttribArray (1);
	glEnableVertexAttribArray (2);
	glEnableVertexAttribArray (3);
	glEnableVertexAttribArray (4);
}

void VAOWrapperSprite::UpdateGPU(){
    //upload vertexTexture array along with any changed data
    glBindBuffer (GL_ARRAY_BUFFER, vboVertex);
    glBufferSubData(GL_ARRAY_BUFFER,    0,      vboVertexSize, vboVertexArray.get());

    glBindBuffer (GL_ARRAY_BUFFER, vboTexture);
    glBufferSubData(GL_ARRAY_BUFFER,    0,      vboTextureSize, vboTextureArray.get());

    glBindBuffer (GL_ARRAY_BUFFER, vboColor);
    glBufferSubData(GL_ARRAY_BUFFER,    0,      vboColorSize, vboColorArray.get());

    glBindBuffer (GL_ARRAY_BUFFER, vboScalingRotation);
    glBufferSubData(GL_ARRAY_BUFFER,    0,      vboScalingRotationSize, vboScalingRotationArray.get());

    glBindBuffer (GL_ARRAY_BUFFER, vboTranslate);
    glBufferSubData(GL_ARRAY_BUFFER,    0,      vboTranslateSize, vboTranslateArray.get());
}

VAOWrapperSprite::~VAOWrapperSprite(){
    glDeleteVertexArrays(1, &vboVertex );
    glDeleteVertexArrays(1, &vboTexture);
    glDeleteVertexArrays(1, &vboColor  );
    glDeleteVertexArrays(1, &vboScalingRotation);
    glDeleteVertexArrays(1, &vboTranslate);
}

/////////////////////
//RenderSpriteBatch//
/////////////////////

RenderSpriteBatch::RenderSpriteBatch(const std::string& tex, const unsigned int& maxSize)
    : RenderableObjectWorld(), maxSprites(maxSize), textureName(tex), vao(maxSize){

    type=ROTypeSpriteBatch;
    currentSize=0;
	texture=K_TextureMan.GetItem(textureName);
	if(texture==NULL){ErrorLog::WriteToFile("ERROR: RenderSpriteBatch; Couldn't find texture named: ", textureName);}
	AddToRenderManager();
}

bool RenderSpriteBatch::CanAddSprites(const int& numSprites){
    return (numSprites + currentSize)<maxSprites;
}

void RenderSpriteBatch::AddSprite(RenderableSprite* sprite){
    sprites.insert(sprite);
    currentSize++;
}

void RenderSpriteBatch::DeleteSprite(RenderableSprite* sprite){
    auto spriteIt=sprites.find(sprite);
    if(spriteIt == sprites.end()){return;}

    sprites.erase( spriteIt );
    currentSize--;
}

void RenderSpriteBatch::Render(L_GL_Program* program){
    unsigned int numberOfSprites=0;
    unsigned int vertexIndex=0;
    RenderableSprite* sprite;
    program->Bind();
    for(auto i=sprites.begin(); i!=sprites.end(); i++){
        //Create array of correct values
        //if sprite isn't active, do not add one from 'numberOfSprites' or add its data to the array; ignore it
        sprite=(*i);
        if(sprite->isActive==false){continue;}
        numberOfSprites++;
        std::string ss;

        vao.GetVertexArray()[vertexIndex]     = sprite->data.vertex1;
        vao.GetVertexArray()[vertexIndex + 1] = sprite->data.vertex2;
        vao.GetVertexArray()[vertexIndex + 2] = sprite->data.vertex3;
        vao.GetVertexArray()[vertexIndex + 3] = sprite->data.vertex4;

        vao.GetTextureArray()[vertexIndex]     = sprite->data.texture1;
        vao.GetTextureArray()[vertexIndex + 1] = sprite->data.texture2;
        vao.GetTextureArray()[vertexIndex + 2] = sprite->data.texture3;
        vao.GetTextureArray()[vertexIndex + 3] = sprite->data.texture4;

        vao.GetColorArray()[vertexIndex]     = sprite->data.color;
        vao.GetColorArray()[vertexIndex + 1] = sprite->data.color;
        vao.GetColorArray()[vertexIndex + 2] = sprite->data.color;
        vao.GetColorArray()[vertexIndex + 3] = sprite->data.color;

        vao.GetScalingRotationArray()[vertexIndex]     = sprite->data.scalingRotation;
        vao.GetScalingRotationArray()[vertexIndex + 1] = sprite->data.scalingRotation;
        vao.GetScalingRotationArray()[vertexIndex + 2] = sprite->data.scalingRotation;
        vao.GetScalingRotationArray()[vertexIndex + 3] = sprite->data.scalingRotation;

        vao.GetTranslateArray()[vertexIndex]     = sprite->data.translate;
        vao.GetTranslateArray()[vertexIndex + 1] = sprite->data.translate;
        vao.GetTranslateArray()[vertexIndex + 2] = sprite->data.translate;
        vao.GetTranslateArray()[vertexIndex + 3] = sprite->data.translate;

        vertexIndex+=4;
    }

    vao.UpdateGPU();

    glBindVertexArray (vao.GetVAOID());
    if(texture!=NULL){texture->Bind();}
    // draw points 0-4 from the currently bound VAO with current in-use shader
    //render 'numberOfSprites' number of elements
    glDrawArrays (GL_QUADS, 0, numberOfSprites*4);
}

////////////////////////
//RenderTiledTileLayer//
////////////////////////

RenderTiledTileLayer::RenderTiledTileLayer(const TiledTileLayer *l)
    : vao(l->tileWidth * l->tileHeight){

    type=ROTypeTileLayer;
    SetDepth(l->GetDepth());
    layer=l;

    color.mA=l->GetAlpha();
    animatedRefreshRateTimer=l->GetAnimationRate();

    if(animatedRefreshRateTimer==0){animated = false;}
    else                           {animated = true; }

    //Get Texture and TiledSet to be used
    const GIDManager* gid = layer->GIDM;
    tiledSet = K_StateMan.GetCurrentState()->GetCurrentMap()->tiledData->tiledSets[layer->GetTiledSet()];
    BuildVAO();

    AddToRenderManager();
}

RenderTiledTileLayer::~RenderTiledTileLayer(){

}

void RenderTiledTileLayer::BuildVAOTile(unsigned int x, unsigned int y){
    unsigned int vertexIndex = ( (x * layer->tileWidth) + y) * 4;
    Vec2 translate;

    Vec2 topLeftVertex      ( 0.0f,  0.0f);
    Vec2 topRightVertex     (16.0f,  0.0f);
    Vec2 bottomRightVertex  (16.0f, 16.0f);
    Vec2 bottomLeftVertex   ( 0.0f, 16.0f);

    //Default y is '1' because '0%0' is undefined
    Vec2 animationVertex(0,1);

    float topTex, rightTex, leftTex, bottomTex;
    GID gid;

    Vec4 color;
    color.x=1.0f;
    color.y=1.0f;
    color.z=1.0f;
    color.w=layer->GetAlpha();

    translate.x=x*16;
    translate.y=y*16;

    gid = layer->GetGID(x,y);

    vao.GetVertexArray()[vertexIndex]     = topLeftVertex + translate;
    vao.GetVertexArray()[vertexIndex + 1] = topRightVertex + translate;
    vao.GetVertexArray()[vertexIndex + 2] = bottomRightVertex + translate;
    vao.GetVertexArray()[vertexIndex + 3] = bottomLeftVertex + translate;

    const LAnimation* animation = tiledSet->GetAnimationDataFromGID(gid);
    if(animation != NULL){
        animationVertex.x = animation->GetSpeed();
        animationVertex.y = animation->NumberOfImages();
    }

    tiledSet->GetTextureCoordinatesFromGID(gid, leftTex,rightTex,topTex,bottomTex);
    Vec2 topLeftTex     (leftTex,  topTex);
    Vec2 topRightTex    (rightTex, topTex);
    Vec2 bottomLeftTex  (rightTex, bottomTex);
    Vec2 bottomRightTex (leftTex,  bottomTex);

    vao.GetTextureArray()[vertexIndex]     = topLeftTex;
    vao.GetTextureArray()[vertexIndex + 1] = topRightTex;
    vao.GetTextureArray()[vertexIndex + 2] = bottomLeftTex;
    vao.GetTextureArray()[vertexIndex + 3] = bottomRightTex;

    vao.GetColorArray()[vertexIndex]     = color;
    vao.GetColorArray()[vertexIndex + 1] = color;
    vao.GetColorArray()[vertexIndex + 2] = color;
    vao.GetColorArray()[vertexIndex + 3] = color;

    vao.GetAnimationArray()[vertexIndex]     = animationVertex;
    vao.GetAnimationArray()[vertexIndex + 1] = animationVertex;
    vao.GetAnimationArray()[vertexIndex + 2] = animationVertex;
    vao.GetAnimationArray()[vertexIndex + 3] = animationVertex;
}

void RenderTiledTileLayer::BuildVAO(){
    for(unsigned int x=0; x!=layer->tileWidth; x++){
        for(unsigned int y=0; y!=layer->tileHeight; y++){
            BuildVAOTile(x,y);
        }
    }
    vao.UpdateGPU();
}

void RenderTiledTileLayer::BuildVAOArea(CRect area){
    for(unsigned int x=area.GetLeft(); x<=area.GetRight(); x++){
        for(unsigned int y=area.GetTop(); y<=area.GetBottom(); y++){
            BuildVAOTile(x,y);
        }
    }
    //must call vao.UpdateGPU() when done!
}

void RenderTiledTileLayer::Render(L_GL_Program* program){

    if(layer->updatedAreas.size()>0){
        for(auto i = layer->updatedAreas.begin(); i != layer->updatedAreas.end(); i++){
            BuildVAOArea((*i));
        }
        //not the best practice, clearing out the vector from the renderer, but it works
        layer->updatedAreas.clear();
        vao.UpdateGPU();
    }
    program->Bind();
    glBindVertexArray (vao.GetVAOID());
    tiledSet->GetTexture()->Bind();
    // draw points 0-4 from the currently bound VAO with current in-use shader
    // render full number of tiles
    glDrawArrays (GL_QUADS, 0, layer->tileWidth * layer->tileHeight * 4);
}


RenderTiledImageLayer::RenderTiledImageLayer(TiledImageLayer* l) : layer(l), vao(1){
    SetDepth(l->GetDepth() + 50);
    type=ROTypeImage;
    render=true;
    AddToRenderManager();
    BuildVAO();
}

void RenderTiledImageLayer::BuildVAO(){
    const LTexture* tex = layer->GetTexture();
    int tWidth = tex->GetWidth();
    int tHeight = tex->GetHeight();

    Vec2 topLeftVertex      ( 0.0f,  0.0f);
    Vec2 topRightVertex     (tWidth, 0.0f);
    Vec2 bottomRightVertex  (tWidth, tHeight);
    Vec2 bottomLeftVertex   ( 0.0f,  tHeight);

    float topTex, rightTex, leftTex, bottomTex;
    unsigned int vertexIndex=0;

    Vec4 color;
    color.x=1.0f;
    color.y=1.0f;
    color.z=1.0f;
    color.w=layer->GetAlpha();

    Vec2 translate;

    //can use this for loop to repeat the bg along the x and y axis

    vertexIndex = 0;
    //for(unsigned int x=0; x!=layer->tileWidth; x++){
        //for(unsigned int y=0; y!=layer->tileHeight; y++){
            translate.x=0;
            translate.y=480;

            vao.GetVertexArray()[vertexIndex]     = topLeftVertex + translate;
            vao.GetVertexArray()[vertexIndex + 1] = topRightVertex + translate;
            vao.GetVertexArray()[vertexIndex + 2] = bottomRightVertex + translate;
            vao.GetVertexArray()[vertexIndex + 3] = bottomLeftVertex + translate;

            Vec2 topLeftTex     (0, 0);
            Vec2 topRightTex    (1, 0);
            Vec2 bottomLeftTex  (0, 1);
            Vec2 bottomRightTex (1, 1);

            vao.GetTextureArray()[vertexIndex]     = topLeftTex;
            vao.GetTextureArray()[vertexIndex + 1] = topRightTex;
            vao.GetTextureArray()[vertexIndex + 2] = bottomLeftTex;
            vao.GetTextureArray()[vertexIndex + 3] = bottomRightTex;

            vao.GetColorArray()[vertexIndex]     = color;
            vao.GetColorArray()[vertexIndex + 1] = color;
            vao.GetColorArray()[vertexIndex + 2] = color;
            vao.GetColorArray()[vertexIndex + 3] = color;

            //vertexIndex+=4;
        //}
    //}

    vao.UpdateGPU();

    //glUseProgram (shader_programme);
    //glBindVertexArray (vao.GetVAOID());
    //if(tiledSet->GetTexture()!=NULL){tiledSet->GetTexture()->Bind();}
}

RenderTiledImageLayer::~RenderTiledImageLayer(){

}

void RenderTiledImageLayer::Render(L_GL_Program* program){
    //program->Bind();
    /*const LTexture* tex = layer->GetTexture();
    int tWidth = tex->GetWidth();
    int tHeight = tex->GetHeight();
    CRect area(0,0,tWidth,tHeight);

    tex->Bind();
    tex->BlitArea(area);
    */
    program->Bind();
    glBindVertexArray (vao.GetVAOID());
    layer->GetTexture()->Bind();
    // draw points 0-4 from the currently bound VAO with current in-use shader
    // render full number of tiles
    glDrawArrays (GL_QUADS, 0, 4);
}
//////////////
//RenderText//
//////////////

RenderText::RenderText(int xv, int yv, std::string t, bool abso) : RenderableObjectScreen(){
    x=xv;
    y=yv;
    VBOID=0;
    IBOID=0;
    mTexture=NULL;
    text=t;
    color.r=255;
    color.g=255;
    color.b=255;
    render=true;
    absolute=abso;
    del=false;
    AddToRenderManager();
}

void RenderText::DeleteTexture(){
    if(mTexture!=NULL){
        delete mTexture;
        mTexture=NULL;
    }
}

void RenderText::Render(L_GL_Program* program){
    L_GL_Program::BindNULL();
    if(del){
        DeleteTexture();
        del=false;
    }
    if(defaultFont==NULL){
            ErrorLog::WriteToFile("Font is NULL", ErrorLog::GenericLogFile);
    }
    if(mTexture==NULL){
        SDL_Surface* surface= TTF_RenderUTF8_Blended( defaultFont, text.c_str(), color );
        if(surface==NULL){
            ErrorLog::WriteToFile("Text's SDL Surface is NULL", ErrorLog::GenericLogFile);
        }
        w=surface->w;
        h=surface->h;

		mTexture=new LTexture(*surface);

        SDL_FreeSurface(surface);
    }

    if(VBOID==0) {
        // Get the coordinates of the image in the texture, expressed
        // as a value from 0 to 1.

        LVertexData2D vData[ 4 ];
        //Texture coordinates
        vData[ 0 ].texCoord.s = 0; vData[ 0 ].texCoord.t = 0;
        vData[ 1 ].texCoord.s = 1; vData[ 1 ].texCoord.t = 0;
        vData[ 2 ].texCoord.s = 1; vData[ 2 ].texCoord.t = 1;
        vData[ 3 ].texCoord.s = 0; vData[ 3 ].texCoord.t = 1;
		//Vertex positions
        vData[ 0 ].position.x = 0;      vData[ 0 ].position.y = 0;
        vData[ 1 ].position.x = w;      vData[ 1 ].position.y = 0;
        vData[ 2 ].position.x = w;      vData[ 2 ].position.y = h;
        vData[ 3 ].position.x = 0;      vData[ 3 ].position.y = h;

        //LVertexData2D newData[ 4 ];
        GLuint iData[4];
        //Set rendering indices
        iData[0] = 0;
        iData[1] = 1;
        iData[2] = 2;
        iData[3] = 3;
        //Create VBO
        glGenBuffers( 1, &VBOID );
        glBindBuffer( GL_ARRAY_BUFFER, VBOID );
        glBufferData( GL_ARRAY_BUFFER, 4 * sizeof(LVertexData2D), vData, GL_DYNAMIC_DRAW );
        //Create IBO
        glGenBuffers( 1, &IBOID );
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, IBOID );
        glBufferData( GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(GLuint), iData, GL_DYNAMIC_DRAW );
        //Unbind buffers
        //glBindBuffer( GL_ARRAY_BUFFER, NULL ); glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, NULL );
    }
    glPushMatrix();
    //glScalef(GetScalingX(), GetScalingY(),1);
    glScalef(1, 1,1);
    //glTranslatef(x,y,0);
    //if(absolute){
        glTranslatef(x,y,0);
    //}
    //else{
        //glTranslatef(x,y,0);
        //glTranslatef(viewPort.x+x, viewPort.y+y, 0);
    //}
    glRotatef(0.0f, 0.0f, 0.0f, 1.0f);

    mTexture->Bind();
        //Enable vertex and texture coordinate arrays
        //glEnableClientState( GL_VERTEX_ARRAY );
        //glEnableClientState( GL_TEXTURE_COORD_ARRAY );
         //Bind vertex buffer
         glBindBuffer( GL_ARRAY_BUFFER, VBOID);
         //Update vertex buffer data
         //glBufferSubData( GL_ARRAY_BUFFER, 0, 4 * sizeof(LVertexData2D), vData );

         //Set texture coordinate data
         glTexCoordPointer( 2, GL_FLOAT, sizeof(LVertexData2D), (GLvoid*)offsetof( LVertexData2D, texCoord ) );
         //Set vertex data
         glVertexPointer( 2, GL_FLOAT, sizeof(LVertexData2D), (GLvoid*)offsetof( LVertexData2D, position ) );

         glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
         glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, IBOID );
         glDrawElements( GL_QUADS, 4, GL_UNSIGNED_INT, NULL );

        //glDisableClientState( GL_TEXTURE_COORD_ARRAY );
        //glDisableClientState( GL_VERTEX_ARRAY );

        /*glBegin(GL_QUADS);
		glTexCoord2f(0,0);		glVertex3i(0,0,0);
		glTexCoord2f(1,0);	glVertex3i(w,0,0);
		glTexCoord2f(1,1);	glVertex3i(w,h,0);
		glTexCoord2f(0,1);	glVertex3i(0,h,0);
		glEnd();*/

    glPopMatrix();
}

RenderText::~RenderText(){
    //Delete texture
    DeleteTexture();
    if( VBOID != 0 ){
        glDeleteBuffers( 1, &VBOID );
        glDeleteBuffers( 1, &IBOID );
    }
}

//////////////
//RenderLine//
//////////////

RenderLine::RenderLine(int x1, int y1, int x2, int y2, bool absolute) : RenderableObjectScreen(){
    mX1=x1;
    mY1=y1;
    mX2=x2;
    mY2=y2;
    color.r=255;
    color.g=0;
    color.b=0;
    render=true;
    mAbsolute=absolute;
    AddToRenderManager();
}

void RenderLine::Render(L_GL_Program* program){
    glBindTexture(GL_TEXTURE_2D, 0);

    double R, G, B;
    R=(double(color.r))/255.0f;
    G=(double(color.g))/255.0f;
    B=(double(color.b))/255.0f;

        glLineWidth(1.0f);
        glColor4f(R, G, B, 1.0f);
        glBegin(GL_LINES);
            glVertex2f(mX1, mY1);
            glVertex2f(mX2, mY2);
        glEnd();
}

RenderLine::~RenderLine(){
}

/////////////
//RenderBox//
/////////////

RenderBox::RenderBox(int x1, int y1, int x2, int y2, bool absolute){
    mX1=x1;
    mY1=y1;
    mX2=x2;
    mY2=y2;
    color.r=255;
    color.g=0;
    color.b=0;
    render=true;
    mAbsolute=absolute;
    AddToRenderManager();
}
RenderBox::~RenderBox(){}

void RenderBox::Render(L_GL_Program* program){
    glBindTexture(GL_TEXTURE_2D, 0);

    double R, G, B;
    R=(double(color.r))/255.0f;
    G=(double(color.g))/255.0f;
    B=(double(color.b))/255.0f;

        glLineWidth(1.0f);
        glColor4f(R, G, B, 1.0f);
        glBegin(GL_QUADS);
            glVertex2f(mX1, mY1);
            glVertex2f(mX2, mY1);
            glVertex2f(mX2, mY2);
            glVertex2f(mX1, mY2);
        glEnd();
}


////////////////
//RenderCamera//
////////////////

RenderCamera::RenderCamera()
    : frameBufferTextureDiffuse(std::unique_ptr<LTexture>(new LTexture(SCREEN_W, SCREEN_H, 4, GL_RGBA)))
    , frameBufferTextureFinal  (std::unique_ptr<LTexture>(new LTexture(SCREEN_W, SCREEN_H, 4, GL_RGBA))){
    scale=1;
    rotation=0;
    view.x=0;
    view.y=0;
    view.w=CAMERA_W;
    view.h=CAMERA_H;

    glGenFramebuffers(1, &FBO);

    Kernel::stateMan.GetCurrentState()->renderMan.AddCamera( this);
}
void RenderCamera::Bind(const GLuint& GlobalCameraUBO){
    //For the purpose of scaling and rotating the viewport, I may want to replace this vec4 with a projection matrix;
    /*Matrix4 T =translate (identity_mat4 (), vec3 (-view.x, -view.y, 0.0));
    Matrix4 R = rotate_z_deg (identity_mat4 (), 0.0f);
    Matrix4 view_mat = R * T;*/

    Matrix4 T= Matrix4::IdentityMatrix();
    T=T.Translate(Vec3 (-view.x, -view.y, 0.0));

    Matrix4 R= Matrix4::IdentityMatrix();
    R=R.RotateZ(0);

    Matrix4 S= Matrix4::IdentityMatrix();
    S= S.Scale(Vec3(4,4,1));


    //Create orthographic matrix
    // http://www.scratchapixel.com/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix/orthographic-projection-matrix

    //down directions is negative y; up direction is positive y
    //y gets flipped twice, once here, and once when applying the framebuffer
    float rightSide   = CAMERA_W;;// - 1.0f;
    float leftSide    = 0;
    float bottomSide  = 0;
    float topSide     = CAMERA_H;// - 1.0f;
    float zFar        = 1.1f;
    float zNear       = 0.1f;

    float normalizedDeviceSpaceX =  2.0f / (rightSide  - leftSide);
    float normalizedDeviceSpaceY =  2.0f / (topSide    - bottomSide);
    float normalizedDeviceSpaceZ =  2.0f / (zFar       - zNear);

    Matrix4 view_mat = T * R * S;
    Matrix4 proj_mat = Matrix4::IdentityMatrix();

    proj_mat.m[ 0] = normalizedDeviceSpaceX;
    proj_mat.m[ 5] = normalizedDeviceSpaceY;
    proj_mat.m[10] = normalizedDeviceSpaceZ;
    proj_mat.m[12] = -(rightSide+leftSide)/(rightSide-leftSide);
    proj_mat.m[13] =  -(topSide+bottomSide)/(topSide-bottomSide);
    proj_mat.m[14] =  (zNear+zFar)/(zNear-zFar);
    proj_mat.m[15] =  1.0f;

    /*proj_mat.m[ 0] =  normalizedDeviceSpaceX;
    proj_mat.m[ 5] =  normalizedDeviceSpaceY;
    proj_mat.m[10] = -2.0f;

    proj_mat.m[12] = -1.0f;
    proj_mat.m[13] =  -1.0f;
    proj_mat.m[14] = 1.0f;
    proj_mat.m[15] =  1.0f;*/

    float position[4];
    //Render Using only full integers for translation to get that pixel-perfect look
    position[0]=view.x;
    position[1]=view.y;
    position[2]=CAMERA_W;
    position[3]=CAMERA_H;

    glBindBuffer(GL_UNIFORM_BUFFER, GlobalCameraUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0,                 (sizeof(float)*4),     position);
    glBufferSubData(GL_UNIFORM_BUFFER, (sizeof(float)*4), (sizeof(float)*16),  proj_mat.m);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);


    //Push viewport bit
    glPushAttrib(GL_VIEWPORT_BIT);
    //Setup frame buffer render
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    //Set Render Viewport
    glViewport(0,0, frameBufferTextureDiffuse->GetWidth(), frameBufferTextureDiffuse->GetHeight());

	//Clear Background
	glClearColor(1.0,1.0,1.0,0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    //Atatch buffer texture
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frameBufferTextureDiffuse->GetOpenGLID(), 0);
}
RenderCamera::~RenderCamera(){
    Kernel::stateMan.GetCurrentState()->renderMan.RemoveCamera(this);
}

void RenderCamera::RenderFrameBufferTextureFinal(){
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    L_GL_Program::BindNULL();
    glBindVertexArray (NULL);

    //Back to initial viewport
    glPopAttrib();

    frameBufferTextureFinal.get()->Bind();
    float Left=     (float)0.0f     / (float)frameBufferTextureFinal->GetWidth();
    float Right=    (float)SCREEN_W / (float)frameBufferTextureFinal->GetWidth();
    float Top=      (float)0.0f     / (float)frameBufferTextureFinal->GetHeight();
    float Bottom=   (float)SCREEN_H / (float)frameBufferTextureFinal->GetHeight();

    glBegin(GL_QUADS);
        glTexCoord2f(Left,  Top);		glVertex3i(0,        0,        0);
        glTexCoord2f(Right, Top);	    glVertex3i(CAMERA_W, 0,        0);
        glTexCoord2f(Right, Bottom);	glVertex3i(CAMERA_W, CAMERA_H, 0);
        glTexCoord2f(Left,  Bottom);	glVertex3i(0,        CAMERA_H, 0);
    glEnd();
}


/////////////////
//RenderManager//
/////////////////
bool LOrderOBJs(RenderableObject* r1, RenderableObject* r2){
    if(r1->GetDepth() <= r2->GetDepth()){return false;}
    return true;
}


GLuint RenderManager::GlobalCameraUBO=0;
GLuint RenderManager::GlobalProgramUBO=0;

RenderManager::RenderManager()
    : CameraDataBindingIndex(1), ProgramDataBindingIndex(2), timeElapsed(0){
    listChange=false;
    nextTextID=0;

    //All of this init code needs moved somewhere now that RenderManager isn't a singleton.

    shaderFragmentNameSpriteBatch  = "Data/Resources/Shaders/fragmentSpriteMain.glsl";
    shaderVertexNameSpriteBatch    = "Data/Resources/Shaders/vertexSpriteMain.glsl";

    std::unique_ptr<const L_GL_Shader> shaderFragmentSpriteBatch (new L_GL_Shader(L_GL_Shader::LoadShaderFromFile(shaderFragmentNameSpriteBatch), SHADER_FRAGMENT));
    std::unique_ptr<const L_GL_Shader> shaderVertexSpriteBatch   (new L_GL_Shader(L_GL_Shader::LoadShaderFromFile(shaderVertexNameSpriteBatch),   SHADER_VERTEX ));

    if(shaderFragmentSpriteBatch->GetShaderID()!=0) {K_ShaderMan.LoadItem(shaderFragmentNameSpriteBatch, shaderFragmentSpriteBatch);}
    if(shaderVertexSpriteBatch->GetShaderID()!=0)   {K_ShaderMan.LoadItem(shaderVertexNameSpriteBatch, shaderVertexSpriteBatch);}

    shaderProgramSpriteBatch.AddShader(K_ShaderMan.GetItem(shaderFragmentNameSpriteBatch));
    shaderProgramSpriteBatch.AddShader(K_ShaderMan.GetItem(shaderVertexNameSpriteBatch));
    shaderProgramSpriteBatch.LinkProgram();
    shaderProgramSpriteBatch.Bind();



    shaderFragmentNameTileLayer  = "Data/Resources/Shaders/fragmentTileMain.glsl";
    shaderVertexNameTileLayer    = "Data/Resources/Shaders/vertexTileMain.glsl";

    std::unique_ptr<const L_GL_Shader> shaderFragmentTileLayer(new L_GL_Shader(L_GL_Shader::LoadShaderFromFile(shaderFragmentNameTileLayer), SHADER_FRAGMENT));
    std::unique_ptr<const L_GL_Shader> shaderVertexTileLayer  (new L_GL_Shader(L_GL_Shader::LoadShaderFromFile(shaderVertexNameTileLayer),   SHADER_VERTEX ));

    if(shaderFragmentTileLayer->GetShaderID()!=0) {K_ShaderMan.LoadItem(shaderFragmentNameTileLayer, shaderFragmentTileLayer);}
    if(shaderVertexTileLayer->GetShaderID()!=0)   {K_ShaderMan.LoadItem(shaderVertexNameTileLayer, shaderVertexTileLayer);}

    shaderProgramTileLayer.AddShader(K_ShaderMan.GetItem(shaderFragmentNameTileLayer));
    shaderProgramTileLayer.AddShader(K_ShaderMan.GetItem(shaderVertexNameTileLayer));
    shaderProgramTileLayer.LinkProgram();
    shaderProgramTileLayer.Bind();


    shaderFragmentNameImage  = "Data/Resources/Shaders/fragmentImage.glsl";
    shaderVertexNameImage    = "Data/Resources/Shaders/vertexImage.glsl";

    std::unique_ptr<const L_GL_Shader> shaderFragmentImage(new L_GL_Shader(L_GL_Shader::LoadShaderFromFile(shaderFragmentNameImage), SHADER_FRAGMENT));
    std::unique_ptr<const L_GL_Shader> shaderVertexImage  (new L_GL_Shader(L_GL_Shader::LoadShaderFromFile(shaderVertexNameImage),   SHADER_VERTEX ));

    if(shaderFragmentImage->GetShaderID()!=0) {K_ShaderMan.LoadItem(shaderFragmentNameImage, shaderFragmentImage);}
    if(shaderVertexImage->GetShaderID()!=0)   {K_ShaderMan.LoadItem(shaderVertexNameImage, shaderVertexImage);}

    shaderProgramImage.AddShader(K_ShaderMan.GetItem(shaderFragmentNameImage));
    shaderProgramImage.AddShader(K_ShaderMan.GetItem(shaderVertexNameImage));
    shaderProgramImage.LinkProgram();
    shaderProgramImage.Bind();



    shaderFragmentNameLight  = "Data/Resources/Shaders/fragmentLightMain.glsl";
    shaderVertexNameLight    = "Data/Resources/Shaders/vertexLightMain.glsl";

    std::unique_ptr<const L_GL_Shader> shaderFragmentLight(new L_GL_Shader(L_GL_Shader::LoadShaderFromFile(shaderFragmentNameLight), SHADER_FRAGMENT));
    std::unique_ptr<const L_GL_Shader> shaderVertexLight  (new L_GL_Shader(L_GL_Shader::LoadShaderFromFile(shaderVertexNameLight),   SHADER_VERTEX ));

    if(shaderFragmentLight->GetShaderID()!=0) {K_ShaderMan.LoadItem(shaderFragmentNameLight, shaderFragmentLight);}
    if(shaderVertexLight->GetShaderID()!=0)   {K_ShaderMan.LoadItem(shaderVertexNameLight, shaderVertexLight);}

    shaderProgramLight.AddShader(K_ShaderMan.GetItem(shaderFragmentNameLight));
    shaderProgramLight.AddShader(K_ShaderMan.GetItem(shaderVertexNameLight));
    shaderProgramLight.LinkProgram();
    shaderProgramLight.Bind();


    //this line keeps from reinitialize the GPU data if it's already been set
    //All this init code needs moved/changed now that RenderManager isn't a singleton
    if(GlobalCameraUBO!=0){return;}

    //Create memory location on GPU to store uniform camera data for ALL SHADER PROGRAMS
    //The memory location ID is then sent to each individual camera so that the cameras can bind
    //the needed data into the uniform buffer
    //This buffer stores a mat4 (proj matrix) and a vec2 (camera translation)
                        //2 floats (padded to vec4)for vec2, 16 for matrix
    GLuint bufferSize=(sizeof(float)*4) + (sizeof(float)*16);
    glGenBuffers(1, &GlobalCameraUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, GlobalCameraUBO);
    glBufferData(GL_UNIFORM_BUFFER, bufferSize, NULL, GL_DYNAMIC_DRAW);
    //Bind generated CPU buffer to the index
    glBindBufferBase(GL_UNIFORM_BUFFER, CameraDataBindingIndex, GlobalCameraUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);


    if(GlobalProgramUBO!=0){return;}
    //Create memory location on GPU to store uniform data for ALL SHADER PROGRAMS
    //The memory location ID is then sent to each individual camera so that the cameras can bind
    //the needed data into the uniform buffer
        //This buffer stores the elapsed time
    bufferSize=(sizeof(float)*4);
    glGenBuffers(1, &GlobalProgramUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, GlobalProgramUBO);
    glBufferData(GL_UNIFORM_BUFFER, bufferSize, NULL, GL_DYNAMIC_DRAW);
    //Bind generated CPU buffer to the index
    glBindBufferBase(GL_UNIFORM_BUFFER, ProgramDataBindingIndex, GlobalProgramUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    //Link uniform buffer location with each program
    //Sprite batch program
    GLuint programHandle            = shaderProgramSpriteBatch.GetHandle();
    GLuint programUniformBlockHandle= shaderProgramSpriteBatch.GetUniformBlockHandle("CameraData");
    //Bind program GPU buffer to the index
    glUniformBlockBinding(programHandle, programUniformBlockHandle, CameraDataBindingIndex);
    try{
        GLuint programUniformBlockHandleProgramData= shaderProgramSpriteBatch.GetUniformBlockHandle("ProgramData");
        glUniformBlockBinding(programHandle, programUniformBlockHandleProgramData, ProgramDataBindingIndex);
    }
    catch(LEngineShaderProgramException e){
        //if the program doesn't use the program data block, the compiled code won't have one
    }


    //Tile layer shader shader setup
    programHandle            = shaderProgramTileLayer.GetHandle();
    programUniformBlockHandle= shaderProgramTileLayer.GetUniformBlockHandle("CameraData");
    //Bind program GPU buffer to the index
    glUniformBlockBinding(programHandle, programUniformBlockHandle, CameraDataBindingIndex);
    try{
        GLuint programUniformBlockHandleProgramData= shaderProgramTileLayer.GetUniformBlockHandle("ProgramData");
        glUniformBlockBinding(programHandle, programUniformBlockHandleProgramData, ProgramDataBindingIndex);
    }
    catch(LEngineShaderProgramException e){
        //if the program doesn't use the program data block, the compiled code won't have one
    }


    //Image layer shader shader setup
    programHandle            = shaderProgramImage.GetHandle();
    programUniformBlockHandle= shaderProgramImage.GetUniformBlockHandle("CameraData");
    //Bind program GPU buffer to the index
    glUniformBlockBinding(programHandle, programUniformBlockHandle, CameraDataBindingIndex);
    try{
        GLuint programUniformBlockHandleProgramData= shaderProgramImage.GetUniformBlockHandle("ProgramData");
        glUniformBlockBinding(programHandle, programUniformBlockHandleProgramData, ProgramDataBindingIndex);
    }
    catch(LEngineShaderProgramException e){
        //if the program doesn't use the program data block the compiled code won't have one
    }

    //Light shader shader setup
    programHandle            = shaderProgramLight.GetHandle();
    programUniformBlockHandle= shaderProgramLight.GetUniformBlockHandle("CameraData");
    //Bind program GPU buffer to the index
    glUniformBlockBinding(programHandle, programUniformBlockHandle, CameraDataBindingIndex);
    try{
        GLuint programUniformBlockHandleProgramData= shaderProgramLight.GetUniformBlockHandle("ProgramData");
        glUniformBlockBinding(programHandle, programUniformBlockHandleProgramData, ProgramDataBindingIndex);
    }
    catch(LEngineShaderProgramException e){
        //if the program doesn't use the program data block, the compiled code won't have one
    }

}

void RenderManager::OrderOBJs(){
    //Negative Depth is closer to the screen
    objectsScreen.sort(&LOrderOBJs);
    objectsWorld.sort(&LOrderOBJs);
    listChange=false;
}

void RenderManager::Render(){
    timeElapsed += 1;
    glBindBuffer(GL_UNIFORM_BUFFER, GlobalProgramUBO);
    float values [4];
    values[0] = timeElapsed;
    values[1] = 0;
    values[2] = 0;
    values[3] = 0;

    //doesn't look like this is updating
    glBufferSubData(GL_UNIFORM_BUFFER, 0, (sizeof(float)*4), &values);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    if(listChange){
         OrderOBJs(); //Sort by Depth
    }

    glLoadIdentity();
    //New Rendering

    if(!(mCameras).empty()){
        //World objects are rendered after the camera sets its matrix (this includes sprite batches)
        auto currentCamera = mCameras.begin();
        (*currentCamera)->Bind(GlobalCameraUBO);
        for(auto i=objectsWorld.begin(); i!=objectsWorld.end(); i++){
            if((*i)->render){(*i)->Render();}
        }

        Coord2d pos((*currentCamera)->GetX(), (*currentCamera)->GetY());
        Kernel::stateMan.GetCurrentState()->comLightMan.Render((*currentCamera)->GetFrameBufferTextureDiffuse(), (*currentCamera)->GetFrameBufferTextureFinal(), pos, &shaderProgramLight);

        (*currentCamera)->RenderFrameBufferTextureFinal();
    }

    //Screen objects are rendered after others, but do not use the camera's matrix
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    L_GL_Program::BindNULL();
    glBindVertexArray (NULL);

    for(auto i=objectsScreen.begin(); i!=objectsScreen.end(); i++){
        if((*i)->render){(*i)->Render();}
    }
}

void RenderManager::AssignCameraUBO(L_GL_Program* program){
    //Get program and uniform buffer handles
    GLuint programHandle            = program->GetHandle();
    GLuint programUniformBlockHandle= program->GetUniformBlockHandle("CameraData");

    //Bind program GPU buffer to the index
    glUniformBlockBinding(programHandle, programUniformBlockHandle, CameraDataBindingIndex);
}

RenderSpriteBatch* RenderManager::GetSpriteBatch(const std::string& textureName, const MAP_DEPTH& depth, const int& numSprites){
    auto textureMapIt= spriteBatchMap.find(depth); //If there isn't a map for this depth value, create it and assign texturemapit to it
    if(textureMapIt==spriteBatchMap.end()){
        spriteBatchMap[depth];
        textureMapIt= spriteBatchMap.find(depth);
    }

    auto spriteBatchVectorIt = textureMapIt->second.find(textureName); //If there isn't a vector for this texture name value, then create it and assign spriteBatchVectorIt to it
    if(spriteBatchVectorIt == textureMapIt->second.end()){
       textureMapIt->second[textureName];
       spriteBatchVectorIt = textureMapIt->second.find(textureName);
    }

    RenderSpriteBatch* batch=NULL;
    for(auto vectorIt = spriteBatchVectorIt->second.begin(); vectorIt!= spriteBatchVectorIt->second.end(); vectorIt++){
        if((*vectorIt)->CanAddSprites(numSprites)==true){batch=(*vectorIt).get(); break;}
    }

    //If there isn't a spritebatch of appropriate size, create one and assign batch to it
    if(batch==NULL){
        //Max size 256
        spriteBatchVectorIt->second.push_back( std::unique_ptr<RenderSpriteBatch>(new RenderSpriteBatch(textureName, 256)));
        batch=(spriteBatchVectorIt->second.back()).get();
        batch->SetDepth(depth);
    }
    return batch;
}

void RenderManager::RemoveObjectScreen(RenderableObjectScreen* obj){
    for(auto i=objectsScreen.begin(); i!=objectsScreen.end(); i++){
        if(obj==*i){
            objectsScreen.erase(i);
            listChange=true;
            break;
        }
    }
}

void RenderManager::RemoveObjectWorld(RenderableObjectWorld* obj){
    for(auto i=objectsWorld.begin(); i!=objectsWorld.end(); i++){
        if(obj==*i){
            objectsWorld.erase(i);
            listChange=true;
            break;
        }
    }
}

void RenderManager::AddObjectWorld(RenderableObjectWorld* obj){
    objectsWorld.push_back(obj);
    listChange=true;

    //Set Correct Shader
    if(obj->type==ROTypeSpriteBatch){
        obj->SetShaderProgram(&shaderProgramSpriteBatch);
    }
    if(obj->type==ROTypeTileLayer){
        obj->SetShaderProgram(&shaderProgramTileLayer);
        //obj->BuildVAO();
    }
    if(obj->type==ROTypeImage){
        obj->SetShaderProgram(&shaderProgramImage);
        //obj->BuildVAO();
    }
}

void RenderManager::AddObjectScreen(RenderableObjectScreen* obj){
    objectsScreen.push_back(obj);
    listChange=true;
}

bool RenderManager::AddCamera(RenderCamera* cam){
    mCameras.insert(cam);
    return true;
}

void RenderManager::RemoveCamera(RenderCamera* cam){
    mCameras.erase(cam);
}

void RenderManager::MoveCameraX(int x){
    //mCurrentCamera->view.x+=x;
}
void RenderManager::MoveCameraY(int y){
    //mCurrentCamera->view.y+=y;
}

void RenderManager::MoveCameraXAbs(int x){
    //mCurrentCamera->view.x=x;
}
void RenderManager::MoveCameraYAbs(int y){
    //mCurrentCamera->view.y=y;
}

