#version 300 es

precision mediump float;

in vec2 texture_coordinates;
in vec4 colorValue;

uniform sampler2D basic_texture;
out vec4 frag_colour;

void main() {
	vec4 texel = texture (basic_texture, texture_coordinates);
	texel = (texel * colorValue);


	texel.rgb = 0.25 + (texel.rgb * 0.001);
	
	frag_colour = texel;
}
