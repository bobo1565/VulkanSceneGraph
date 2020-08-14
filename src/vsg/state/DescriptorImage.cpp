/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/CopyAndReleaseImageDataCommand.h>
#include <vsg/io/Options.h>
#include <vsg/state/DescriptorImage.h>
#include <vsg/traversals/CompileTraversal.h>
#include <vsg/vk/CommandBuffer.h>

using namespace vsg;

/////////////////////////////////////////////////////////////////////////////////////////
//
// vsg::computeNumMipMapLevels
//
uint32_t vsg::computeNumMipMapLevels(const Data* data, const Sampler* sampler)
{
    uint32_t mipLevels = sampler != nullptr ? static_cast<uint32_t>(ceil(sampler->info().maxLod)) : 1;
    if (mipLevels == 0)
    {
        mipLevels = 1;
    }

    // clamp the mipLevels so that its no larger than what the data dimensions support
    uint32_t maxDimension = std::max({data->width(), data->height(), data->depth()});
    while ((1u << (mipLevels - 1)) > maxDimension)
    {
        --mipLevels;
    }

    //mipLevels = 1;  // disable mipmapping

    return mipLevels;
}

void ImageData::computeNumMipMapLevels()
{
    if (imageView && imageView->getImage())
    {
        auto info = imageView->getImage()->createInfo;
        auto mipLevels = vsg::computeNumMipMapLevels(info->data, sampler);
        imageView->getImage()->createInfo->mipLevels = mipLevels;
        imageView->createInfo->subresourceRange.levelCount = mipLevels;

        const auto& mipmapOffsets = info->data->computeMipmapOffsets();
        bool generatMipmaps = (mipLevels > 1) && (mipmapOffsets.size() <= 1);
        if (generatMipmaps) info->usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// DescriptorImage
//
DescriptorImage::DescriptorImage() :
    Inherit(0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
{
}

DescriptorImage::DescriptorImage(ref_ptr<Sampler> sampler, ref_ptr<Data> data, uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType) :
    Inherit(dstBinding, dstArrayElement, descriptorType)
{
    if (sampler && data)
    {
        auto image = Image::create(Image::CreateInfo::create(data));
        auto imageView = ImageView::create(ImageView::CreateInfo::create(image));
        _imageDataList.emplace_back(ImageData{sampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
    }
}

DescriptorImage::DescriptorImage(const SamplerImages& samplerImages, uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType) :
    Inherit(dstBinding, dstArrayElement, descriptorType)
{
    for(auto& si : samplerImages)
    {
        if (si.sampler && si.data)
        {
            auto image = Image::create(Image::CreateInfo::create(si.data));
            auto imageView = ImageView::create(ImageView::CreateInfo::create(image));
            _imageDataList.emplace_back(ImageData{si.sampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
        }
    }
}

DescriptorImage::DescriptorImage(const ImageData& imageData, uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType) :
    Inherit(dstBinding, dstArrayElement, descriptorType)
{
    _imageDataList.emplace_back(imageData);
}

DescriptorImage::DescriptorImage(const ImageDataList& imageDataList, uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType) :
    Inherit(dstBinding, dstArrayElement, descriptorType),
    _imageDataList(imageDataList)
{
}

void DescriptorImage::read(Input& input)
{
    Descriptor::read(input);

    // TODO old version

    _imageDataList.resize(input.readValue<uint32_t>("NumImages"));
    for (auto& imageData : _imageDataList)
    {
        ref_ptr<Data> data;
        input.readObject("Sampler", imageData.sampler);
        input.readObject("Image", data);

        auto image = Image::create(Image::CreateInfo::create(data));
        imageData.imageView = ImageView::create(ImageView::CreateInfo::create(image));
        imageData.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
}

void DescriptorImage::write(Output& output) const
{
    Descriptor::write(output);

    // TODO old version

    output.writeValue<uint32_t>("NumImages", _imageDataList.size());
    for (auto& imageData : _imageDataList)
    {
        output.writeObject("Sampler", imageData.sampler.get());

        ref_ptr<Data> data;
        if (imageData.imageView && imageData.imageView->getImage() && imageData.imageView->getImage()->createInfo) data = imageData.imageView->getImage()->createInfo->data;

        output.writeObject("Image", data.get());
    }
}

void DescriptorImage::compile(Context& context)
{
    if (_imageDataList.empty()) return;

    for(auto& imageData : _imageDataList)
    {
        if (imageData.sampler) imageData.sampler->compile(context);
        if (imageData.imageView)
        {
            auto imageView = imageData.imageView;

            if (imageView->createInfo && imageView->createInfo->image && imageView->createInfo->image->createInfo && imageView->createInfo->image->createInfo->data)
            {
                imageData.computeNumMipMapLevels();

                auto info = imageView->createInfo->image->createInfo;

                imageView->compile(context);

                auto stagingBufferData = copyDataToStagingBuffer(context, info->data);
                if (stagingBufferData)
                {
                    context.commands.emplace_back(new CopyAndReleaseImageDataCommand(stagingBufferData, imageData, info->mipLevels));
                }
            }
            else
            {
                imageView->compile(context);
            }
        }
    }
}

void DescriptorImage::assignTo(Context& context, VkWriteDescriptorSet& wds) const
{
    Descriptor::assignTo(context, wds);

    // convert from VSG to Vk
    auto pImageInfo = context.scratchMemory->allocate<VkDescriptorImageInfo>(_imageDataList.size());
    wds.descriptorCount = static_cast<uint32_t>(_imageDataList.size());
    wds.pImageInfo = pImageInfo;
    for (size_t i = 0; i < _imageDataList.size(); ++i)
    {
        const ImageData& data = _imageDataList[i];

        VkDescriptorImageInfo& info = pImageInfo[i];
        if (data.sampler)
            info.sampler = data.sampler->vk(context.deviceID);
        else
            info.sampler = 0;

        if (data.imageView)
            info.imageView = data.imageView->vk(context.deviceID);
        else
            info.imageView = 0;

        info.imageLayout = data.imageLayout;
    }
}

uint32_t DescriptorImage::getNumDescriptors() const
{
    return static_cast<uint32_t>(_imageDataList.size());
}
