attribute vec3 a_position;
attribute vec2 a_packedColor;

uniform mat4 u_modelView;
uniform mat4 u_projection;
uniform mat4 u_pivotTransform;
uniform float u_zScale;

varying LOW_P vec4 v_color;

void main()
{
  vec4 pos = vec4(a_position, 1.0) * u_modelView;
  pos.xyw = (pos * u_projection).xyw;
  pos.z = a_position.z * u_zScale;
  gl_Position = u_pivotTransform * pos;
  v_color = unpackColor(a_packedColor);
}
