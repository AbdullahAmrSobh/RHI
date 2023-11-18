struct VSInput
{
	[[vk::location(0)]] float2 Pos : POSITION0;
	[[vk::location(1)]] float4 Color : COLOR0;
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

VSOutput VSMain(VSInput input)
{
	VSOutput output; 
	output.position = float4(input.Pos, 0, 1);
	output.color    = input.Color;
	return output;
}

PSOutput PSMain(VSOutput input)
{
	PSOutput output;
	output.color = input.color; 
	return output;
}
