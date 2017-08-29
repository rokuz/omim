// Implementation of Fast Approximate Anti-Aliasing (FXAA) is based on https://github.com/mattdesl/glsl-fxaa

uniform sampler2D u_colorTex;

uniform vec4 u_framebufferMetrics;

varying vec2 v_colorTexCoords;
varying vec4 v_rgbNW_NE;
varying vec4 v_rgbSW_SE;

const vec3 kLuma = vec3(0.299, 0.587, 0.114);

#ifndef FXAA_REDUCE_MIN
  #define FXAA_REDUCE_MIN (1.0/ 128.0)
#endif
#ifndef FXAA_REDUCE_MUL
  #define FXAA_REDUCE_MUL (1.0 / 8.0)
#endif
#ifndef FXAA_SPAN_MAX
  #define FXAA_SPAN_MAX 8.0
#endif
#ifdef GLES3
  #define FXAASampleLevelZero(tex, coord) textureLod(tex, coord, 0.0)
#else
  #define FXAASampleLevelZero(tex, coord) texture2D(tex, coord)
#endif

void main()
{
  vec3 rgbNW = FXAASampleLevelZero(u_colorTex, v_rgbNW_NE.xy).xyz;
  vec3 rgbNE = FXAASampleLevelZero(u_colorTex, v_rgbNW_NE.zw).xyz;
  vec3 rgbSW = FXAASampleLevelZero(u_colorTex, v_rgbSW_SE.xy).xyz;
  vec3 rgbSE = FXAASampleLevelZero(u_colorTex, v_rgbSW_SE.zw).xyz;
  vec4 texColor = FXAASampleLevelZero(u_colorTex, v_colorTexCoords);
  float lumaNW = dot(rgbNW, kLuma);
  float lumaNE = dot(rgbNE, kLuma);
  float lumaSW = dot(rgbSW, kLuma);
  float lumaSE = dot(rgbSE, kLuma);
  float lumaM = dot(texColor.xyz,  kLuma);
  float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
  float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

  vec2 dir = vec2(-lumaNW - lumaNE + lumaSW + lumaSE, lumaNW + lumaSW - lumaNE - lumaSE);

  float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) *
                        (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);
  float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
  dir = min(vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX),
            max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),
            dir * rcpDirMin)) * u_framebufferMetrics.xy;

  vec3 rgbA = 0.5 * (
      FXAASampleLevelZero(u_colorTex, v_colorTexCoords + dir * (1.0 / 3.0 - 0.5)).xyz +
      FXAASampleLevelZero(u_colorTex, v_colorTexCoords + dir * (2.0 / 3.0 - 0.5)).xyz);
  vec3 rgbB = rgbA * 0.5 + 0.25 * (
      FXAASampleLevelZero(u_colorTex, v_colorTexCoords - dir * 0.5).xyz +
      FXAASampleLevelZero(u_colorTex, v_colorTexCoords + dir * 0.5).xyz);

  vec4 color;
  float lumaB = dot(rgbB, kLuma);
  if ((lumaB < lumaMin) || (lumaB > lumaMax))
    color = vec4(rgbA, 1.0f);
  else
    color = vec4(rgbB, 1.0f);

  gl_FragColor = color;
}
