#pragma once

#define RHI_INTERFACE_BOILERPLATE(className)          \
    className()                            = default; \
    className(const className&)            = delete;  \
    className(className&&)                 = default; \
    virtual ~className()                   = default; \
    className& operator=(const className&) = delete;  \
    className& operator=(className&&)      = default;
