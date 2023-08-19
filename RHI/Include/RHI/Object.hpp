#pragma once
#pragma once 

namespace RHI
{

class Context;

class Object
{
public:
    Object(Context* context) 
        : m_context(context)

protected:
    Context* m_context;
};

}  // namespace RHI