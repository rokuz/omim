// Warning! Beware to use this shader. "discard" command may significally reduce performance.
// Unfortunately some CG algorithms cannot be implemented on OpenGL ES 2.0 without discarding
// fragments from depth buffer.

varying LOW_P vec4 v_color;
varying vec3 v_radius;

#ifdef SAMSUNG_GOOGLE_NEXUS
uniform sampler2D u_baseTex;
#endif

uniform sampler2D u_colorTex;
uniform float u_opacity;

const float kAntialiasingThreshold = 0.92;

void main()
{
  vec4 color = v_color;
  float smallRadius = v_radius.z * kAntialiasingThreshold;
  float stepValue = smoothstep(smallRadius * smallRadius, v_radius.z * v_radius.z,
                               v_radius.x * v_radius.x + v_radius.y * v_radius.y);
  color.a = u_opacity * (1.0 - stepValue);
  if (color.a < 0.01)
    discard;
  gl_FragColor = samsungGoogleNexusWorkaround(color);
}
