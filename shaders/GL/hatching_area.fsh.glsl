varying LOW_P vec4 v_color;
varying vec2 v_maskTexCoords;

uniform float u_opacity;
uniform sampler2D u_maskTex;

void main()
{
  LOW_P vec4 color = v_color;
  color *= texture2D(u_maskTex, v_maskTexCoords);
  color.a *= u_opacity;
  gl_FragColor = color;
}

