#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/state/DescriptorImage.h>

namespace vsg
{

    class VSG_DECLSPEC DescriptorImageView : public Inherit<Descriptor, DescriptorImageView>
    {
    public:
        DescriptorImageView();

        DescriptorImageView(ImageData imageData, uint32_t dstBinding = 0, uint32_t dstArrayElement = 0, VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

        /** ImageDataList is automatically filled in by the DecriptorImage::compile() using the sampler and image data objects.*/
        ImageDataList& getImageDataList() { return _imageDataList; }
        const ImageDataList& getImageDataList() const { return _imageDataList; }

        void read(Input& input) override;
        void write(Output& output) const override;

        void compile(Context& context) override;

        void assignTo(Context& context, VkWriteDescriptorSet& wds) const override;

        uint32_t getNumDescriptors() const override;

    protected:
        ImageDataList _imageDataList;
        bool _compiled;
    };
    VSG_type_name(vsg::DescriptorImageView);

} // namespace vsg
