varying vec2 v_maskTexCoord;
varying LOW_P vec4 v_color;

#ifdef SAMSUNG_GOOGLE_NEXUS
uniform sampler2D u_baseTex;
#endif

uniform sampler2D u_maskTex;
uniform float u_opacity;
uniform vec2 u_contrastGamma;

void main()
{
  LOW_P vec4 glyphColor = v_color;
#ifdef GLES3
  float alpha = texture2D(u_maskTex, v_maskTexCoord).r;
#else
  float alpha = texture2D(u_maskTex, v_maskTexCoord).a;
#endif
  glyphColor.a *= u_opacity * alpha;
  gl_FragColor = samsungGoogleNexusWorkaround(glyphColor);
}
