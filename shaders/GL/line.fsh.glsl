varying LOW_P vec4 v_color;
varying vec2 v_halfLength;

uniform float u_opacity;

const float aaPixelsCount = 2.5;

void main()
{
  LOW_P vec4 color = v_color;
  color.a *= u_opacity;
  
  float currentW = abs(v_halfLength.x);
  float diff = v_halfLength.y - currentW;
  color.a *= mix(0.3, 1.0, clamp(diff / aaPixelsCount, 0.0, 1.0));
  
  gl_FragColor = color;
}
