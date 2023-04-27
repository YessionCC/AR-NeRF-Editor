#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 MVP;
uniform mat4 M;
uniform mat4 LightVP;

out vec3 PositionWorld;
out vec3 sTexCoords;

void main() {
  PositionWorld = vec3(M*vec4(aPos, 1.0));
  vec4 ProjPosInLight = LightVP*M*vec4(aPos, 1.0);

  // for shadow mapping, we should do what will be done for gl_Position manually
  // and should NOTICE after divide by w and *0.5+0.5
  // the value for x,y,z are not in [0, 1]
  // for x,y, it will texture pixels out of range(related to wrapping mode)
  // for z, we should NOTICE: if z>1, and shadowmap texture is [0, 1]
  // it maybe will be taken as in shadow
  // however, the 1 in texture always represent for infinity far
  sTexCoords = ProjPosInLight.xyz / ProjPosInLight.w;
  sTexCoords.xyz = sTexCoords.xyz*0.5+0.5;
  //sTexCoords.z = min(sTexCoords.z, 1);
  gl_Position = MVP * vec4(aPos, 1.0);
}