varying vec2 v_texCoords;
varying LOW_P vec4 v_maskColor;

uniform sampler2D u_baseTex;
uniform float u_opacity;

void main()
{
  vec4 finalColor = texture2D(u_baseTex, v_texCoords) * v_maskColor;
  finalColor.a *= u_opacity;
  gl_FragColor = finalColor;
}
