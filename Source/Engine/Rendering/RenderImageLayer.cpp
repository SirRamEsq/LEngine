#include "RenderImageLayer.h"
#include "../RenderManager.h"


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
    BuildVAO(CRect(0,0,0,0));
}

void RenderImageLayer::BuildVAO(CRect camera){
    const RSC_Texture* tex = layer->GetTexture();
    int tWidth = tex->GetWidth();
    int tHeight = tex->GetHeight();

    Vec2 topLeftVertex      ( 0.0f, 0.0f);
    Vec2 topRightVertex     ( 1.0f, 0.0f);
    Vec2 bottomRightVertex  ( 1.0f, 1.0f);
    Vec2 bottomLeftVertex   ( 0.0f, 1.0f);

    Vec4 color;
    color.x=1.0f;
    color.y=1.0f;
    color.z=1.0f;
    color.w=layer->GetAlpha();

	auto parallax = layer->GetParallax();
	float parallaxX = parallax.x;
	float parallaxY = parallax.y;

    auto translate = layer->GetOffset();

	//How many full images are contained within the camera viewport
	int fullX = camera.w / tex->GetWidth();
	int fullY = camera.h / tex->GetHeight();

	Vec2 topLeftTex     (   int(floor(camera.GetLeft()  * parallaxX)+translate.x) % tex->GetWidth(),        ( int(floor( camera.GetTop() * parallaxY)+translate.y) % tex->GetHeight()) );
	Vec2 topRightTex    ( ( int(floor(camera.GetRight() * parallaxX)+translate.x) % tex->GetWidth())+fullX, ( int(floor( camera.GetTop() * parallaxY)+translate.y) % tex->GetHeight()) );
	Vec2 bottomLeftTex  (   int(floor(camera.GetLeft()  * parallaxX)+translate.x) % tex->GetWidth(),        ( int(floor( camera.GetBottom() * parallaxY)+translate.y) % tex->GetHeight())+fullY );
	Vec2 bottomRightTex ( ( int(floor(camera.GetRight() * parallaxX)+translate.x) % tex->GetWidth())+fullX, ( int(floor( camera.GetBottom() * parallaxY)+translate.y) % tex->GetHeight())+fullY );

	int vertexIndex=0;
	vao.GetVertexArray()[vertexIndex]     = topLeftVertex;
	vao.GetVertexArray()[vertexIndex + 1] = topRightVertex;
	vao.GetVertexArray()[vertexIndex + 2] = bottomRightVertex;
	vao.GetVertexArray()[vertexIndex + 3] = bottomLeftVertex;

	vao.GetTextureArray()[vertexIndex]     = topLeftTex;
	vao.GetTextureArray()[vertexIndex + 1] = topRightTex;
	vao.GetTextureArray()[vertexIndex + 2] = bottomLeftTex;
	vao.GetTextureArray()[vertexIndex + 3] = bottomRightTex;

	vao.GetColorArray()[vertexIndex]     = color;
	vao.GetColorArray()[vertexIndex + 1] = color;
	vao.GetColorArray()[vertexIndex + 2] = color;
	vao.GetColorArray()[vertexIndex + 3] = color;

    vao.UpdateGPU();
}

RenderImageLayer::~RenderImageLayer(){

}

void RenderImageLayer::Render(const RenderCamera* camera, const RSC_GLProgram* program){
	//Store old texture wrap settings
	GLint oldParamS;
	GLint oldParamT;
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, &oldParamS); 
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, &oldParamT); 

	//Set new texture wrap settings
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//Calculate a projection matrix here based on distance from the camera?
	BuildVAO(camera->GetView());

	//Bind everything
    program->Bind();
    glBindVertexArray (vao.GetVAOID());
    layer->GetTexture()->Bind();
    glDrawArrays (GL_QUADS, 0, 4);

	//Restore old texture Wrap Settings
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, oldParamS);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, oldParamT);
}

