#version 300 es

precision mediump float;

//XY are light texture coords, ZW are Diffuse texture coords
in vec4 textureCoordinates;
in vec4 colorValue;

uniform sampler2D diffuse;
uniform sampler2D lightTexture;
out vec4 frag_colour;

void main() {
	//vec2 randomValue = rand(textureCoordinates.yw);
	
	vec4 diffuseTexel 	= texture (diffuse, textureCoordinates.zw);
	vec4 lightTexel 	= texture (lightTexture, textureCoordinates.xy);
	float average = (lightTexel.r + lightTexel.g + lightTexel.b)/3.0;
	lightTexel.a=average - (1.0 - colorValue.a);
	lightTexel.rgb = mix(lightTexel.rgb, colorValue.rgb, colorValue.a);
	//lightTexel.a = 1.0 - 1ightTexel.a;
	vec4 texel 		= vec4( mix(diffuseTexel.rgb, lightTexel.rgb, 0.5), lightTexel.a*2.0);

	//texel.rgb = 1.0 - texel.rgb;		 //invert texture;

						//greyscale
	//float averageTexel= (texel.r + texel.g + texel.b)/3.0;
	//texel.r=averageTexel;
	//texel.g=averageTexel;
	//texel.b=averageTexel;
	//texel.a = 1.0 - texel.a;

	frag_colour = texel;
}
