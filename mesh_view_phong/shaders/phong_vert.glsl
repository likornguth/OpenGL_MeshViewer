#version 140

#define MAX_LIGHTS 10
#define EPSILON 0.000001

// input vertex attributes
in vec3 position;
in vec3 normal;

uniform mat4 mv_matrix;
uniform mat4 norm_matrix;
uniform mat4 proj_matrix;

// output attributes
out vec4 vertex_color;
out vec3 v_position;
out vec3 v_normal;
out vec3 view_vec;
out vec3 normal_vec;

void main(void)
{
  v_position = vec3(mv_matrix * vec4(position,1));
  vec4 mv_position = mv_matrix * vec4(position, 1);
  gl_Position = proj_matrix * mv_position;

  normal_vec = normalize((norm_matrix * vec4(normal, 0)).xyz);
  view_vec = normalize(-mv_position.xyz);
  // assign color to red 
  vertex_color = vec4(0, 0, 0, 1);
  // assign v_normal to vertex normal
  v_normal = normal;

}