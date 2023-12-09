struct VSInput
{
	[[vk::location(0)]] float3 position : POSITION0;
};

struct VSOutput
{
	float4 position : SV_POSITION;
	float4 color : TEXCOORD;
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
