attribute vec4 a_position;
attribute vec2 a_normal;
attribute vec2 a_texCoords;
attribute vec2 a_packedColor;

uniform mat4 u_modelView;
uniform mat4 u_projection;
uniform mat4 u_pivotTransform;

varying vec2 v_texCoords;
varying LOW_P vec4 v_maskColor;

void main()
{
  vec4 pos = vec4(a_position.xyz, 1) * u_modelView;
  vec4 shiftedPos = vec4(a_normal, 0, 0) + pos;
  gl_Position = applyPivotTransform(shiftedPos * u_projection, u_pivotTransform, 0.0);
  v_texCoords = a_texCoords;
  v_maskColor = unpackColor(a_packedColor);
}
