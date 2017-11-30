#version 300 es

precision mediump float;

in vec3 texture_coordinates;
in vec4 colorValue;

uniform sampler2D basic_texture;
out vec4 frag_colour;

void main() {
	vec4 texel = texture (basic_texture, texture_coordinates.xy);
	texel = (texel * colorValue);

	//texel.rgb = 1.0 - texel.rgb;		 //invert texture;

	frag_colour = texel;
}
