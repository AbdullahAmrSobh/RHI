// glsl_shader.vert, compiled with:
// # glslangValidator -V -x -o glsl_shader.vert.u32 glsl_shader.vert
/*

layout(push_constant) uniform uPushConstant { vec2 uScale; vec2 uTranslate; } pc;

out gl_PerVertex { vec4 gl_Position; };
layout(location = 0) out struct { vec4 Color; vec2 UV; } Out;

void main()
{
    Out.Color = aColor;
    Out.UV = aUV;
    gl_Position = vec4(aPos * pc.uScale + pc.uTranslate, 0, 1);
}
*/

// glsl_shader.frag, compiled with:
// # glslangValidator -V -x -o glsl_shader.frag.u32 glsl_shader.frag

#version 450 core
layout(location = 0) out vec4 fColor;
layout(set=0, binding=0) uniform sampler2D sTexture;
layout(location = 0) in struct { vec4 Color; vec2 UV; } In;
void main()
{
    fColor = In.Color * texture(sTexture, In.UV.st);
}


struct VSInput
{
	[[vk::location(0)]] float2 position : POSITION0;
	[[vk::location(1)]] float2 uv       : TEXCOORD0;
	[[vk::location(2)]] float4 colo r   : COLOR0;
};

struct VSOutput
{
    float4 position: POSITION;
	float4 color   : COLOR0;
	float2 uv      : TEXCOORD;
};

struct PSOutput
{
	float4 color : SV_TARGET0;
};

cbuffer UBO : register(b0)
{
	float4x4 viewProjection;
};

VSOutput VSMain(VSInput input)
{
	VSOutput output; 
	output.position = mul(viewProjection, float4(input.position, 1));
	output.color    = float4(1.0, 1.0, 1.0, 1.0);
	return output;
}

PSOutput PSMain(VSOutput input)
{
	PSOutput output;
	output.color = input.color; 
	return output;
}
