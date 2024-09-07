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
	ConstantBuffer<SI::TransformUB> transformUB;
};

ParameterBlock<ShaderParamters> bindGroup0;

[[shader("vertex")]]
PSInput VSMain(VSInput input)
{
    var modelToClipMat4 = bindGroup0.viewCB.worldToClipMatrix * bindGroup0.transformUB.modelToWorldMatrix;
    PSInput result;
    result.csPosition = mul(modelToClipMat4, float4(input.position, 1.0f));
    return result;
}

[[shader("fragment")]]
PSOutput PSMain(PSInput input)
{
	PSOutput output;
    output.color = float4(0.6, 0.4, 0.5, 1.0);
	return output;
}
