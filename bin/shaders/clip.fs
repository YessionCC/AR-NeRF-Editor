#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texMain;

void main() {   
  //FragColor = vec4(texture(texMain, TexCoords).rrr, 1.0);
  // vec3 normal = texture(texMain, TexCoords).rgb;
  // FragColor = vec4(normal, 1.0);
  
  FragColor = texture(texMain, TexCoords);
}