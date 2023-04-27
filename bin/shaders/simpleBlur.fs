#version 330 core
out float FragColor;

in vec2 TexCoords;

uniform vec2 ScreenSize;
uniform int BlurRadius;
uniform sampler2D SSAOInput;

void main() {   
  vec2 texelSize = 1.0 / ScreenSize;
  // there should have a temp to record values, if directly use FragColor,
  // it will cause unexpected result
  float res = 0.0;
  for(int i = -BlurRadius; i<=BlurRadius; i++) {
    for(int j = -BlurRadius; j<=BlurRadius; j++) {
      res += texture(SSAOInput, vec2(i, j)*texelSize+TexCoords).r;
    }
  }
  float blurArea = (BlurRadius*2.0+1.0)*(BlurRadius*2.0+1.0);
  FragColor = res / blurArea;
}