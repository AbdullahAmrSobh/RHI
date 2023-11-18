- [] Make Rename resources and name spaces 
- [] Make Frame Graph reference resource directly 
- [] rename Frame graph to Render graph
- [] Make Separate types for graphics, compute and ray tracing pipelines
- [] memory defragmentation for graph and pool resources
- [] resource delete queue

==========================================================================================
Frame life cycle
- [-] Frame Begin
    - [-] if invalid
        - [-] reset
            - [] clear pass list
            - [x] clear pass attachments
        - [-] compile
            - [-] topological sort the graph
            - [x] allocate transient resources
            - [x] create resource views
                - [x] for swapchain images create view for each image attachment
    - [] prepare command lists
        - [] get current frame pool
            - [] for every pass in pass list
                - [] allocate command list
                    - [] if command list reused reset and begin
- [x] Frame End
    - [x] execute
        - [x] for every pass
            - [x] execute pass
                - [x] for every resource collect wait semaphores
                - [x] for every output resource collect signal semaphores
            - [x] if owns swapchain
                - [] queue present
        - [x] for swapchain in present queue
            - [x] present swapchain image
                - [x] queue present 
                - [x] swap buffers

==========================================================================================

Allow for multiple command lists to be executed in parallel from different device queues.
Command List allocations requirements
    1. Each thread can own multiple command list (one is the recommended)
    2. Each frame has independent set of command lists, the number of command list sets
    is equal to the number of maximum of all swapchains swap image values.

For every image attachment associated with swapchain, create image views per every swapchain image there is