struct VSInput
{
	float3 position : POSITION0;
	float3 normal   : NORMAL0;
	// float2 uv0      : TEXCOORD0;
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
	float4 worldPosition  : SV_TARGET1;
};

struct PerFrame
{
	float4x4 viewProjectionMatrix;
	float4x4 projectionMatrix;
	float4x4 viewMatrix;
	float4x4 inverseViewMatrix;
};

struct PerDraw
{
	float4x4 modelMatrix;

	// DirLight dirLight;
	// PointLight pointLights[MAX_LIGHTS];
	// SpotLight spotLights[MAX_LIGHTS];
};

struct Data
{
	ConstantBuffer<PerFrame> perFrame;
	ConstantBuffer<PerDraw>  perDraw;
};

ParameterBlock<Data> data;

float4 TransformToClipSpace(float3 position)
{
	float4x4 mvp = mul(mul(data.perFrame.projectionMatrix, data.perFrame.viewMatrix), data.perDraw.modelMatrix);
	return mul(mvp, float4(position, 1.0));
}

float4 NormalToColor(float3 normal)
{
    float3 remappedNormal = 0.5f * (normal + 1.0f);
    return float4(remappedNormal, 1.0f);
}

[[shader("vertex")]]
VSOutput VSMain(VSInput input)
{
	VSOutput output;
	output.position = TransformToClipSpace(input.position);
	output.color    = NormalToColor(input.normal);
	// output.uv       = input.uv0;
	return output;
}

[[shader("fragment")]]
PSOutput PSMain(VSOutput input)
{
	PSOutput output;
	output.color = input.color;
	output.worldPosition = input.position;
	return output;
}
