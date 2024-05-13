- Fix leaks (should close app without any assertions, errors, or warnnings)
    - Fix CommandLists leaks
    - Fix RenderGraph leaks
    - Consider adding reference counting for resources referencing other resources (e.g. layout resources)

- Fix swapchain issues
    - Fix swapchain resizing
    - Fix changing surface

- Render Graph
    - Simplify interface. Reduce the code needed to setup a full render graph

- Bind Group
    - Simplify, consider making create + update a single call

- API:
    - Consider removing all RAII handles, replace with Create/Destroy, or Init/Shutdown functions
    - Consider removing all layout resources i.e. BindGroupLayout, PipelineLayout

Should Frame end be
1. when render graph is dispatched?
2. when swapchain presents?
3. expliclt function call