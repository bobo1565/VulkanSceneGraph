#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <any>

#include <vsg/vk/Swapchain.h>

namespace vsg
{
    // forward declare
    class Window;

    class WindowTraits : public Inherit<Object, WindowTraits>
    {
    public:
        WindowTraits() {}
        WindowTraits(const WindowTraits&) = default;
        WindowTraits& operator=(const WindowTraits&) = default;

        WindowTraits(int32_t in_x, int32_t in_y, uint32_t in_width, uint32_t in_height) :
            x(in_x),
            y(in_y),
            width(in_width),
            height(in_height) {}

        WindowTraits(uint32_t in_width, uint32_t in_height) :
            width(in_width),
            height(in_height) {}

        int32_t x = 0;
        int32_t y = 0;
        uint32_t width = 1280;
        uint32_t height = 1024;

        bool fullscreen = false;

        std::string display; /// A non empty display string overrides any X11 DISPLAY env var that may have been set. ignored on non X11 systems
        int screenNum = -1;  /// negative screenNum value indicates system defaults should be assumed, a non zero value will set the screenNum, overriding any display or DISPLAY setting for this.

        std::string windowClass = "vsg::Window";
        std::string windowTitle = "vsg window";

        bool decoration = true;
        bool hdpi = true;

        // X11 hint of whether to ignore the Window managers redirection of window size/position
        bool overrideRedirect = false;

        SwapchainPreferences swapchainPreferences;
        VkFormat depthFormat = VK_FORMAT_D24_UNORM_S8_UINT; //VK_FORMAT_D32_SFLOAT; // VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_SFLOAT_S8_UINT
        VkImageUsageFlags depthImageUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

        VkQueueFlags queueFlags = VK_QUEUE_GRAPHICS_BIT;
        VkPipelineStageFlagBits imageAvailableSemaphoreWaitFlag = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        bool debugLayer = false;
        bool apiDumpLayer = false;

        vsg::Names instanceExtensionNames;
        vsg::Names deviceExtensionNames;

        // Multisampling
        // A bitmask of sample counts. The window's framebuffer will
        // be configured with the maxium requested value that is
        // supported by the device.
        VkSampleCountFlags samples = VK_SAMPLE_COUNT_1_BIT;
        ref_ptr<Device> device;

        Window* shareWindow = nullptr;

        AllocationCallbacks* allocator = nullptr;

        std::any nativeWindow;
        std::any systemConnection;

    protected:
        virtual ~WindowTraits() {}
    };
    VSG_type_name(vsg::WindowTraits);

} // namespace vsg
