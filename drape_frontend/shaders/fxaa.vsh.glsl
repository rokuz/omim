// Implementation of Fast Approximate Anti-Aliasing (FXAA) is based on https://github.com/mattdesl/glsl-fxaa

attribute vec2 a_pos;
attribute vec2 a_tcoord;

uniform vec4 u_framebufferMetrics;

varying vec2 v_colorTexCoords;
varying vec4 v_rgbNW_NE;
varying vec4 v_rgbSW_SE;

void main()
{
  v_colorTexCoords = a_tcoord;
  v_rgbNW_NE = vec4(a_tcoord + vec2(-1.0, -1.0) * u_framebufferMetrics.xy,
                    a_tcoord + vec2(1.0, -1.0) * u_framebufferMetrics.xy);
  v_rgbSW_SE = vec4(a_tcoord + vec2(-1.0, 1.0) * u_framebufferMetrics.xy,
                    a_tcoord + vec2(1.0, 1.0) * u_framebufferMetrics.xy);
  gl_Position = vec4(a_pos, 0.0, 1.0);
}
