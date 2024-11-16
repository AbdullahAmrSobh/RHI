struct VSInput
{
	float3 position;
    float3 normal;
    float2 uv;
};

struct PSInput
{
    float4 csPosition : SV_Position;
    float3 color      : TEXCOORD0;
    float2 uv         : TEXCOORD0;
};

struct PSOutput
{
    float4 color : SV_Target;
};

struct SceneGlobalBuffer
{
    float4x4 worldToViewMatrix;
    float4x4 viewToClipMatrix;
};

struct PerDrawBuffer
{
    float4x4 modelToWorldMatrix;
};

struct ShaderParamters
{
	ConstantBuffer<SceneGlobalBuffer> sceneGlobalBuffer;
	ConstantBuffer<PerDrawBuffer> perDrawData;
};

ParameterBlock<ShaderParamters> bindGroup0;

[[shader("vertex")]]
PSInput VSMain(VSInput input)
{
    let perDraw = bindGroup0.perDrawData;
    let scene = bindGroup0.sceneGlobalBuffer;

    PSInput result;
    let modelToClip = bindGroup0.perDrawData.modelToWorldMatrix;
    // let modelToClip = bindGroup0.perDrawData.modelToWorldMatrix * scene.worldToViewMatrix * scene.viewToClipMatrix;
    // let modelToClip =   scene.viewToClipMatrix * scene.worldToViewMatrix * bindGroup0.perDrawData.modelToWorldMatrix;
    result.csPosition = mul(modelToClip, float4(input.position, 1.0f));
    result.color = input.normal;
    result.uv = input.uv;
    return result;
}

[[shader("fragment")]]
PSOutput PSMain(PSInput input)
{
	PSOutput output;
    output.color = float4(input.color, 1.0);
	return output;
}
