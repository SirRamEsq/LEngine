#version 300 es

layout (location = 0) in vec2 position; 	//vertex data
layout (location = 1) in vec4 texture; 		//x,y, position (in tileCoords) of first frame of tile on texture, z,w are tex width and height
layout (location = 2) in vec2 animation;	//animation data (x = animation sped (float), y = max frames (int) )

out vec2 texture_coordinates;
out vec4 colorValue;

layout(std140) uniform CameraData
{
//Only camera translation is needed
//Camera scaling and rotation will take place when the camera applies it's buffer texutre (that it renderes the screen to) to a viewport
    vec4 cameraTranslation;
    mat4 projMatrix;
};

layout(std140) uniform ProgramData
{
//time.x = current time
    vec4 time;
};

uniform vec4 layerColor;

void main() {
	highp int timeValue = int(animation.x * time.x);
	highp int maxFrame = int(animation.y);

	//Fragment shader variables
	vec2 texTemp;
	texTemp.x = texture.x +  ( float(timeValue % maxFrame) * (16.0 / texture.z) );
	texTemp.y = texture.y;
	texture_coordinates 	= texTemp;
	/*
	vec4 layerColor;
	layerColor.x = 1.0;
	layerColor.y = 1.0;
	layerColor.z = 1.0;
	layerColor.w = 1.0;
	*/
	colorValue 		= layerColor;
	
	vec2 temp;
	temp.x = position.x - cameraTranslation.x;
	temp.y = position.y - cameraTranslation.y;

	gl_Position = projMatrix * (vec4(temp, 1.0, 1.0));
}
