varying LOW_P vec4 v_color;
varying float v_intensity;

#ifdef SAMSUNG_GOOGLE_NEXUS
uniform sampler2D u_baseTex;
#endif

uniform float u_opacity;

void main()
{
  vec4 finalColor = vec4((v_intensity * 0.2 + 0.8) * v_color.rgb, u_opacity);
  gl_FragColor = samsungGoogleNexusWorkaround(finalColor);
}
