#pragma once

#include <vsg/all.h>


class SubmitOpenVRCommand : public vsg::Command
{
public:
  SubmitOpenVRCommand(vrhelp::Env *vr)
      : mVr(vr)
  {
  }

  void read(vsg::Input &input) override {}
  void write(vsg::Output &output) const override {}

  void compile(vsg::Context &context) override {}

  void record(vsg::CommandBuffer &commandBuffer) const override
  {
    // Submit to SteamVR
    vr::VRTextureBounds_t bounds;
    bounds.uMin = 0.0f;
    bounds.uMax = 1.0f;
    bounds.vMin = 0.0f;
    bounds.vMax = 1.0f;
    /*
      vr::VRVulkanTextureData_t vulkanData;
      vulkanData.m_nImage = ( uint64_t ) hmdImageLeft.colourImage.imageView->vk(window->getOrCreateDevice()->deviceID);
      vulkanData.m_pDevice = ( VkDevice_T * ) window->getOrCreateDevice()->getDevice();
      vulkanData.m_pPhysicalDevice = ( VkPhysicalDevice_T * ) window->getOrCreateDevice()->getPhysicalDevice()->getPhysicalDevice();
      vulkanData.m_pInstance = ( VkInstance_T *) window->getOrCreateDevice()->getInstance()->getInstance(); 
      vulkanData.m_pQueue = ( VkQueue_T * ) window->getOrCreateDevice()->getQueue(0)->queue(); // TODO: use the correct queue ID
      vulkanData.m_nQueueFamilyIndex = 0; // TODO: use the correct queue ID

      uint32_t hmdWidth=0, hmdHeight=0;
      mVr->getRecommendedTargetSize(hmdWidth, hmdHeight);

      vulkanData.m_nWidth = hmdWidth;
      vulkanData.m_nHeight = hmdHeight;
      vulkanData.m_nFormat = imageFormat;
      vulkanData.m_nSampleCount = 1;

      vr::Texture_t texture = { &vulkanData, vr::TextureType_Vulkan, vr::ColorSpace_Auto };
      vr::VRCompositor()->Submit( vr::Eye_Left, &texture, &bounds ); 

      // vulkanData.m_nImage = ( uint64_t ) m_rightEyeDesc.m_pImage;
      vulkanData.m_nImage = ( uint64_t ) hmdImageLeft.colourImage.imageView->vk(window->getOrCreateDevice()->deviceID);
      vr::VRCompositor()->Submit( vr::Eye_Right, &texture, &bounds );
*/
  }

protected:
  virtual ~SubmitOpenVRCommand() {}
  vrhelp::Env *mVr = nullptr;
};