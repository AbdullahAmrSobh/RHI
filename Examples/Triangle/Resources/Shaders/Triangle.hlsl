struct VSInput
{
	[[vk::location(0)]] float3 position : POSITION0;
	[[vk::location(1)]] float3 normal   : NORMAL0;
	// [[vk::location(2)]] float2 uv0      : TEXCOORD0;
};

struct VSOutput
{
	float4 position : SV_POSITION;
	// float2 uv: TEXCOORD;
	float4 color : COLOR;
};

struct PSOutput
{
	float4 color : SV_TARGET0;
};

[[vk::binding(0)]] cbuffer PerFrame : register(b0)
{
	float4x4 viewProjectionMatrix;
	float4x4 projectionMatrix;
	float4x4 viewMatrix;
	float4x4 inverseViewMatrix;
};

[[vk::binding(1)]] cbuffer PerDraw : register(b1)
{
	float4x4 modelMatrix;

	// DirLight dirLight;
	// PointLight pointLights[MAX_LIGHTS];
	// SpotLight spotLights[MAX_LIGHTS];
};

float4 TransformToClipSpace(float3 position)
{
	float4x4 mvp = mul(mul(projectionMatrix, viewMatrix), modelMatrix);
	return mul(mvp, float4(position, 1.0));
}

float4 NormalToColor(float3 normal)
{
    float3 remappedNormal = 0.5f * (normal + 1.0f);
    return float4(remappedNormal, 1.0f);
}

VSOutput VSMain(VSInput input)
{
	VSOutput output;
	output.position = TransformToClipSpace(input.position);
	output.color    = NormalToColor(input.normal);
	// output.uv       = input.uv0;
	return output;
}

PSOutput PSMain(VSOutput input)
{
	PSOutput output;
	output.color = input.color;
	return output;
}
