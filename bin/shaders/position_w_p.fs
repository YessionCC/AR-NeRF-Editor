#version 330 core
out vec3 FragColor;

in vec3 PositionWorld;

void main() {    
  FragColor = PositionWorld;
}