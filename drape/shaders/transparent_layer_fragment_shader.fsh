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

  if ((c1.r + c1.g + c1.b) < 0.01)
  {
    c1.a = 0.0;
  }
  else
  {
    float d = dxdy(c1.r, c2.r, c3.r) + dxdy(c1.a, c2.a, c3.a);
    c1.rgb *= (0.75 + 0.25 * step(d, 0.0));
    c1.a = opacity;
  }

  gl_FragColor = c1;
}
