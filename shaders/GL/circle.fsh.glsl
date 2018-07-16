varying LOW_P vec4 v_color;
varying vec3 v_radius;

#ifdef SAMSUNG_GOOGLE_NEXUS
uniform sampler2D u_baseTex;
#endif

uniform float u_opacity;

const float aaPixelsCount = 2.5;

void main()
{
  LOW_P vec4 finalColor = v_color;
  float smallRadius = v_radius.z - aaPixelsCount;
  float stepValue = smoothstep(smallRadius * smallRadius, v_radius.z * v_radius.z,
                               v_radius.x * v_radius.x + v_radius.y * v_radius.y);
  finalColor.a = finalColor.a * u_opacity * (1.0 - stepValue);
  gl_FragColor = samsungGoogleNexusWorkaround(finalColor);
}
