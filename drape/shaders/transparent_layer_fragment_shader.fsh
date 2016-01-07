uniform sampler2D tex;
varying vec2 v_tcoord;

const float opacity = 0.7;

uniform vec2 u_pixelSize;

float dxdy(float v1, float v2, float v3)
{
  float dx = v1 - v2;
  float dy = v1 - v3;
  return dx * dx + dy * dy;
}

void main()
{
  vec4 c1 = texture2D(tex, v_tcoord);
  vec4 c2 = texture2D(tex, v_tcoord + vec2(u_pixelSize.x, 0.0));
  vec4 c3 = texture2D(tex, v_tcoord + vec2(0.0, u_pixelSize.y));

  float alpha = opacity;
  if (c1.r + c1.g + c1.b < 0.01)
    alpha = 0.0;

  float d = dxdy(c1.r, c2.r, c3.r) + dxdy(c1.a, c2.a, c3.a);
  c1.rgb *= (0.5 + 0.5 * step(d, 0.0));

  gl_FragColor = vec4(c1.rgb, alpha);
}
