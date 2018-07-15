#pragma once

#include "drape/glsl_types.hpp"

#include <glm_config.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glm/gtx/rotate_vector.hpp>

#include <cmath>

namespace glsl
{
using glm::dot;
using glm::cross;
using glm::normalize;
using glm::length;
using glm::distance;

using glm::translate;
using glm::rotate;
using glm::scale;
using glm::transpose;
  
//inline float PackColor(dp::Color const & color)
//{
//  auto const rgba = ToVec4(color);
//  static vec4 const kScalar = vec4(1.0f, 1.0f/255.0f, 1.0f/65025.0f, 1.0f/16581375.0f);
//  return dot(rgba, kScalar);
//}
//#include "base/logging.hpp"
//inline dp::Color UnpackColor(float c)
//{
//  static vec4 const kScalar = vec4(1.0f, 255.0f, 65025.0f, 16581375.0f);
//  vec4 enc = kScalar * c;
//  float f;
//  vec4 fracPart = vec4(modff(enc.x, &f), modff(enc.y, &f), modff(enc.z, &f), modff(enc.w, &f));
//  enc.x = fracPart.x - fracPart.y / 255.0f;
//  enc.y = fracPart.y - fracPart.z / 255.0f;
//  enc.z = fracPart.z - fracPart.w / 255.0f;
//  enc.w = fracPart.w;
//  return dp::Color(static_cast<uint8_t>(255.0f * enc.x), static_cast<uint8_t>(255.0f * enc.y),
//                   static_cast<uint8_t>(255.0f * enc.z), static_cast<uint8_t>(255.0f * enc.w));
//}
  
inline vec2 PackColor(dp::Color const & color)
{
  static vec2 const kScalar = vec2(1.0f, 1.0f/1000.0f);
  return vec2(dot(vec2(color.GetRed(), color.GetGreen()), kScalar),
              dot(vec2(color.GetBlue(), color.GetAlpha()), kScalar));
}

inline dp::Color UnpackColor(vec2 const & c)
{
  vec4 enc;
  enc.y = roundf(modff(c.x, &enc.x) * 1000.0f);
  enc.w = roundf(modff(c.y, &enc.z) * 1000.0f);
  return dp::Color(static_cast<uint8_t>(enc.x), static_cast<uint8_t>(enc.y),
                   static_cast<uint8_t>(enc.z), static_cast<uint8_t>(enc.w));
}
}  // namespace glsl
