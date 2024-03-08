#pragma once

namespace RHI
{
    class Attachment;

    class TransientAllocator
    {
    public:
        virtual ~TransientAllocator()                 = default;
        virtual void Begin()                          = 0;
        virtual void End()                            = 0;
        virtual void Allocate(Attachment* attachment) = 0;
        virtual void Release(Attachment* attachment)  = 0;
        virtual void Destroy(Attachment* attachment)  = 0;
    };

} // namespace RHI