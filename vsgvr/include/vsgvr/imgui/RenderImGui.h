#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2023 Gareth Francis
Copyright(c) 2021 Don Burns, Roland Hill and Robert Osfield.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <functional>

#include <vsg/commands/ClearAttachments.h>
#include <vsg/app/Window.h>
#include <vsg/vk/DescriptorPool.h>

#include <vsgImGui/imgui.h>

#include <vsgvr/xr/GraphicsBindingVulkan.h>

namespace vsgvr
{
    /**
     * TODO: This is a fork of vsgImGui::RenderImGui, in order to remove the requirement for a Window
     *       It should be possible to push this back to vsgImGui, with vsgvr only keeping the composition layer binding
     */
    class VSGVR_DECLSPEC RenderImGui : public vsg::Inherit<vsg::Command, RenderImGui>
    {
    public:
        RenderImGui(vsg::ref_ptr<vsgvr::GraphicsBindingVulkan> graphicsBinding, VkExtent2D extent, bool useClearAttachments = false);

        using Component = std::function<bool()>;
        using Components = std::list<Component>;

        /// add a GUI rendering component that provides the ImGui calls to render the
        /// required GUI elements.
        void add(const Component& component);

        Components& getComponents() { return _components; }
        const Components& getComponents() const { return _components; }

        bool renderComponents() const;

        void record(vsg::CommandBuffer& commandBuffer) const override;

    private:
        virtual ~RenderImGui();

        vsg::ref_ptr<vsg::Device> _device;
        uint32_t _queueFamily;
        vsg::ref_ptr<vsg::Queue> _queue;
        vsg::ref_ptr<vsg::DescriptorPool> _descriptorPool;
        Components _components;

        vsg::ref_ptr<vsg::ClearAttachments> _clearAttachments;
        vsg::ref_ptr<vsg::RenderPass> _renderPass;

        void _init(vsg::ref_ptr<vsgvr::GraphicsBindingVulkan> graphicsBinding, VkExtent2D extent);
        void _uploadFonts();
    };
}

EVSG_type_name(vsgvr::RenderImGui);
