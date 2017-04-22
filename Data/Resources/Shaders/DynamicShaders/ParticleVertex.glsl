#version 300 es

// the time since the creation of the particular particle system
uniform float time;

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 textureCoords;
layout (location = 2) in vec2 velocity;
layout (location = 3) in vec2 acceleration;
layout (location = 4) in vec4 color;

//lifeTime.x = timeStamp when particle was created, lifeTime.y = timeStamp to be destroyed
layout (location = 5) in vec2 lifetime;

layout(std140) uniform CameraData{
//Only camera translation is needed
//Camera scaling and rotation will take place when the camera applies it's buffer texutre (that it renderes the screen to) to a viewport
    vec4 cameraTranslation;
    mat4 projMatrix;
};

out vec4 fragmentColor;
out vec2 fragmentUV;


void main(){
	//if (particleAlpha <= 0.0)
	//	return;
	// calculate the corrent position of the particle
	float timeOffset = (time - lifetime.x);
	vec2 particlePosition = position
				+ velocity * timeOffset
				+ acceleration * (timeOffset * timeOffset * 0.5);

	// calculate the opacity of the particle
	// (does not have to be clamped to 0 since we are discarding
	//  non-positive alpha particles in the geometry shader)
	//float particleAlpha = min(1.0, (time / lifetime.y) * -2.0);
	float particleAlpha = ((time-timeOffset) / (lifetime.y-timeOffset));
	fragmentColor	= color;
	fragmentColor.a	= particleAlpha;
	fragmentUV=textureCoords;

	vec2 temp=particlePosition;
	temp.x = particlePosition.x - cameraTranslation.x;
	temp.y = particlePosition.y - cameraTranslation.y;

	gl_Position = projMatrix * vec4(temp, 1.0, 1.0);
}
