#version 300 es

layout (location = 0) in vec2 position; 	//vertex data
layout (location = 1) in vec2 texture; 		//texture
layout (location = 2) in vec4 color;		//color
layout (location = 3) in vec4 scalingRotation;  //ScalingRotation
layout (location = 4) in vec2 trans;		//Translation

out vec2 texture_coordinates;
out vec4 colorValue;

layout(std140) uniform CameraData
{
//Only camera translation is needed
//Camera scaling and rotation will take place when the camera applies it's buffer texutre (that it renderes the screen to) to a viewport
    vec4 cameraTranslation;
    mat4 projMatrix;
};

const float ONE_DEG_IN_RAD = ((2.0 * 3.14) / 360.0);   // 0.017444444

void main() {
	//Fragment shader variables
	texture_coordinates 	= texture;
	colorValue 		= color;
	
	//Calculate position
	float rad = scalingRotation.z * ONE_DEG_IN_RAD;
	float scaleX = scalingRotation.x;
	float scaleY = scalingRotation.y;

	mat4 scaling = mat4(
	  	vec4(scaleX, 0.0,    0.0,   0.0),           //first column
		vec4(0.0,    scaleY, 0.0,   0.0),           //second column
	  	vec4(0.0,    0.0,    1.0,   0.0),           //third column
	  	vec4(0.0,    0.0,    0.0,   1.0)            //fourth column
	);

	mat4 translate = mat4(
	  	vec4(1.0, 	0.0, 	0.0,   0.0),     	//first column
		vec4(0.0, 	1.0, 	0.0,   0.0),   		//second column
	  	vec4(0.0, 	0.0, 	1.0,   0.0),   		//third column
	  	vec4(trans.x, trans.y, 	0.0,   1.0)     	//fourth column
	);

	mat4 rotationZ = mat4(
	  	vec4(cos(rad),  sin(rad), 0.0, 	0.0),           //first column
		vec4(-sin(rad), cos(rad), 0.0, 	0.0),           //second column
	  	vec4(0.0,       0.0,      1.0, 	0.0),           //third column
	  	vec4(0.0, 	0.0, 	  0.0, 	1.0)            //fourth column
	);	

	vec4 temp = vec4(position.x, position.y, 1.0, 1.0);
	vec4 finalPosition =  (translate * rotationZ * scaling) * temp;

	//Camera translation
	finalPosition.x = finalPosition.x - cameraTranslation.x;
	finalPosition.y = finalPosition.y - cameraTranslation.y;

	gl_Position = projMatrix * finalPosition;
}
