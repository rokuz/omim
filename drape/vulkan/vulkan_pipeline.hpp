#pragma once

#include "drape/binding_info.hpp"
#include "drape/graphics_context.hpp"
#include "drape/pointers.hpp"
#include "drape/texture_types.hpp"
#include "drape/vulkan/vulkan_gpu_program.hpp"

#include <vulkan_wrapper.h>
#include <vulkan/vulkan.h>

#include <cstdint>
#include <map>

namespace dp
{
namespace vulkan
{
class VulkanPipeline
{
public:
  struct DepthStencilKey
  {
    void SetDepthTestEnabled(bool enabled);
    void SetDepthTestFunction(TestFunction depthFunction);
    void SetStencilTestEnabled(bool enabled);
    void SetStencilFunction(StencilFace face, TestFunction stencilFunction);
    void SetStencilActions(StencilFace face, StencilAction stencilFailAction,
                           StencilAction depthFailAction, StencilAction passAction);
    bool operator<(DepthStencilKey const & rhs) const;
    bool operator!=(DepthStencilKey const & rhs) const;
    
    bool m_depthEnabled = false;
    bool m_stencilEnabled = false;
    TestFunction m_depthFunction = TestFunction::Always;
    uint64_t m_stencil = 0;
  };

  struct PipelineKey
  {
    bool operator<(PipelineKey const & rhs) const;

    VkRenderPass m_renderPass = {};
    ref_ptr<VulkanGpuProgram> m_program;
    DepthStencilKey m_depthStencil;
    BindingInfoArray m_bindingInfo;
    uint8_t m_bindingInfoCount = 0;
    VkPrimitiveTopology m_primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    bool m_blendingEnabled = false;
  };

  VulkanPipeline(VkDevice device, int appVersionCode);
  void Dump(VkDevice device);
  void Destroy(VkDevice device);

  VkPipeline GetPipeline(VkDevice device, PipelineKey const & key);

private:
  int const m_appVersionCode;
  VkPipelineCache m_vulkanPipelineCache;

  using PipelineCache = std::map<PipelineKey, VkPipeline>;
  PipelineCache m_pipelineCache;
  bool m_isChanged = false;
};
}  // namespace vulkan
}  // namespace dp
