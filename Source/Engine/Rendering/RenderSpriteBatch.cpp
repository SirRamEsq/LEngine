#include "RenderSpriteBatch.h"
#include "../Kernel.h"

////////////////////
//RenderableSprite//
////////////////////

RenderSpriteBatch::Sprite::Sprite(const std::string& texture, const unsigned int& w, const unsigned int& h, const MAP_DEPTH& d, const Vec2& off)
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

RenderSpriteBatch::Sprite::~Sprite(){
    spriteBatch->DeleteSprite(this);
}


/////////////////////
//RenderSpriteBatch//
/////////////////////

RenderSpriteBatch::RenderSpriteBatch(RenderManager* rm, const std::string& tex, const unsigned int& maxSize)
    : RenderableObjectWorld(rm, RenderableObject::TYPE::SpriteBatch), maxSprites(maxSize), textureName(tex), vao(maxSize){

    currentSize=0;
	//this dependency on the Kernel is ok, this is just to grab a resource
	texture=K_TextureMan.GetItem(textureName);
	if(texture==NULL){ErrorLog::WriteToFile("ERROR: RenderSpriteBatch; Couldn't find texture named: ", textureName);}
	AddToRenderManager();
}

bool RenderSpriteBatch::CanAddSprites(const int& numSprites){
    return (numSprites + currentSize)<maxSprites;
}

void RenderSpriteBatch::AddSprite(RenderSpriteBatch::Sprite* sprite){
    sprites.insert(sprite);
    currentSize++;
}

void RenderSpriteBatch::DeleteSprite(RenderSpriteBatch::Sprite* sprite){
    auto spriteIt=sprites.find(sprite);
    if(spriteIt == sprites.end()){return;}

    sprites.erase( spriteIt );
    currentSize--;
}

void RenderSpriteBatch::Render(L_GL_Program* program){
    unsigned int numberOfSprites=0;
    unsigned int vertexIndex=0;
    RenderSpriteBatch::Sprite* sprite;
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

