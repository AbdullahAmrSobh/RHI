struct VSInput
{
	[[vk::location(0)]] float3 position : POSITION0;
	[[vk::location(1)]] float3 normal   : NORMAL0;
	[[vk::location(2)]] float2 uv0      : TEXCOORD0;
};

struct VSOutput
{
	float4 position : SV_POSITION;
	float2 uv: TEXCOORD;
	float4 color : COLOR;
};

struct PSOutput
{
	float4 color : SV_TARGET0;
};

[[vk::binding(0)]] cbuffer UBO : register(b0)
{
	float4x4 viewProjection;
};

[[vk::binding(1)]] Texture2D texture : register(t0);
[[vk::binding(2)]] SamplerState textureSampler : register(s0);

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
	output.uv       = input.uv0;
	return output;
}

PSOutput PSMain(VSOutput input)
{
	float4 finalColor = texture.Sample(textureSampler, input.uv);

	PSOutput output;
	output.color = finalColor;
	return output;
}
