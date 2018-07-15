attribute vec4 a_position;
attribute vec2 a_normal;
attribute vec2 a_packedColor;
attribute vec2 a_maskTexCoord;

uniform mat4 u_modelView;
uniform mat4 u_projection;
uniform mat4 u_pivotTransform;

varying LOW_P vec4 v_color;
varying vec2 v_maskTexCoord;

void main()
{
  vec4 pos = vec4(a_position.xyz, 1) * u_modelView;
  vec4 shiftedPos = vec4(a_normal, 0.0, 0.0) + pos;
  gl_Position = applyPivotTransform(shiftedPos * u_projection, u_pivotTransform, 0.0);
  v_color = unpackColor(a_packedColor);
  v_maskTexCoord = a_maskTexCoord;
}
