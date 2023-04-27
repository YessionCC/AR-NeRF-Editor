#version 330 core
out vec3 FragColor;

in vec3 PositionWorld;
uniform bool camcoord;
uniform mat4 V;

void main() {   
  float depth = gl_FragCoord.z;
  if(camcoord) {
    depth = -(V*vec4(PositionWorld, 1.0)).z;
  }

  FragColor = vec3(depth);
}