#pragma once

#import "drape/oglcontext.hpp"
#import "drape/drape_global.hpp"

#import <QuartzCore/CAEAGLLayer.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

#include "std/atomic.hpp"

class iosOGLContext : public dp::OGLContext
{
public:
  iosOGLContext(CAEAGLLayer * layer, dp::ApiVersion apiVersion,
                iosOGLContext * contextToShareWith, bool needBuffers = false);
  ~iosOGLContext();

  virtual void makeCurrent();
  virtual void present();
  virtual void setDefaultFramebuffer();
  virtual void resize(int w, int h);
  
  void setPresentAvailable(bool available);

private:
  CAEAGLLayer * m_layer;
  EAGLContext * m_nativeContext;

  void initBuffers();
  void destroyBuffers();

  //{@ Buffers
  bool m_needBuffers;
  bool m_hasBuffers;

  GLuint m_renderBufferId;
  GLuint m_depthBufferId;
  GLuint m_frameBufferId;
  //@} buffers
  
  atomic<bool> m_presentAvailable;
};
