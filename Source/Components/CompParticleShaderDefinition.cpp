#include "CompParticle.h"

const std::string PARTICLE_SHADER_VERTEX_DECLARATIONS=
"\n"
"//VERTEX SHADER\n"
"#version 300 es\n"

"// the time since the creation of the particular particle system\n"
"uniform float time;\n"

"layout (location = 0) in vec2 position;\n"
"layout (location = 1) in vec2 textureCoords;\n"
"layout (location = 2) in vec2 velocity;\n"
"layout (location = 3) in vec2 acceleration;\n"
"layout (location = 4) in vec4 color;\n"

"//lifeTime.x = timeStamp when particle was created, lifeTime.y = timeStamp to be destroyed\n"
"layout (location = 5) in vec2 lifetime;\n"

"//Mostly used for fancy effects (to know what the width/height of an object is), not actual scaling\n"
"//these values are from the center of the polygon to the end (only half the width/height of the poly)\n"
"layout (location = 6) in vec2 scaling;\n"

"layout(std140) uniform CameraData{\n"
"//Only camera translation is needed\n"
"//Camera scaling and rotation will take place when the camera applies it's buffer texutre (that it renderes the screen to) to a viewport\n"
    "vec4 cameraTranslation;\n"
    "mat4 projMatrix;\n"
"};\n"

"out vec4 fragmentColor;\n"
"out vec2 fragmentUV;\n"
;

const std::string PARTICLE_SHADER_VERTEX_MAIN_BEGIN=
"void main(){\n"
	"//if (particleAlpha <= 0.0)\n"
	"//	return;\n"
	"//Time since particle was created\n"
	"float timeOffset = (time - lifetime.x);\n"
	"// calculate the current position of the particle\n"
	"vec2 particlePosition = position\n"
				"+ velocity * timeOffset\n"
				"+ acceleration * (timeOffset * timeOffset * 0.5);\n"

	"// calculate the opacity of the particle\n"
	"// (does not have to be clamped to 0 since we are discarding\n"
	"//  non-positive alpha particles in the geometry shader)\n"
	"//float particleAlpha = min(1.0, (time / lifetime.y) * -2.0);\n"
	"float timeRatio    = (timeOffset) / (lifetime.y - lifetime.x);\n"
	"fragmentColor	    = color;\n"
	"fragmentColor.a	= timeRatio;\n"
	"fragmentUV=textureCoords;\n"

	"vec2 temp=particlePosition;\n"
	"temp.x = particlePosition.x - cameraTranslation.x;\n"
	"temp.y = particlePosition.y - cameraTranslation.y;\n"
;

const std::string PARTICLE_SHADER_VERTEX_MAIN_LUASTRING_BEGIN=
    ""
;

const std::string PARTICLE_SHADER_VERTEX_MAIN_LUASTRING_EFFECT_SHRINK=
    "temp.x = temp.x - (timeRatio * scaling.x * textureCoords.x);\n"
    "temp.y = temp.y - (timeRatio * scaling.y * textureCoords.y);\n"
;

const std::string PARTICLE_SHADER_VERTEX_MAIN_LUASTRING_EFFECT_EXPAND=
    "temp.x = temp.x + (timeRatio * scaling.x * textureCoords.x);\n"
    "temp.y = temp.y + (timeRatio * scaling.y * textureCoords.y);\n"
;

const std::string PARTICLE_SHADER_VERTEX_MAIN_LUASTRING_END=
    "vec4 luaOut=projMatrix * vec4(temp, 1.0, 1.0);\n"
;

const std::string PARTICLE_SHADER_VERTEX_MAIN_END=
	"gl_Position = luaOut;\n"
"}"
;



const std::string PARTICLE_SHADER_FRAGMENT_DECLARATIONS=
"\n"
"//FRAGMENT SHADER\n"
"#version 300 es\n"

"precision mediump float;\n"

"in vec4  fragmentColor;\n"
"in vec2  fragmentUV;\n"

"out vec4 fragColor;\n"
;

const std::string PARTICLE_SHADER_FRAGMENT_MAIN_BEGIN=
"void main(){\n"
	"// colour the vertex with a simple procedural circular texture\n"
	"float dotProductUV = dot(fragmentUV, fragmentUV);\n"

    "float timeAlpha    = fragmentColor.a;\n"
	"float circleAlpha  = 1.0 - dotProductUV;\n"
	"float ringAlpha    = dotProductUV;\n"
	"float squareAlpha  = 1.0;\n"
;
const std::string PARTICLE_SHADER_FRAGMENT_MAIN_LUASTRING_BEGIN=
    "vec4 luaOut=vec4(fragmentColor.rgb,"
;
const std::string PARTICLE_SHADER_FRAGMENT_MAIN_LUASTRING_SHAPE_CIRCLE=
    "circleAlpha);\n"
    "if(dotProductUV > 1.0)\n"
		"discard;\n"
;
const std::string PARTICLE_SHADER_FRAGMENT_MAIN_LUASTRING_SHAPE_RING=
    "ringAlpha);\n"
    "if(dotProductUV > 1.0)\n"
		"discard;\n"
;
const std::string PARTICLE_SHADER_FRAGMENT_MAIN_LUASTRING_SHAPE_SQUARE=
    "squareAlpha);\n"
;
const std::string PARTICLE_SHADER_FRAGMENT_MAIN_LUASTRING_EFFECT_FADE_IN=
    "//The * 3.0 is used to keep the color as bold as possible\n"
    "luaOut.a = (timeAlpha) * luaOut.a * 3.0;\n"
;
const std::string PARTICLE_SHADER_FRAGMENT_MAIN_LUASTRING_EFFECT_FADE_OUT=
    "//The * 3.0 is used to keep the color as bold as possible\n"
    "luaOut.a = (1.0 - timeAlpha) * luaOut.a * 3.0;\n"
;
const std::string PARTICLE_SHADER_FRAGMENT_MAIN_LUASTRING_END=
    ""
;
const std::string PARTICLE_SHADER_FRAGMENT_MAIN_END=
	"fragColor = luaOut;\n"
"}"
;
