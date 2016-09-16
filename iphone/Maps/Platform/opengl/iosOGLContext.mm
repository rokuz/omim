#import "iosOGLContext.h"
#import "base/assert.hpp"
#import "base/logging.cpp"

#import "drape/glfunctions.hpp"

#include "std/algorithm.hpp"

iosOGLContext::iosOGLContext(CAEAGLLayer * layer, iosOGLContext * contextToShareWith, bool needBuffers)
  : m_layer(layer)
  , m_nativeContext(NULL)
  , m_needBuffers(needBuffers)
  , m_hasBuffers(false)
  , m_renderBufferId(0)
  , m_msaaRenderBufferId(0)
  , m_depthBufferId(0)
  , m_frameBufferId(0)
  , m_msaaFrameBufferId(0)
  , m_msaaEnabled(false)
  , m_presentAvailable(true)
{
  if (contextToShareWith != NULL)
  {
    m_nativeContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2
                                           sharegroup: contextToShareWith->m_nativeContext.sharegroup];
  }
  else
    m_nativeContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
}

iosOGLContext::~iosOGLContext()
{
  destroyBuffers();
}

void iosOGLContext::makeCurrent()
{
  ASSERT(m_nativeContext != NULL, ());
  [EAGLContext setCurrentContext: m_nativeContext];

  if (m_needBuffers && !m_hasBuffers)
    initBuffers();
}

void iosOGLContext::setPresentAvailable(bool available)
{
  m_presentAvailable = available;
}

void iosOGLContext::present()
{
  ASSERT(m_nativeContext != NULL, ());
  ASSERT_NOT_EQUAL(m_renderBufferId, 0, ());
  
  if (m_msaaEnabled)
  {
    ASSERT_NOT_EQUAL(m_msaaRenderBufferId, 0, ());
  
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER_APPLE, m_frameBufferId);
    glBindFramebuffer(GL_READ_FRAMEBUFFER_APPLE, m_msaaFrameBufferId);
    glResolveMultisampleFramebufferAPPLE();
  
    GLenum const discards[] = { GL_COLOR_ATTACHMENT0, GL_DEPTH_ATTACHMENT };
    GLCHECK(glDiscardFramebufferEXT(GL_READ_FRAMEBUFFER_APPLE, 2, discards));
  }
  else
  {
    GLenum const discards[] = { GL_DEPTH_ATTACHMENT };
    GLCHECK(glDiscardFramebufferEXT(GL_FRAMEBUFFER, 1, discards));
  }

  glBindRenderbuffer(GL_RENDERBUFFER, m_renderBufferId);
  
  if (m_presentAvailable)
    [m_nativeContext presentRenderbuffer: GL_RENDERBUFFER];

  if (!m_msaaEnabled)
  {
    GLenum const discards[] = { GL_COLOR_ATTACHMENT0 };
    GLCHECK(glDiscardFramebufferEXT(GL_FRAMEBUFFER, 1, discards));
  }
}

void iosOGLContext::setDefaultFramebuffer()
{
  ASSERT_NOT_EQUAL(m_frameBufferId, 0, ());
  if (m_msaaEnabled)
    ASSERT_NOT_EQUAL(m_msaaFrameBufferId, 0, ());
  glBindFramebuffer(GL_FRAMEBUFFER, m_msaaEnabled ? m_msaaFrameBufferId : m_frameBufferId);
}

void iosOGLContext::resize(int w, int h)
{
  if (m_needBuffers && m_hasBuffers)
  {
    GLint width = 0;
    GLint height = 0;
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
    if (width == w && height == h)
      return;

    destroyBuffers();
    initBuffers();
  }
}

void iosOGLContext::initBuffers()
{
  ASSERT(m_needBuffers, ());

  if (!m_hasBuffers)
  {
    // Color
    glGenRenderbuffers(1, &m_renderBufferId);
    glBindRenderbuffer(GL_RENDERBUFFER, m_renderBufferId);
    [m_nativeContext renderbufferStorage:GL_RENDERBUFFER fromDrawable: m_layer];
    
    GLint width = 0;
    GLint height = 0;
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
    m_msaaEnabled = (max(width, height) < 1400);
    int const kSamplesCount = 4;
    
    if (m_msaaEnabled)
    {
      // MSAA color
      glGenRenderbuffers(1, &m_msaaRenderBufferId);
      glBindRenderbuffer(GL_RENDERBUFFER, m_msaaRenderBufferId);
      glRenderbufferStorageMultisampleAPPLE(GL_RENDERBUFFER, kSamplesCount, GL_RGBA8_OES, width, height);
    
      // MSAA depth
      glGenRenderbuffers(1, &m_depthBufferId);
      glBindRenderbuffer(GL_RENDERBUFFER, m_depthBufferId);
      glRenderbufferStorageMultisampleAPPLE(GL_RENDERBUFFER, kSamplesCount, GL_DEPTH_COMPONENT16, width, height);
      
      // MSAA framebuffer
      glGenFramebuffers(1, &m_msaaFrameBufferId);
      glBindFramebuffer(GL_FRAMEBUFFER, m_msaaFrameBufferId);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_msaaRenderBufferId);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthBufferId);
      GLint fbStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
      if (fbStatus != GL_FRAMEBUFFER_COMPLETE)
        LOG(LERROR, ("Incomplete MSAA framebuffer:", fbStatus));
    }
    else
    {
      // Depth
      glGenRenderbuffers(1, &m_depthBufferId);
      glBindRenderbuffer(GL_RENDERBUFFER, m_depthBufferId);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
    }

    // Framebuffer
    glGenFramebuffers(1, &m_frameBufferId);
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferId);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_renderBufferId);
    if (!m_msaaEnabled)
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthBufferId);
    GLint fbStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fbStatus != GL_FRAMEBUFFER_COMPLETE)
      LOG(LERROR, ("Incomplete framebuffer:", fbStatus));

    m_hasBuffers = true;
  }
}

void iosOGLContext::destroyBuffers()
{
  if (m_needBuffers && m_hasBuffers)
  {
    glFinish();
    if (m_msaaEnabled)
    {
      glBindFramebuffer(GL_FRAMEBUFFER, m_msaaFrameBufferId);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, 0);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferId);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, 0);
    if (!m_msaaEnabled)
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    
    if (m_msaaEnabled)
    {
      glDeleteFramebuffers(1, &m_msaaFrameBufferId);
      glDeleteRenderbuffers(1, &m_msaaRenderBufferId);
    }
    glDeleteFramebuffers(1, &m_frameBufferId);
    glDeleteRenderbuffers(1, &m_renderBufferId);
    glDeleteRenderbuffers(1, &m_depthBufferId);
    
    m_hasBuffers = false;
  }
}
