#ifdef SAMSUNG_GOOGLE_NEXUS
uniform sampler2D u_baseTex;
#endif

varying LOW_P vec4 v_color;

void main()
{
  gl_FragColor = samsungGoogleNexusWorkaround(v_color);
}
