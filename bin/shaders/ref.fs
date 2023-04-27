#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 NormalWorld;
in vec3 PositionWorld;

uniform samplerCube skybox;
uniform vec3 CamPos;

void main() {    
  vec3 cam2pos = normalize(PositionWorld - CamPos);
  // reflect(dir, normal): two params all need to be normalized
  // dir: camera position to reflection position
  
  // !!!!NOTICE: When using the interpolated normal, it should be normalized first!!!
  vec3 dirRefl = reflect(cam2pos, normalize(NormalWorld));
  FragColor = texture(skybox, dirRefl);
}