#pragma once
#import <MetalKit/MetalKit.h>

#include "drape/graphics_context.hpp"
#include "drape/gpu_program.hpp"
#include "drape/metal/metal_cleaner.hpp"
#include "drape/metal/metal_states.hpp"
#include "drape/metal/metal_texture.hpp"
#include "drape/pointers.hpp"
#include "drape/texture_types.hpp"

#include "geometry/point2d.hpp"

#include <cstdint>
#include <functional>

namespace dp
{
namespace metal
{
class MetalBaseContext : public dp::GraphicsContext
{
public:
  using DrawableRequest = std::function<id<CAMetalDrawable>()>;
  
  MetalBaseContext() = default;
  MetalBaseContext(id<MTLDevice> device, m2::PointU const & screenSize,
                   DrawableRequest && drawableRequest);
  
  void Present() override;
  void MakeCurrent() override {}
  void DoneCurrent() override {}
  bool Validate() override { return true; }
  void Resize(int w, int h) override;
  void SetFramebuffer(ref_ptr<dp::BaseFramebuffer> framebuffer) override;
  void ApplyFramebuffer(bool enableParallel, std::string const & framebufferLabel) override;
  void Init(ApiVersion apiVersion) override;
  ApiVersion GetApiVersion() const override;
  std::string GetRendererName() const override;
  std::string GetRendererVersion() const override;
  ref_ptr<dp::GraphicsContext> GetParallelContext() const override;
  
  void PushDebugLabel(std::string const & label) override;
  void PopDebugLabel() override;
  
  void SetClearColor(Color const & color) override;
  void Clear(uint32_t clearBits, uint32_t storeBits) override;
  void Flush() override {}
  void SetViewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h) override;
  void SetDepthTestEnabled(bool enabled) override;
  void SetDepthTestFunction(TestFunction depthFunction) override;
  void SetStencilTestEnabled(bool enabled) override;
  void SetStencilFunction(StencilFace face, TestFunction stencilFunction) override;
  void SetStencilActions(StencilFace face, StencilAction stencilFailAction,
                         StencilAction depthFailAction, StencilAction passAction) override;
  void SetStencilReferenceValue(uint32_t stencilReferenceValue) override { m_stencilReferenceValue = stencilReferenceValue; }
  
  virtual id<MTLDevice> GetMetalDevice() const;
  virtual id<MTLRenderCommandEncoder> GetCommandEncoder() const;
  virtual id<MTLDepthStencilState> GetDepthStencilState();
  id<MTLDepthStencilState> GetDepthStencilState(MetalStates::DepthStencilKey const & key);
  virtual id<MTLRenderPipelineState> GetPipelineState(ref_ptr<GpuProgram> program, bool blendingEnabled);
  virtual id<MTLSamplerState> GetSamplerState(TextureFilter filter, TextureWrapping wrapSMode,
                                              TextureWrapping wrapTMode);
  
  virtual void SetSystemPrograms(drape_ptr<GpuProgram> && programClearColor,
                                 drape_ptr<GpuProgram> && programClearDepth,
                                 drape_ptr<GpuProgram> && programClearColorAndDepth);
  
  // Do not call this method, it's only for internal purposes. Use GetCommandEncoder() instead.
  id<MTLRenderCommandEncoder> GetAdditionalCommandEncoder() const { return m_additionalCommandEncoder; }
  
protected:
  void RecreateDepthTexture(m2::PointU const & screenSize);
  void RequestFrameDrawable();
  void ResetFrameDrawable();
  void InitEncoder(id<MTLRenderCommandEncoder> encoder, std::string const & label);
  void FinishCurrentEncoding();

  id<MTLDevice> m_device;
  DrawableRequest m_drawableRequest;
  drape_ptr<dp::GraphicsContext> m_parallelContext;
  
  drape_ptr<MetalTexture> m_depthTexture;
  MTLRenderPassDescriptor * m_renderPassDescriptor;
  id<MTLCommandQueue> m_commandQueue;
  ref_ptr<dp::BaseFramebuffer> m_currentFramebuffer;
  
  MetalStates::DepthStencilKey m_currentDepthStencilKey;
  MetalStates m_metalStates;
  
  // These objects are recreated each frame. They MUST NOT be stored anywhere.
  id<CAMetalDrawable> m_frameDrawable;
  id<MTLCommandBuffer> m_frameCommandBuffer;
  id<MTLRenderCommandEncoder> m_currentCommandEncoder;
  
  id<MTLParallelRenderCommandEncoder> m_currentParallelCommandEncoder;
  id<MTLRenderCommandEncoder> m_additionalCommandEncoder;
  
  MetalCleaner m_cleaner;
  
  uint32_t m_stencilReferenceValue = 1;
};
}  // namespace metal
}  // namespace dp
