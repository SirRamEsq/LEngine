#version 130

precision mediump float;

// we take single points as input
layout (points) in;
// we emit a triangle strip with up to 4 vertices per shader instance
layout (triangle_strip, max_vertices = 4) out; 

uniform mat4 modelviewMatrix;
uniform mat4 projectionMatrix;

in vec3 particlePosition;
in vec4 particleColor;
in float particleAlpha;

out vec2  fragmentUV;
out vec4  fragmentColor;
out float fragmentAlpha;


void main()
{
	fragmentColor=particleColor
	// we discard particles that have already faded out
	if (particleAlpha <= 0.0)
		return;

	// assign alpha to output
	fragmentAlpha = particleAlpha;

	// transform the center of the particle into camera space
	vec4 center = modelviewMatrix * vec4(particlePosition, 1);

	// emit the four vertices
	vec2 uv = vec2(-1, -1);
	vec4 p = center;
	p.xy += uv;
	fragmentUV = uv;
	gl_Position = projectionMatrix * p;
	EmitVertex();

	uv = vec2(1, -1);
	p = center;
	p.xy += uv;
	fragmentUV = uv;
	gl_Position = projectionMatrix * p;
	EmitVertex();

	uv = vec2(-1, 1);
	p = center;
	p.xy += uv;
	fragmentUV = uv;
	gl_Position = projectionMatrix * p;
	EmitVertex();

	uv = vec2(1, 1);
	p = center;
	p.xy += uv;
	fragmentUV = uv;
	gl_Position = projectionMatrix * p;
	EmitVertex();

	// emitting the primitive excplicitely is optional,
	// but I like to include it to make it clear when
	// geometry is created
	EndPrimitive();
}
