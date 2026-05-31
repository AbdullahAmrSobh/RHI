# Render Hardware Interface (RHI)

**Render Hardware Interface (RHI)** is a lightweight abstraction layer on top of graphics APIs. Currently, it supports Vulkan and WebGPU.

## Build Native

RHI requires the following to build:

> cmake -DCMAKE_BUILD_TYPE:STRING=Debug -S ./ -B ./build

> cmake --build /build --config Debug --target all

## Build WebAssembly

requires EMSDK 4.0+

> emcmake cmake -DCMAKE_BUILD_TYPE:STRING=Debug -S ./ -B ./build

> cmake --build /build --config Debug --target all

## Example Usage

_TODO_

## Docs

The API is self expainatory, read the code

## Supported Platforms

| Platform    | Vulkan 1.3| WebGPU | D3D12 |
|------------|--------|--------|-------|
| Windows    | ✅     | ✅     | ✅    |
| Linux      | ❌     | ❌     | ❌    |
| Android    | ❌     | ❌     | ❌    |
| WebAssembly| ❌     | ✅     | ❌    |

Features
    Ray-Tracing: 🟨
    Mesh-Shaders: 🟨
