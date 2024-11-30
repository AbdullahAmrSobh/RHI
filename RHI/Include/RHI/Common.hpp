#pragma once

#define RHI_INTERFACE_BOILERPLATE(className)          \
    className()                            = default; \
    className(const className&)            = delete;  \
    className(className&&)                 = default; \
    virtual ~className()                   = default; \
    className& operator=(const className&) = delete;  \
    className& operator=(className&&)      = default;

#define RHI_INTERFACE_BOILERPLATE_HPP(className) \
    className();                                 \
    className(const className&);                 \
    className(className&&);                      \
    virtual ~className();                        \
    className& operator=(const className&);      \
    className& operator=(className&&);

#define RHI_INTERFACE_BOILERPLATE_CPP(className)                 \
    className::className()                            = default; \
    className::className(const className&)            = delete;  \
    className::className(className&&)                 = default; \
    className::~className()                           = default; \
    className& className::operator=(const className&) = delete;  \
    className& className::operator=(className&&)      = default;
