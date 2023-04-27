#version 330 core
out float FragColor;

in vec2 TexCoords;

uniform float level;
uniform vec2 pixSize; // 1.0 / texSize
uniform sampler2D texMain;

void main() {   
  vec2 spos = gl_FragCoord.xy - vec2(0.5);
  vec2 tpos = spos*2 + vec2(0.5);
  float min_d = textureLod(texMain, tpos*pixSize, level - 1).r;
  min_d = min(min_d, textureLod(texMain, (tpos + vec2(0.0, 1.0))*pixSize, level-1).r);
  min_d = min(min_d, textureLod(texMain, (tpos + vec2(1.0, 1.0))*pixSize, level-1).r);
  min_d = min(min_d, textureLod(texMain, (tpos + vec2(1.0, 0.0))*pixSize, level-1).r);

  FragColor = min_d;
}