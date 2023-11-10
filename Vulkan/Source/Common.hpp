#pragma once
#include <vk_mem_alloc.h>

#define RHIVK_ASSERT_SUCCESS(result) RHI_ASSERT(result == VK_SUCCESS)

#define RHIVK_RETURN_VKERR_CODE(result) \
    if (result != VK_SUCCESS)           \
    {                                   \
        return result;                  \
    }

#define RHIVK_RETURN_ERR(result)                   \
    if (result != VK_SUCCESS)                      \
    {                                              \
        return { {}, ConvertResult(result) }; \
    }

#define RHIVK_RETURN_ERR_CODE(result)      \
    if (result != VK_SUCCESS)              \
    {                                      \
        return ConvertResult(result); \
    }