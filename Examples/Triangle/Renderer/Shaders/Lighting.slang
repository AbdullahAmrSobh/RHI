import ShaderBindingTable;

[[shader("compute"), numthreads(32, 32, 1)]]
void CSMain(uint3 threadId : SV_DispatchThreadID)
{
    var coord = threadId.xy;
    var position = bindings.pass_gBuffer_wsPosition[coord];
    // var normal = bindings.pass_gBuffer_normal[coord];

    bindings.pass_compose_output[coord] = position;
}