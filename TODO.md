- [x] Resource bindings
- [] async compute
- [] ray tracing support
- [] mesh shaders support
- [] dynamic shader bind groups / bindless draws
- [x] validation layer
- [] multi-pass setup
- [x] fix memory leaks
- [] unify interface (RAII vs raw-handles make a decision) 
- [] rename resources and name spaces 
- [] reference Frame Graph resource directly (no attachments interface)
- [] memory defragmentation for graph and pool resources
- [] swapchain resize
- [] multithreading support
- [x] select memory type for images
- [] stream resources support
- [] add profiling/instrumentation
- [] multithreading support
- [x] resource copy / staging
- [x] multiple command list per pass
- [x] bind group layout object
- [x] command list reuse
- [x] cmake cleanup
- [] dont create vkBufferView when not needed (base on attachment usage)
- [] add CreateInfo structs validation layer

==========================================================================================
- abstract standard template library
- [] Add basic containers
    - [] memory
        - [] allocator interface
        - [] arena allocators
        - [] buddy allocators
        - [] local allocators
        - [] stack based allocators
    - [] thread context
    - [] job system
    - [] string
    - [] string view
    - [] map
    - [] set
    - [] array
    - [] vector
    - [] linked list
    - [] span
    - [] iterators library
    - [] expected/result
    - [] dynamic library loader
    - [] std types u32, i32, ...etc

==========================================================================================
- [] support library 
    - [] logging
    - [] file system

==========================================================================================
- [] examples
    - [] basic mesh

==========================================================================================
iterate on interface
    

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
    - [ ] prepare command lists
        - [ ] get current frame pool
            - [ ] for every pass in pass list
                - [ ] allocate command list
                    - [ ] if command list reused reset and begin
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


1. Main function
2. Application Base class and loop
3. Window system
4. Input and events
5. Time stamp
6. Camera and basic rendering pipeline

- Logging and profiling

Resource Loader 

Scene Graph

convert aiMatrix4x4 to glm::mat4
create scene graph
