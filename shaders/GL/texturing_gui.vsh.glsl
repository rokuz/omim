attribute vec2 a_position;
attribute vec2 a_texCoords;

uniform mat4 u_modelView;
uniform mat4 u_projection;

varying vec2 v_texCoords;

void main()
{
  gl_Position = vec4(a_position, 0, 1) * u_modelView * u_projection;
  v_texCoords = a_texCoords;
}
