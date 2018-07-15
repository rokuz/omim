attribute vec3 a_position;
attribute vec4 a_normal;
attribute vec2 a_packedColor;
attribute vec2 a_offset;

uniform mat4 u_modelView;
uniform mat4 u_projection;
uniform mat4 u_pivotTransform;

varying vec4 v_normal;
varying LOW_P vec4 v_color;

void main()
{
  vec4 p = vec4(a_position, 1) * u_modelView;
  vec4 pos = vec4(a_normal.xy + a_offset, 0, 0) + p;
  gl_Position = applyPivotTransform(pos * u_projection, u_pivotTransform, 0.0);
  v_color = unpackColor(a_packedColor);
  v_normal = a_normal;
}
