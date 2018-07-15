attribute vec4 a_position;
attribute vec2 a_normal;
attribute vec2 a_packedColor;
attribute vec2 a_packedOutlineColor;
attribute vec2 a_maskTexCoord;

uniform mat4 u_modelView;
uniform mat4 u_projection;
uniform mat4 u_pivotTransform;
uniform float u_isOutlinePass;

varying LOW_P vec4 v_color;
varying vec2 v_maskTexCoord;

const float kBaseDepthShift = -10.0;

void main()
{
  float isOutline = step(0.5, u_isOutlinePass);
  float depthShift = kBaseDepthShift * isOutline;

  vec4 pos = (vec4(a_position.xyz, 1) + vec4(0.0, 0.0, depthShift, 0.0)) * u_modelView;
  vec4 shiftedPos = vec4(a_normal, 0.0, 0.0) + pos;
  gl_Position = applyPivotTransform(shiftedPos * u_projection, u_pivotTransform, 0.0);
  vec2 packedColor = mix(a_packedColor, a_packedOutlineColor, isOutline);
  v_color = unpackColor(packedColor);
  v_maskTexCoord = a_maskTexCoord;
}
