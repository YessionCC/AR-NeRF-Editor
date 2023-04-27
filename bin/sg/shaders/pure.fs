#version 330 core
out float FragColor;

in vec3 vNormal;

void main() {  
  FragColor = 0.0;  
  //FragColor = (abs(vNormal.z)+0.2)*vec4(0.8,0.8,0.8,1.0);
}