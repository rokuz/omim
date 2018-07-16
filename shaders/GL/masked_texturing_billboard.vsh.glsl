attribute vec4 a_position;
attribute vec2 a_normal;
attribute vec2 a_texCoords;
attribute vec2 a_packedColor;

uniform mat4 u_modelView;
uniform mat4 u_projection;
uniform mat4 u_pivotTransform;
uniform float u_zScale;

varying vec2 v_texCoords;
varying LOW_P vec4 v_maskColor;

void main()
{
  vec4 pivot = vec4(a_position.xyz, 1.0) * u_modelView;
  vec4 offset = vec4(a_normal, 0.0, 0.0) * u_projection;
  gl_Position = applyBillboardPivotTransform(pivot * u_projection, u_pivotTransform,
                                             a_position.w * u_zScale, offset.xy);
  v_texCoords = a_texCoords;
  v_maskColor = unpackColor(a_packedColor);
}
