#include "drape/metal/metal_base_context.hpp"
#include "drape/metal/metal_texture.hpp"

#include "drape/framebuffer.hpp"

#include "base/assert.hpp"

#include <algorithm>
#include <functional>
#include <string>
#include <vector>
#include <utility>

namespace dp
{
namespace metal
{
class ParallelMetalBaseContext : public MetalBaseContext
{
public:
  explicit ParallelMetalBaseContext(ref_ptr<MetalBaseContext> context)
    : m_mainContext(context)
  {}
  
  void Present() override
  {
    CHECK(false, ("Prohibited to call from parallel context."));
  }
  
  void Resize(int w, int h) override
  {
    CHECK(false, ("Prohibited to call from parallel context."));
  }
  
  void SetFramebuffer(ref_ptr<dp::BaseFramebuffer> framebuffer) override
  {
    CHECK(false, ("Prohibited to call from parallel context."));
  }
  
  void ApplyFramebuffer(bool enableParallel, std::string const & framebufferLabel) override
  {
    CHECK(false, ("Prohibited to call from parallel context."));
  }
  
  void Init(ApiVersion apiVersion) override
  {
    CHECK(false, ("Prohibited to call from parallel context."));
  }
  
  ApiVersion GetApiVersion() const override { return m_mainContext->GetApiVersion(); }
  std::string GetRendererName() const override { return m_mainContext->GetRendererName(); }
  std::string GetRendererVersion() const override { return m_mainContext->GetRendererVersion(); }
  ref_ptr<dp::GraphicsContext> GetParallelContext() const override { return m_mainContext->GetParallelContext(); }
  
  void PushDebugLabel(std::string const & label) override { m_mainContext->PushDebugLabel(label); }
  void PopDebugLabel() override { m_mainContext->PopDebugLabel(); }
  
  void SetClearColor(Color const & color) override
  {
    CHECK(false, ("Prohibited to call from parallel context."));
  }
  
  void Clear(uint32_t clearBits, uint32_t storeBits) override
  {
    CHECK(GetCommandEncoder() != nil, ());
    m_mainContext->Clear(clearBits, storeBits);
  }
  
  void SetViewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h) override
  {
    CHECK(false, ("Prohibited to call from parallel context."));
  }
  
  void SetDepthTestEnabled(bool enabled) override
  {
    m_parallelDepthStencilKey.m_depthEnabled = enabled;
  }
  
  void SetDepthTestFunction(dp::TestFunction depthFunction) override
  {
    m_parallelDepthStencilKey.m_depthFunction = depthFunction;
  }
  
  void SetStencilTestEnabled(bool enabled) override
  {
    CHECK(false, ("Unsuppored in parallel context."));
  }
  
  void SetStencilFunction(StencilFace face, TestFunction stencilFunction) override
  {
    CHECK(false, ("Unsuppored in parallel context."));
  }
  
  void SetStencilActions(StencilFace face, StencilAction stencilFailAction,
                         StencilAction depthFailAction, StencilAction passAction) override
  {
    CHECK(false, ("Unsuppored in parallel context."));
  }
  
  void SetStencilReferenceValue(uint32_t stencilReferenceValue) override
  {
    CHECK(false, ("Unsuppored in parallel context."));
  }
  
  id<MTLDevice> GetMetalDevice() const override { return m_mainContext->GetMetalDevice(); }
  
  id<MTLRenderCommandEncoder> GetCommandEncoder() const override
  {
    id<MTLRenderCommandEncoder> encoder = m_mainContext->GetAdditionalCommandEncoder();
    CHECK(encoder != nil, ("Probably encoding commands were called before ApplyFramebuffer or not from parallel context."));
    return encoder;
  }
  
  id<MTLDepthStencilState> GetDepthStencilState() override
  {
    return m_mainContext->GetDepthStencilState(m_parallelDepthStencilKey);
  }
  
  id<MTLRenderPipelineState> GetPipelineState(ref_ptr<GpuProgram> program, bool blendingEnabled) override
  {
    return m_mainContext->GetPipelineState(program, blendingEnabled);
  }
  
  id<MTLSamplerState> GetSamplerState(TextureFilter filter, TextureWrapping wrapSMode,
                                      TextureWrapping wrapTMode) override
  {
    return m_mainContext->GetSamplerState(filter, wrapSMode, wrapTMode);
  }
  
  void SetSystemPrograms(drape_ptr<GpuProgram> && programClearColor,
                         drape_ptr<GpuProgram> && programClearDepth,
                         drape_ptr<GpuProgram> && programClearColorAndDepth) override
  {
    CHECK(false, ("Prohibited to call from parallel context."));
  }
  
protected:
  ref_ptr<MetalBaseContext> m_mainContext;
  MetalStates::DepthStencilKey m_parallelDepthStencilKey;
};

MetalBaseContext::MetalBaseContext(id<MTLDevice> device, m2::PointU const & screenSize,
                                   DrawableRequest && drawableRequest)
  : m_device(device)
  , m_drawableRequest(std::move(drawableRequest))
  , m_parallelContext(make_unique_dp<ParallelMetalBaseContext>(this))
{
  m_renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
  m_renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
  m_renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
  m_renderPassDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
  m_renderPassDescriptor.depthAttachment.storeAction = MTLStoreActionStore;
  m_renderPassDescriptor.depthAttachment.clearDepth = 1.0;
  m_renderPassDescriptor.stencilAttachment.loadAction = MTLLoadActionClear;
  m_renderPassDescriptor.stencilAttachment.storeAction = MTLStoreActionStore;
  m_renderPassDescriptor.stencilAttachment.clearStencil = 0;
  
  RecreateDepthTexture(screenSize);
}
  
void MetalBaseContext::RecreateDepthTexture(m2::PointU const & screenSize)
{
  if (screenSize.x == 0 || screenSize.y == 0)
  {
    m_depthTexture.reset();
    return;
  }
  
  m_depthTexture = make_unique_dp<MetalTexture>(nullptr /* allocator */);
  HWTexture::Params params;
  params.m_width = screenSize.x;
  params.m_height = screenSize.y;
  params.m_format = TextureFormat::Depth;
  params.m_isRenderTarget = true;
  m_depthTexture->Create(make_ref(this), params, nullptr /* data */);
}
  
void MetalBaseContext::Init(dp::ApiVersion apiVersion)
{
  CHECK(apiVersion == dp::ApiVersion::Metal, ());
  m_commandQueue = [m_device newCommandQueue];
}

ApiVersion MetalBaseContext::GetApiVersion() const
{
  return dp::ApiVersion::Metal;
}
  
std::string MetalBaseContext::GetRendererName() const
{
  return std::string([m_device.name UTF8String]);
}

std::string MetalBaseContext::GetRendererVersion() const
{
  static std::vector<std::pair<MTLFeatureSet, std::string>> features;
  if (features.empty())
  {
    features.reserve(12);
    features.emplace_back(MTLFeatureSet_iOS_GPUFamily1_v1, "iOS_GPUFamily1_v1");
    features.emplace_back(MTLFeatureSet_iOS_GPUFamily2_v1, "iOS_GPUFamily2_v1");
    features.emplace_back(MTLFeatureSet_iOS_GPUFamily1_v2, "iOS_GPUFamily1_v2");
    features.emplace_back(MTLFeatureSet_iOS_GPUFamily2_v2, "iOS_GPUFamily2_v2");
    features.emplace_back(MTLFeatureSet_iOS_GPUFamily3_v1, "iOS_GPUFamily3_v1");
    if (@available(iOS 10.0, *))
    {
      features.emplace_back(MTLFeatureSet_iOS_GPUFamily1_v3, "iOS_GPUFamily1_v3");
      features.emplace_back(MTLFeatureSet_iOS_GPUFamily2_v3, "iOS_GPUFamily2_v3");
      features.emplace_back(MTLFeatureSet_iOS_GPUFamily3_v2, "iOS_GPUFamily3_v2");
    }
    if (@available(iOS 11.0, *))
    {
      features.emplace_back(MTLFeatureSet_iOS_GPUFamily1_v4, "iOS_GPUFamily1_v4");
      features.emplace_back(MTLFeatureSet_iOS_GPUFamily2_v4, "iOS_GPUFamily2_v4");
      features.emplace_back(MTLFeatureSet_iOS_GPUFamily3_v3, "iOS_GPUFamily3_v3");
      features.emplace_back(MTLFeatureSet_iOS_GPUFamily4_v1, "iOS_GPUFamily4_v1");
    }
    if (@available(iOS 12.0, *))
    {
      features.emplace_back(MTLFeatureSet_iOS_GPUFamily1_v5, "iOS_GPUFamily1_v5");
      features.emplace_back(MTLFeatureSet_iOS_GPUFamily2_v5, "iOS_GPUFamily2_v5");
      features.emplace_back(MTLFeatureSet_iOS_GPUFamily3_v4, "iOS_GPUFamily3_v4");
      features.emplace_back(MTLFeatureSet_iOS_GPUFamily4_v2, "iOS_GPUFamily4_v2");
      features.emplace_back(MTLFeatureSet_iOS_GPUFamily5_v1, "iOS_GPUFamily5_v1");
    }
    std::sort(features.begin(), features.end(), [](auto const & s1, auto const & s2)
    {
      return s1.first > s2.first;
    });
  }
  
  for (auto featureSet : features)
  {
    if ([m_device supportsFeatureSet:featureSet.first])
      return featureSet.second;
  }
  return "Unknown";
}
  
ref_ptr<dp::GraphicsContext> MetalBaseContext::GetParallelContext() const
{
  return make_ref(m_parallelContext);
}
  
void MetalBaseContext::PushDebugLabel(std::string const & label)
{
  id<MTLRenderCommandEncoder> encoder = GetCommandEncoder();
  if (encoder == nil)
    return;
  [encoder pushDebugGroup:@(label.c_str())];
}
  
void MetalBaseContext::PopDebugLabel()
{
  id<MTLRenderCommandEncoder> encoder = GetCommandEncoder();
  if (encoder == nil)
    return;
  [encoder popDebugGroup];
}
  
void MetalBaseContext::Resize(int w, int h)
{
  if (m_depthTexture && m_depthTexture->GetWidth() == w && m_depthTexture->GetHeight() == h)
    return;
  
  RecreateDepthTexture(m2::PointU(w, h));
}

void MetalBaseContext::SetFramebuffer(ref_ptr<dp::BaseFramebuffer> framebuffer)
{
  FinishCurrentEncoding();
  m_currentFramebuffer = framebuffer;
}

void MetalBaseContext::ApplyFramebuffer(bool enableParallel, std::string const & framebufferLabel)
{
  // Initialize frame command buffer if there is no one.
  if (!m_frameCommandBuffer)
  {
    m_frameCommandBuffer = [m_commandQueue commandBuffer];
    m_frameCommandBuffer.label = @"Frame command buffer";
  }
  
  if (!m_currentFramebuffer)
  {
    // Use default(system) framebuffer and depth-stencil.
    RequestFrameDrawable();
    m_renderPassDescriptor.colorAttachments[0].texture = m_frameDrawable != nil ? m_frameDrawable.texture : nil;
    m_renderPassDescriptor.depthAttachment.texture = m_depthTexture ? m_depthTexture->GetTexture() : nil;
    m_renderPassDescriptor.stencilAttachment.texture = nil;
  }
  else
  {
    ref_ptr<Framebuffer> framebuffer = m_currentFramebuffer;
    
    ASSERT(dynamic_cast<MetalTexture *>(framebuffer->GetTexture()->GetHardwareTexture().get()) != nullptr, ());
    ref_ptr<MetalTexture> colorAttachment = framebuffer->GetTexture()->GetHardwareTexture();
    m_renderPassDescriptor.colorAttachments[0].texture = colorAttachment->GetTexture();
    
    auto const depthStencilRef = framebuffer->GetDepthStencilRef();
    if (depthStencilRef != nullptr)
    {
      ASSERT(dynamic_cast<MetalTexture *>(depthStencilRef->GetTexture()->GetHardwareTexture().get()) != nullptr, ());
      ref_ptr<MetalTexture> depthStencilAttachment = depthStencilRef->GetTexture()->GetHardwareTexture();
      m_renderPassDescriptor.depthAttachment.texture = depthStencilAttachment->GetTexture();
      if (depthStencilAttachment->GetFormat() == dp::TextureFormat::DepthStencil)
        m_renderPassDescriptor.stencilAttachment.texture = depthStencilAttachment->GetTexture();
      else
        m_renderPassDescriptor.stencilAttachment.texture = nil;
    }
    else
    {
      m_renderPassDescriptor.depthAttachment.texture = nil;
      m_renderPassDescriptor.stencilAttachment.texture = nil;
    }
  }
  
  CHECK(m_currentCommandEncoder == nil, ("Current command encoder was not finished."));
  
  if (enableParallel)
  {
    m_currentParallelCommandEncoder = [m_frameCommandBuffer parallelRenderCommandEncoderWithDescriptor:m_renderPassDescriptor];
    m_currentCommandEncoder.label = @((framebufferLabel + " Parallel").c_str());
    
    // Additional encoder here is before the main one. It means it will be "renderer early".
    m_additionalCommandEncoder = [m_currentParallelCommandEncoder renderCommandEncoder];
    InitEncoder(m_additionalCommandEncoder, framebufferLabel + " Additional");
    
    m_currentCommandEncoder = [m_currentParallelCommandEncoder renderCommandEncoder];
    InitEncoder(m_currentCommandEncoder, framebufferLabel + " Main");
  }
  else
  {
    m_currentCommandEncoder = [m_frameCommandBuffer renderCommandEncoderWithDescriptor:m_renderPassDescriptor];
    InitEncoder(m_currentCommandEncoder, framebufferLabel);
  }
}
  
void MetalBaseContext::InitEncoder(id<MTLRenderCommandEncoder> encoder, std::string const & label)
{
  encoder.label = @(label.c_str());
  [encoder pushDebugGroup:@(label.c_str())];
  
  // Default rendering options.
  [encoder setFrontFacingWinding:MTLWindingClockwise];
  [encoder setCullMode:MTLCullModeBack];
  [encoder setStencilReferenceValue:m_stencilReferenceValue];
}

void MetalBaseContext::SetClearColor(dp::Color const & color)
{
  m_cleaner.SetClearColor(color);
  m_renderPassDescriptor.colorAttachments[0].clearColor =
    MTLClearColorMake(color.GetRedF(), color.GetGreenF(), color.GetBlueF(), color.GetAlphaF());
}
  
void MetalBaseContext::Clear(uint32_t clearBits, uint32_t storeBits)
{
  id<MTLRenderCommandEncoder> encoder = GetCommandEncoder();
  if (encoder != nil)
  {
    if ((clearBits & ClearBits::ColorBit) && (clearBits & ClearBits::DepthBit))
      m_cleaner.ClearColorAndDepth(make_ref(this), encoder);
    else if (clearBits & ClearBits::ColorBit)
      m_cleaner.ClearColor(make_ref(this), encoder);
    else if (clearBits & ClearBits::DepthBit)
      m_cleaner.ClearDepth(make_ref(this), encoder);
    
    if (clearBits & ClearBits::StencilBit)
      CHECK(false, ("Stencil clearing is not implemented"));
  }
  else
  {
    // Here, if we do not clear attachments, we load data ONLY if we store it afterwards, otherwise we use 'DontCare' option
    // to improve performance.
    if (clearBits & ClearBits::ColorBit)
      m_renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
    else
      m_renderPassDescriptor.colorAttachments[0].loadAction = (storeBits & ClearBits::ColorBit) ? MTLLoadActionLoad : MTLLoadActionDontCare;
    
    if (clearBits & ClearBits::DepthBit)
      m_renderPassDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
    else
      m_renderPassDescriptor.depthAttachment.loadAction = (storeBits & ClearBits::DepthBit) ? MTLLoadActionLoad : MTLLoadActionDontCare;
    
    if (clearBits & ClearBits::StencilBit)
      m_renderPassDescriptor.stencilAttachment.loadAction = MTLLoadActionClear;
    else
      m_renderPassDescriptor.stencilAttachment.loadAction = (storeBits & ClearBits::StencilBit) ? MTLLoadActionLoad : MTLLoadActionDontCare;
    
    // Apply storing mode.
    if (storeBits & ClearBits::ColorBit)
      m_renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
    else
      m_renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionDontCare;
    
    if (storeBits & ClearBits::DepthBit)
      m_renderPassDescriptor.depthAttachment.storeAction = MTLStoreActionStore;
    else
      m_renderPassDescriptor.depthAttachment.storeAction = MTLStoreActionDontCare;
    
    if (storeBits & ClearBits::StencilBit)
      m_renderPassDescriptor.stencilAttachment.storeAction = MTLStoreActionStore;
    else
      m_renderPassDescriptor.stencilAttachment.storeAction = MTLStoreActionDontCare;
  }
}
  
void MetalBaseContext::SetViewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
  id<MTLRenderCommandEncoder> encoder = GetCommandEncoder();
  [encoder setViewport:(MTLViewport){ static_cast<double>(x), static_cast<double>(y),
                                      static_cast<double>(w), static_cast<double>(h),
                                      0.0, 1.0 }];
  [encoder setScissorRect:(MTLScissorRect){ x, y, w, h }];
}

void MetalBaseContext::SetDepthTestEnabled(bool enabled)
{
  m_currentDepthStencilKey.m_depthEnabled = enabled;
}

void MetalBaseContext::SetDepthTestFunction(dp::TestFunction depthFunction)
{
  m_currentDepthStencilKey.m_depthFunction = depthFunction;
}

void MetalBaseContext::SetStencilTestEnabled(bool enabled)
{
  m_currentDepthStencilKey.m_stencilEnabled = enabled;
}

void MetalBaseContext::SetStencilFunction(dp::StencilFace face, dp::TestFunction stencilFunction)
{
  m_currentDepthStencilKey.SetStencilFunction(face, stencilFunction);
}

void MetalBaseContext::SetStencilActions(dp::StencilFace face,
                                         dp::StencilAction stencilFailAction,
                                         dp::StencilAction depthFailAction,
                                         dp::StencilAction passAction)
{
  m_currentDepthStencilKey.SetStencilActions(face, stencilFailAction, depthFailAction, passAction);
}
  
id<MTLDevice> MetalBaseContext::GetMetalDevice() const
{
  return m_device;
}
  
id<MTLRenderCommandEncoder> MetalBaseContext::GetCommandEncoder() const
{
  return m_currentCommandEncoder;
}
  
id<MTLDepthStencilState> MetalBaseContext::GetDepthStencilState()
{
  return GetDepthStencilState(m_currentDepthStencilKey);
}
  
id<MTLDepthStencilState> MetalBaseContext::GetDepthStencilState(MetalStates::DepthStencilKey const & key)
{
  return m_metalStates.GetDepthStencilState(m_device, key);
}
  
id<MTLRenderPipelineState> MetalBaseContext::GetPipelineState(ref_ptr<GpuProgram> program, bool blendingEnabled)
{
  CHECK(GetCommandEncoder() != nil, ("Probably encoding commands were called before ApplyFramebuffer."));
  
  id<MTLTexture> colorTexture = m_renderPassDescriptor.colorAttachments[0].texture;
  CHECK(colorTexture != nil, ());
  
  id<MTLTexture> depthTexture = m_renderPassDescriptor.depthAttachment.texture;
  MTLPixelFormat depthStencilFormat = (depthTexture != nil) ? depthTexture.pixelFormat : MTLPixelFormatInvalid;
  
  MetalStates::PipelineKey const key(program, colorTexture.pixelFormat, depthStencilFormat, blendingEnabled);
  return m_metalStates.GetPipelineState(m_device, key);
}
  
id<MTLSamplerState> MetalBaseContext::GetSamplerState(TextureFilter filter, TextureWrapping wrapSMode,
                                                      TextureWrapping wrapTMode)
{
  MetalStates::SamplerKey const key(filter, wrapSMode, wrapTMode);
  return m_metalStates.GetSamplerState(m_device, key);
}

void MetalBaseContext::Present()
{
  FinishCurrentEncoding();
  
  RequestFrameDrawable();
  if (m_frameDrawable)
    [m_frameCommandBuffer presentDrawable:m_frameDrawable];
  
  [m_frameCommandBuffer commit];
  m_frameDrawable = nil;
  [m_frameCommandBuffer waitUntilCompleted];
  m_frameCommandBuffer = nil;
}
  
void MetalBaseContext::RequestFrameDrawable()
{
  if (m_frameDrawable != nil)
    return;
  
  CHECK(m_drawableRequest != nullptr, ());
  m_frameDrawable = m_drawableRequest();
}
  
void MetalBaseContext::ResetFrameDrawable()
{
  if (m_frameDrawable == nil)
    return;
  
  m_frameDrawable = nil;
  RequestFrameDrawable();
}

void MetalBaseContext::FinishCurrentEncoding()
{
  [m_additionalCommandEncoder popDebugGroup];
  [m_additionalCommandEncoder endEncoding];
  
  [m_currentCommandEncoder popDebugGroup];
  [m_currentCommandEncoder endEncoding];
  
  [m_currentParallelCommandEncoder endEncoding];
  
  m_currentCommandEncoder = nil;
  m_additionalCommandEncoder = nil;
  m_currentParallelCommandEncoder = nil;
}
  
void MetalBaseContext::SetSystemPrograms(drape_ptr<GpuProgram> && programClearColor,
                                         drape_ptr<GpuProgram> && programClearDepth,
                                         drape_ptr<GpuProgram> && programClearColorAndDepth)
{
  m_cleaner.Init(make_ref(this), std::move(programClearColor), std::move(programClearDepth),
                 std::move(programClearColorAndDepth));
}
}  // namespace metal

void RenderFrameMediator(std::function<void()> && renderFrameFunction)
{
  @autoreleasepool
  {
    renderFrameFunction();
  }
}
}  // namespace dp
