#pragma once

#include <functional>
#include <memory>
#include <string>

#include "RHI/Export.hpp"
#include "RHI/Handle.hpp"

namespace RHI
{

class FrameGraphBuilder;
class CommandList;
class ImageView;
class BufferView;

enum class PassQueue
{
    Graphics,
    Compute,
    Transfer,
};

/// @brief Base class for Pass producers, user overides this class to setup their own pass.
class PassProducer
{
    friend class FrameScheduler;

public:
    PassProducer(std::string name, PassQueue queueType);
    virtual ~PassProducer() = default;

    /// @brief Sets up the attachments that will be used in this pass.
    virtual void SetupAttachments(FrameGraphBuilder& builder) = 0;

    /// @brief Builds the CommandList for this pass.
    virtual void BuildCommandList(CommandList& commandList) = 0;
};

/// @brief PassProducer with functional callbacks.
class PassProducerCallbacks final : public PassProducer
{
    friend class FrameScheduler;

public:
    using SetupAttachmentsCallback = std::function<void(FrameGraphBuilder& builder)>;
    using BuildCommandListCallback = std::function<void(CommandList& commandList)>;

    PassProducerCallbacks(std::string              name,
                          PassQueue                queueType,
                          SetupAttachmentsCallback setupAttachmentsCallback,
                          BuildCommandListCallback buildCommandListCallback)
        : PassProducer(name, queueType)
        , m_setupAttachmentsCallback(setupAttachmentsCallback)
        , m_buildCommandListCallback(buildCommandListCallback)
    {
    }

    ~PassProducerCallbacks() = default;

    /// @brief Sets up the attachments that will be used in this pass.
    void SetupAttachments(FrameGraphBuilder& builder) override
    {
        m_setupAttachmentsCallback(builder);
    }

    /// @brief Builds the CommandList for this pass.
    void BuildCommandList(CommandList& commandList) override
    {
        m_buildCommandListCallback(commandList);
    }

private:
    SetupAttachmentsCallback m_setupAttachmentsCallback;
    BuildCommandListCallback m_buildCommandListCallback;
};

}  // namespace RHI