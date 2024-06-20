#include "ShaderInterface/Core.slang"

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

struct Data
{
	ConstantBuffer<SceneTransform> sceneTransform;
	ConstantBuffer<ObjectTransform> objectTransform;
};

ParameterBlock<Data> data;

float4 TransformToClipSpace(float3 position)
{
	float4x4 mvp = mul(mul(data.sceneTransform.projectionMatrix, data.sceneTransform.viewMatrix), data.objectTransform.modelMatrix);
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
