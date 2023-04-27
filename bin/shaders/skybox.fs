#version 330 core
out vec4 FragColor;

in vec3 textureDir;
uniform samplerCube skybox;
uniform samplerCube cpTex;



void main() {  
  //FragColor = texture(skybox, textureDir);
  vec3 col;
  if(gl_FragCoord.x>0) {
    col = texture(skybox, textureDir).rgb;
  }
  else {
    col = texture(cpTex, textureDir).rgb;
  }
  col = max(vec3(0.0), col);
  col = col/(vec3(1.0)+col);
  col = pow(col, vec3(1.0/2.2));
  FragColor = vec4(col, 1.0);
}