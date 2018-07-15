uniform vec4 u_color;

#ifdef SAMSUNG_GOOGLE_NEXUS
uniform sampler2D u_baseTex;
#endif

void main()
{
  gl_FragColor = samsungGoogleNexusWorkaround(u_color);
}
