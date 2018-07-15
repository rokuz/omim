// Warning! Beware to use this shader. "discard" command may significally reduce performance.
// Unfortunately some CG algorithms cannot be implemented on OpenGL ES 2.0 without discarding
// fragments from depth buffer.

uniform sampler2D u_baseTex;
uniform float u_opacity;
uniform vec4 u_maskColor;

varying vec2 v_texCoords;

void main()
{
  vec4 finalColor = texture2D(u_baseTex, v_texCoords);
  finalColor.a *= u_opacity;
  if (finalColor.a < 0.01)
    discard;
  finalColor = vec4(mix(finalColor.rgb, u_maskColor.rgb, u_maskColor.a), finalColor.a);
  gl_FragColor = finalColor;
}
