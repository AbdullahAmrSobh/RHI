struct VSInput
{
	float3 position;
    float4 color;
};

struct PSInput
{
    float4 csPosition : SV_Position;
    float4 color      : TEXCOORD0;
};

struct PSOutput
{
    float4 color : SV_Target;
};

struct UniformData
{
    float4x4 transform;
};

struct ShaderParamters
{
	ConstantBuffer<UniformData> viewCB;
};

ParameterBlock<ShaderParamters> bindGroup0;

[[shader("vertex")]]
PSInput VSMain(VSInput input)
{
    PSInput result;
    result.csPosition = mul(bindGroup0.viewCB.transform, float4(input.position, 1.0f));
    result.color = input.color;
    return result;
}

[[shader("fragment")]]
PSOutput PSMain(PSInput input)
{
	PSOutput output;
    output.color = input.color;
	return output;
}
