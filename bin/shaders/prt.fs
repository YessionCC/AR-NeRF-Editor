#version 330 core
out vec3 FragColor;

in vec3 SH9_1;
in vec3 SH9_2;
in vec3 SH9_3;
uniform vec3 EnvSH9[9];

void main() {
  vec3 col = 
    EnvSH9[0]*SH9_1.x+EnvSH9[1]*SH9_1.y+EnvSH9[2]*SH9_1.z +
    EnvSH9[3]*SH9_2.x+EnvSH9[4]*SH9_2.y+EnvSH9[5]*SH9_2.z +
    EnvSH9[6]*SH9_3.x+EnvSH9[7]*SH9_3.y+EnvSH9[8]*SH9_3.z;
  col = max(vec3(0.0), col);
  col = col/(col+vec3(1.0)); 
  col = pow(col, vec3(1.0/2.2)); 
  FragColor = col;
}