attribute vec3 a_position;
attribute vec2 a_packedColor;

uniform mat4 u_modelView;
uniform mat4 u_projection;
uniform mat4 u_pivotTransform;

varying LOW_P vec4 v_color;

void main()
{
  vec2 transformedAxisPos = (vec4(a_position.xy, 0.0, 1.0) * u_modelView).xy;
  vec4 pos = vec4(transformedAxisPos, a_position.z, 1.0) * u_projection;
  v_color = unpackColor(a_packedColor);
  gl_Position = applyPivotTransform(pos, u_pivotTransform, 0.0);
}
