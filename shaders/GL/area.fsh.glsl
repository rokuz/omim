varying LOW_P vec4 v_color;

#ifdef SAMSUNG_GOOGLE_NEXUS
uniform sampler2D u_baseTex;
#endif

uniform float u_opacity;

void main()
{
  LOW_P vec4 finalColor = v_color;
  finalColor.a *= u_opacity;
  gl_FragColor = samsungGoogleNexusWorkaround(finalColor);
}
