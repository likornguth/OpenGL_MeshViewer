#version 140
#define MAX_LIGHTS 10
#define EPSILON 0.000001

// output frag color
out vec4 FragColor;

// input from vertex shader
in vec4 vertex_color;
in vec3 v_position;
in vec3 v_normal;
in vec3 view_vec;
in vec3 normal_vec;


struct PointLight {
  vec3 position;
  vec3 intensity;
  vec3 ambient;
};

struct PhongMaterial {
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;
};

// uniform attributes - from application?
uniform PointLight point_lights[MAX_LIGHTS];
uniform uint point_light_count;
uniform PhongMaterial material;


vec3 Illuminate(vec3 vertex_position, uint i, vec3 view_vec, vec3 normal_vec)
{
  // compute irradiance
  vec3 light_vec = point_lights[i].position - vertex_position;
  float dist2 = max(EPSILON, dot(light_vec, light_vec));
  light_vec = normalize(light_vec);
  vec3 irradiance = point_lights[i].intensity * (max(0, dot(light_vec, normal_vec))
					     / dist2);

  // ambient coefficient
  vec3 out_color = point_lights[i].ambient * material.ambient;

  // specular coefficient
  vec3 half_vec = normalize((light_vec + view_vec) * .5);
  float half_dot_normal = dot(half_vec, normal_vec);
  vec3 specular_coeff = vec3(0);
  if (half_dot_normal > 0)
    specular_coeff = material.specular*pow(half_dot_normal, material.shininess);

  // compute out color
  out_color += (material.diffuse + specular_coeff) * irradiance;
  return out_color;
}

void main(void)
{
  FragColor = vertex_color;
  for (uint i = 0u; i < point_light_count; ++i)
    FragColor.xyz += Illuminate(v_position, i, view_vec, normal_vec);


}