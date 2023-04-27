#version 330 core
in vec4 FragPos;

uniform vec3 LightPos;
uniform float LightPlaneFar;

/*
unlike shadow.fs that auto use gl_Position.z as the depth output,
here we calc all in world space and take the distance as the depth output,
just for the convenience when comparing depth using cubemap.
In fact, in shadow.fs, we can also use this way
*/

void main() {
  // get distance between fragment and light source
  float lightDistance = length(FragPos.xyz - LightPos);

  // map to [0;1] range by dividing by far_plane
  // (may exist depth larger than 1, but no effects)
  lightDistance = lightDistance / LightPlaneFar;

  // write this as modified depth
  gl_FragDepth = lightDistance;
}