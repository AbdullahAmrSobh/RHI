#include "ShaderInterface/Core.slang"

struct VSInput
{
	float3 position : POSITION0;
	float3 normal   : NORMAL0;
	float2 uv0      : TEXCOORD0;
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
	float4 worldPosition  : SV_TARGET1;
};

struct Data
{
	ConstantBuffer<SceneTransform> sceneTransform;
	ConstantBuffer<ObjectTransform> objectTransform;
	SamplerState samplerState;
	Texture2D texture[];
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
	output.uv       = input.uv0;
	output.uv.y     *= -1.0;
	return output;
}

[[shader("fragment")]]
PSOutput PSMain(VSOutput input)
{
	let colorTexture = data.texture[NonUniformResourceIndex(data.objectTransform.colorIndex)];
	let normalTexture = data.texture[NonUniformResourceIndex(data.objectTransform.normalIndex)];
	let color = colorTexture.Sample(data.samplerState, input.uv);
	let normal = normalTexture.Sample(data.samplerState, input.uv);

	PSOutput output;
	output.color = color;
	output.worldPosition = normal;
	return output;
}
