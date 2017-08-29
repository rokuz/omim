uniform sampler2D u_colorTex;
uniform sampler2D u_fxaaTex;

varying vec2 v_colorTexCoords;

void main()
{
  vec4 color = texture2D(u_colorTex, v_colorTexCoords);
  vec4 fxaaColor = texture2D(u_fxaaTex, v_colorTexCoords);
  vec3 finalColor = mix(color.rgb, fxaaColor.rgb, fxaaColor.a);
  gl_FragColor = vec4(finalColor, color.a);
}
