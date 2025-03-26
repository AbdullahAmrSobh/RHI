# Render Hardware Interface (RHI)

**Render Hardware Interface (RHI)** is a lightweight abstraction layer on top of graphics APIs. Currently, it supports Vulkan and WebGPU.

## Build

RHI requires the following to build:
- CMake
- C++20 compatible compiler

## Example Usage

_TODO_

## Docs

_TODO_

## Supported Platforms

| Platform    | Vulkan 1.3| WebGPU | D3D12 |
|------------|--------|--------|-------|
| Windows    | ✅     | ✅     | ✅    |
| Linux      | ❌     | ❌     | ❌    |
| Android    | ❌     | ❌     | ❌    |
| WebAssembly| ❌     | ✅     | ❌    |

Features
    Async-Compute: ❌
    Ray-Tracing: ❌
    Mesh-Shaders: ❌
