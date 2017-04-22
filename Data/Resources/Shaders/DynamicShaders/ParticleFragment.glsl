#version 300 es

precision mediump float;

in vec4  fragmentColor;
in vec2  fragmentUV;

out vec4 fragColor;

void main(){
	// colour the vertex with a simple procedural circular texture
	float d = dot(fragmentUV, fragmentUV);

	if(d > 1.0)
		discard;

	float a = 1.0 - d;

	fragColor = vec4(fragmentColor.rgb, a);
}
