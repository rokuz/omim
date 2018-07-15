attribute vec3 a_position;
attribute vec4 a_normal;
attribute vec4 a_colorTexCoords;

uniform mat4 u_modelView;
uniform mat4 u_projection;
uniform mat4 u_pivotTransform;

varying vec4 v_normal;
varying LOW_P vec4 v_color;

void main()
{
  vec4 pivot = vec4(a_position.xyz, 1.0) * u_modelView;
  vec4 offset = vec4(a_normal.xy + a_colorTexCoords.zw, 0.0, 0.0) * u_projection;
  gl_Position = applyBillboardPivotTransform(pivot * u_projection, u_pivotTransform, 0.0, offset.xy);
  v_color = unpackColor(a_colorTexCoords);
  v_normal = a_normal;
}
