uniform sampler2D u_baseTex;
uniform float u_opacity;

varying vec2 v_texCoords;

void main()
{
  vec4 finalColor = texture2D(u_baseTex, v_texCoords);
  finalColor.a *= u_opacity;
  gl_FragColor = finalColor;
}
