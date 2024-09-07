#include <ShaderInterface/Core.slang>

struct VSInput
{
	float3 position;
};

struct PSInput
{
    float4 csPosition : SV_Position;
};

struct PSOutput
{
    float4 color : SV_Target;
};

struct ShaderParamters
{
	ConstantBuffer<SI::ViewCB> viewCB;
};

ParameterBlock<ShaderParamters> bindGroup0;

[[shader("vertex")]]
PSInput VSMain(VSInput input)
{
    PSInput result;
    result.csPosition = mul(bindGroup0.viewCB.worldToClipMatrix, float4(input.position, 1.0f));
    return result;
}

[[shader("fragment")]]
PSOutput PSMain(PSInput input)
{
	PSOutput output;
    output.color = bindGroup0.viewCB.color;
	return output;
}
