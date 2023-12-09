struct VSInput
{
	[[vk::location(0)]] float3 position : POSITION0 : register(c0);
	[[vk::location(1)]] float3 normal   : NORMAL0   : register(c1);
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

float4 NormalToColor(float3 normal)
{
    float3 remappedNormal = 0.5f * (normal + 1.0f);
    return float4(remappedNormal, 1.0f);
}

VSOutput VSMain(VSInput input)
{
	float4 finalPosition = float4(input.position * float3(1.0, -1.0, 1.0), 1.0);

	VSOutput output;
	output.position = mul(viewProjection, finalPosition);
	output.color    = NormalToColor(input.normal);
	return output;
}

PSOutput PSMain(VSOutput input)
{
	PSOutput output;
	output.color = input.color;
	return output;
}
