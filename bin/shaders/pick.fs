#version 330 core
layout (location = 0) out uint FragID;

in vec2 TexCoords;

uniform uint ID;

void main() {    
  FragID = ID;
}