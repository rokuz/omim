varying LOW_P vec4 v_color;
varying vec2 v_maskTexCoord;
varying vec2 v_halfLength;

#ifdef SAMSUNG_GOOGLE_NEXUS
uniform sampler2D u_baseTex;
#endif

uniform sampler2D u_maskTex;
uniform float u_opacity;

const float aaPixelsCount = 2.5;

void main()
{
  vec4 color = v_color;
#ifdef GLES3
  float mask = texture2D(u_maskTex, v_maskTexCoord).r;
#else
  float mask = texture2D(u_maskTex, v_maskTexCoord).a;
#endif
  color.a = color.a * mask * u_opacity;
  
  float currentW = abs(v_halfLength.x);
  float diff = v_halfLength.y - currentW;
  color.a *= mix(0.3, 1.0, clamp(diff / aaPixelsCount, 0.0, 1.0));

  gl_FragColor = samsungGoogleNexusWorkaround(color);
}
