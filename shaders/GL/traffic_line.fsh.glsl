varying LOW_P vec4 v_color;

#ifdef SAMSUNG_GOOGLE_NEXUS
uniform sampler2D u_baseTex;
#endif

uniform float u_opacity;

void main()
{
  gl_FragColor = samsungGoogleNexusWorkaround(vec4(v_color.rgb, u_opacity));
}
