#include "RenderImageLayer.h"


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

///////////////////
//TiledImageLayer//
///////////////////
RenderImageLayer::RenderImageLayer(RenderManager* rm, TiledImageLayer* l)
    : RenderableObjectWorld(rm, RenderableObject::TYPE::Image), layer(l), vao(1){
    SetDepth(l->GetDepth() + 50);
    render=true;
    AddToRenderManager();
    BuildVAO();
}

void RenderImageLayer::BuildVAO(){
    const RSC_Texture* tex = layer->GetTexture();
    int tWidth = tex->GetWidth();
    int tHeight = tex->GetHeight();

    Vec2 topLeftVertex      ( 0.0f,  0.0f);
    Vec2 topRightVertex     (tWidth, 0.0f);
    Vec2 bottomRightVertex  (tWidth, tHeight);
    Vec2 bottomLeftVertex   ( 0.0f,  tHeight);

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

RenderImageLayer::~RenderImageLayer(){

}

void RenderImageLayer::Render(const RSC_GLProgram* program){
    //program->Bind();
    /*const RSC_Texture* tex = layer->GetTexture();
    int tWidth = tex->GetWidth();
    int tHeight = tex->GetHeight();
    CRect area(0,0,tWidth,tHeight);

    tex->Bind();
    tex->BlitArea(area);
    */

	//Calculate a projection matrix here based on distance from the camera?

    program->Bind();
    glBindVertexArray (vao.GetVAOID());
    layer->GetTexture()->Bind();
    // draw points 0-4 from the currently bound VAO with current in-use shader
    // render full number of tiles
    glDrawArrays (GL_QUADS, 0, 4);
}

