#version 330 core
out vec4 FragColor;

in vec3 textureDir;
uniform sampler2D Tex;

void main() {
  vec3 v = normalize(textureDir);
  vec2 uv = vec2(atan(v.z, v.x), -asin(v.y));
  uv *= vec2(0.1591, 0.3183); // 1.0/(2PI, PI)
  uv += 0.5;
  FragColor = texture(Tex, uv);
}