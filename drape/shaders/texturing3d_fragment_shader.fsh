uniform sampler2D u_colorTex;
uniform float u_opacity;

varying vec2 v_colorTexCoords;
varying vec2 v_intensity;

void main(void)
{
  vec4 finalColor = vec4(texture2D(u_colorTex, v_colorTexCoords).rgb, u_opacity);
  gl_FragColor = vec4((v_intensity.x * 0.2 + 0.8) * finalColor.rgb, v_intensity.y);
}
