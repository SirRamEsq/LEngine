#include "../Kernel.h"
#include "RenderPrimitives.h"
#include "../Resources/RSC_Texture.h"

//////////////
//RenderText//
//////////////

RenderText::RenderText(RenderManager* rm, int xv, int yv, std::string t, bool abso) : RenderableObjectScreen(rm){
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

void RenderText::Render(const RenderCamera* camera, const RSC_GLProgram* program){
    RSC_GLProgram::BindNULL();
    if(del){
        DeleteTexture();
        del=false;
    }
    if(defaultFont==NULL){
            LOG_ERROR("Font is NULL");
    }
    if(mTexture==NULL){
        SDL_Surface* surface= TTF_RenderUTF8_Blended( defaultFont, text.c_str(), color );
        if(surface==NULL){
            LOG_ERROR("Text's SDL Surface is NULL");
        }
        w=surface->w;
        h=surface->h;

		mTexture=new RSC_Texture(*surface);

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

RenderLine::RenderLine(RenderManager* rm, int x1, int y1, int x2, int y2, bool absolute) : RenderableObjectScreen(rm){
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

void RenderLine::Render(const RenderCamera* camera, const RSC_GLProgram* program){
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

RenderBox::RenderBox(RenderManager* rm, int x1, int y1, int x2, int y2, bool absolute)
 : RenderableObjectScreen(rm){
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

void RenderBox::Render(const RenderCamera* camera, const RSC_GLProgram* program){
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

