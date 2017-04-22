#version 300 es

layout (location = 0) in vec2 position; 	//vertex data
//Texture data is either 1 or 0; this is for the light texture, not the diffuse texture
layout (location = 1) in vec2 texture; 		//texture
layout (location = 2) in vec4 color;		//color

out vec4 textureCoordinates;
out vec4 colorValue;

layout(std140) uniform CameraData
{
//Only camera translation is needed
//Camera scaling and rotation will take place when the camera applies it's buffer texutre (that it renderes the screen to) to a viewport
    vec4 cameraTranslation;
    mat4 projMatrix;
};

void main() {
	vec2 temp;
	temp.x = position.x - cameraTranslation.x;
	temp.y = position.y - cameraTranslation.y;

	//Fragment shader variables
	textureCoordinates.xy 	= texture;
	colorValue 		= color;

	textureCoordinates.z	= (temp.x / cameraTranslation.z);
	textureCoordinates.w	= (temp.y / cameraTranslation.w);

	gl_Position = projMatrix * (vec4(temp, 1.0, 1.0));
}
