attribute vec3 a_position;
attribute vec2 a_packedColor;
attribute vec2 a_maskTexCoords;

uniform mat4 u_modelView;
uniform mat4 u_projection;
uniform mat4 u_pivotTransform;

varying LOW_P vec4 v_color;
varying vec2 v_maskTexCoords;

void main()
{
  vec4 pos = vec4(a_position, 1) * u_modelView * u_projection;
  gl_Position = applyPivotTransform(pos, u_pivotTransform, 0.0);
  v_color = unpackColor(a_packedColor);
  v_maskTexCoords = a_maskTexCoords;
}
