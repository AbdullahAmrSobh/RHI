#include "Assets/Image.hpp"

#include <cmp_core.h>
#include <compressonator.h>

#include <TL/Memory.hpp>
#include <TL/FileSystem/FileSystem.hpp>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_RESIZE2_IMPLEMENTATION

#include "external/stb_image.h"
#include "external/stb_image_write.h"
#include "external/stb_image_resize2.h"
#include "external/dds_image/dds.hpp"
#include "external/dds_image/dds_formats.hpp"
#include "external/dds.h"

#include <cmp_core.h>

#include <filesystem>

namespace Examples::Assets
{
    struct FormatInfo
    {
        ImageFormat imageFormat;
        CMP_FORMAT cmp;
        DXGI_FORMAT dxgi;
        uint8_t totalSize;
    };

    constexpr FormatInfo FormatsConversionLut[uint32_t(ImageFormat::COUNT)] =
    {
        {ImageFormat::Unknown,     CMP_FORMAT_Unknown,   DXGI_FORMAT_UNKNOWN,            0 },
        { ImageFormat::R8,         CMP_FORMAT_R_8,       DXGI_FORMAT_R8_UNORM,           1 },
        { ImageFormat::RG8,        CMP_FORMAT_RG_8,      DXGI_FORMAT_R8G8_UNORM,         2 },
        { ImageFormat::RGB8,       CMP_FORMAT_RGB_888,   DXGI_FORMAT_UNKNOWN,            3 },
        { ImageFormat::RGBA8,      CMP_FORMAT_RGBA_8888, DXGI_FORMAT_R8G8B8A8_UNORM,     4 },
        { ImageFormat::R16,        CMP_FORMAT_R_16,      DXGI_FORMAT_R16_UNORM,          2 },
        { ImageFormat::RG16,       CMP_FORMAT_RG_16,     DXGI_FORMAT_R16G16_UNORM,       4 },
        { ImageFormat::RGB16,      CMP_FORMAT_MAX,       DXGI_FORMAT_UNKNOWN,            6 },
        { ImageFormat::RGBA16,     CMP_FORMAT_RGBA_16,   DXGI_FORMAT_R16G16B16A16_UNORM, 8 },
        { ImageFormat::R32,        CMP_FORMAT_R_32F,     DXGI_FORMAT_R32_FLOAT,          4 },
        { ImageFormat::RG32,       CMP_FORMAT_RG_32F,    DXGI_FORMAT_R32G32_FLOAT,       8 },
        { ImageFormat::RGB32,      CMP_FORMAT_RGB_32F,   DXGI_FORMAT_R32G32B32_FLOAT,    12},
        { ImageFormat::RGBA32,     CMP_FORMAT_RGBA_32F,  DXGI_FORMAT_R32G32B32A32_FLOAT, 16},
        { ImageFormat::BC1,        CMP_FORMAT_BC1,       DXGI_FORMAT_BC1_UNORM,          0 },
        { ImageFormat::BC2,        CMP_FORMAT_BC2,       DXGI_FORMAT_BC2_UNORM,          0 },
        { ImageFormat::BC3,        CMP_FORMAT_BC3,       DXGI_FORMAT_BC3_UNORM,          0 },
        { ImageFormat::BC4,        CMP_FORMAT_BC4,       DXGI_FORMAT_BC4_UNORM,          0 },
        { ImageFormat::BC4_Signed, CMP_FORMAT_BC4_S,     DXGI_FORMAT_BC4_SNORM,          0 },
        { ImageFormat::BC5,        CMP_FORMAT_BC5,       DXGI_FORMAT_BC5_UNORM,          0 },
        { ImageFormat::BC5_Signed, CMP_FORMAT_BC5_S,     DXGI_FORMAT_BC5_SNORM,          0 },
        { ImageFormat::BC6H,       CMP_FORMAT_BC6H,      DXGI_FORMAT_BC6H_UF16,          0 },
        { ImageFormat::BC6H_SF,    CMP_FORMAT_BC6H_SF,   DXGI_FORMAT_BC6H_SF16,          0 },
        { ImageFormat::BC7,        CMP_FORMAT_BC7,       DXGI_FORMAT_BC7_UNORM,          0 },
    };

    inline static CMP_FORMAT ConvertFormatTo_CMP_Format(ImageFormat format)
    {
        return FormatsConversionLut[static_cast<uint32_t>(format)].cmp;
    }

    inline static DXGI_FORMAT ConvertFormatTo_DXGI_FORMAT(ImageFormat format)
    {
        return FormatsConversionLut[static_cast<uint32_t>(format)].dxgi;
    }

    inline static ImageFormat ConvertFormatTo_ImageFormat(DXGI_FORMAT format)
    {
        for (auto formatInfo : FormatsConversionLut)
        {
            if (formatInfo.dxgi == format)
            {
                return formatInfo.imageFormat;
            }
        }
        return ImageFormat::Unknown;
    }

    TL::Ptr<Image> Image::STBLoad(TL::Block block)
    {
        int32_t width, height, comp;
        if (stbi_info_from_memory((stbi_uc*)block.ptr, (int32_t)block.size, &width, &height, &comp))
        {
            TL_LOG_INFO("Failed to load image - {}", stbi_failure_reason());
        }

        ImageFormat format;
        bool isHdr = stbi_is_hdr_from_memory((stbi_uc*)block.ptr, (int32_t)block.size) != 0;
        bool is16Bit = stbi_is_16_bit_from_memory((stbi_uc*)block.ptr, (int32_t)block.size) != 0;

        switch (comp)
        {
        case STBI_default:    TL_UNREACHABLE();
        case STBI_grey:       format = is16Bit ? ImageFormat::R16 : ImageFormat::R8; break;
        case STBI_grey_alpha: format = is16Bit ? ImageFormat::RG16 : ImageFormat::RG8; break;
        case STBI_rgb:        format = is16Bit ? ImageFormat::RGB16 : ImageFormat::RGB8; break;
        case STBI_rgb_alpha:  format = is16Bit ? ImageFormat::RGBA16 : ImageFormat::RGBA8; break;
        }

        size_t formatSize = comp;
        if (is16Bit)
            formatSize *= 2;

        /// @todo: revist this
        void* data;
        if (is16Bit)
            data = stbi_load_16_from_memory((stbi_uc*)block.ptr, (int32_t)block.size, &width, &height, &comp, STBI_default);
        else if (isHdr)
            data = stbi_loadf_from_memory((stbi_uc*)block.ptr, (int32_t)block.size, &width, &height, &comp, STBI_default);
        else
            data = stbi_load_from_memory((stbi_uc*)block.ptr, (int32_t)block.size, &width, &height, &comp, STBI_default);

        size_t dataSize = width * height * formatSize;
        auto imageBlock = TL::Allocator::Allocate(dataSize, 1);
        memcpy(imageBlock.ptr, data, dataSize);
        stbi_image_free(data);

        auto image = TL::CreatePtr<Image>();
        // image->m_name;
        image->m_format = format;
        image->m_size = { (uint32_t)width, (uint32_t)height, 0 };
        image->m_mipLevelsCount = 1;
        image->m_arrayElementsCount = 1;
        image->m_data = imageBlock;
        return image;
    }

    TL::Ptr<Image> Image::DDSLoad(TL::Block block)
    {
        dds::Image image;
        auto res = dds::readImage((uint8_t*)block.ptr, block.size, &image);
        TL_ASSERT(res == dds::Success);

        auto newBlock = TL::Allocator::Allocate(image.data.size(), 1);
        memcpy(newBlock.ptr, image.data.data(), image.data.size() * sizeof(uint8_t));

        auto newImage = TL::CreatePtr<Image>();
        newImage->m_format = ConvertFormatTo_ImageFormat(image.format);
        newImage->m_size.width = image.width;
        newImage->m_size.height = image.height;
        newImage->m_size.depth = image.depth;
        newImage->m_arrayElementsCount = image.arraySize;
        newImage->m_mipLevelsCount = image.numMips;
        newImage->m_data = newBlock;
        return newImage;
    }

    TL::Ptr<Image> Image::Load(TL::Block block)
    {
        auto header = dds::read_header(block.ptr, block.size);
        return header.magic == dds::fourcc('D', 'D', 'S', ' ') ? DDSLoad(block) : STBLoad(block);
    }

    TL::Block Image::Save(Image& image)
    {
        auto block = TL::Allocator::Allocate(sizeof(dds::Header) + image.m_data.size, alignof(dds::Header));

        dds::write_header(
            (dds::Header*)block.ptr,
            (dds::DXGI_FORMAT)ConvertFormatTo_DXGI_FORMAT(image.m_format),
            image.m_size.width,
            image.m_size.height,
            image.m_mipLevelsCount,
            image.m_arrayElementsCount,
            false,
            image.m_size.depth);

        memcpy((char*)block.ptr + sizeof(dds::Header), image.m_data.ptr, image.m_data.size);
        return block;
    }

    TL::Ptr<Image> Image::Convert(Image& image, ImageSize newSize, ImageFormat newFormat)
    {
        if (newSize.width == 0)
            newSize = image.m_size;
        if (newFormat == ImageFormat::Unknown)
            newFormat = image.m_format;

        auto formatSize = FormatsConversionLut[(uint32_t)image.m_format].totalSize;
        CMP_Texture srcTexture{
            .dwSize = sizeof(CMP_Texture),
            .dwWidth = image.m_size.width,
            .dwHeight = image.m_size.height,
            .dwPitch = image.m_size.width * formatSize,
            .format = ConvertFormatTo_CMP_Format(image.m_format),
            .transcodeFormat = CMP_FORMAT_Unknown,
            .nBlockHeight = 0,
            .nBlockWidth = 0,
            .nBlockDepth = 0,
            .dwDataSize = (CMP_DWORD)image.m_data.size,
            .pData = (CMP_BYTE*)image.m_data.ptr,
            .pMipSet = nullptr,
        };

        uint32_t newDwPitch = FormatsConversionLut[(uint32_t)newFormat].totalSize;
        CMP_Texture dstTexture{
            .dwSize = sizeof(CMP_Texture),
            .dwWidth = newSize.width,
            .dwHeight = newSize.height,
            .dwPitch = newSize.width * newDwPitch,
            .format = ConvertFormatTo_CMP_Format(newFormat),
            .transcodeFormat = CMP_FORMAT_Unknown,
            .nBlockHeight = 0,
            .nBlockWidth = 0,
            .nBlockDepth = 0,
            .dwDataSize = CMP_CalculateBufferSize(&dstTexture),
            .pData = nullptr,
            .pMipSet = nullptr,
        };
        auto dstBlock = TL::Allocator::Allocate(dstTexture.dwDataSize, 1);
        dstTexture.pData = (CMP_BYTE*)dstBlock.ptr;

        auto feedbackProgress = nullptr;
        // auto feedbackProgress = [](CMP_FLOAT fProgress, CMP_DWORD_PTR pUser1, CMP_DWORD_PTR pUser2) -> bool
        // {
        //     // TODO
        //     return false;
        // };

        CMP_CompressOptions compressOptions{};
        compressOptions.nEncodeWith = CMP_GPU_VLK;
        auto res = CMP_ConvertTexture(&srcTexture, &dstTexture, &compressOptions, feedbackProgress);
        TL_ASSERT(res == CMP_OK);

        auto newImage = TL::CreatePtr<Image>();
        newImage->m_format = newFormat;
        newImage->m_size = newSize;
        newImage->m_arrayElementsCount = 1;
        newImage->m_mipLevelsCount = 1;
        newImage->m_data = dstBlock;
        return newImage;
    }

    Image::~Image()
    {
        if (m_data.ptr)
        {
            TL::Allocator::Release(m_data, 1);
        }
    }
} // namespace Examples::Assets