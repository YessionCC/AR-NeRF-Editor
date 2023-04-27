#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 NormalWorld;
in vec3 PositionWorld;

uniform sampler2D DepthMap;
uniform sampler2D ColorMap;

uniform mat4 VP;
uniform vec3 CamPos;

uniform vec2 CamPlane;
uniform vec2 PixSizes[9];
uniform int MaxLevel;

vec3 world2NDC01(vec3 wpos) {
  vec4 ndcPos = VP*vec4(wpos, 1.0);
  return ndcPos.xyz / ndcPos.w * 0.5 + vec3(0.5);
}

bool inScreen(vec2 spos) {
  return spos.x >= 0.0 && spos.x<=1.0 && spos.y>= 0.0 && spos.y<=1.0;
}

float LinearizeDepth(float depth) {
  float z = depth * 2.0 - 1.0;
  return (2.0 * CamPlane.x * CamPlane.y) / 
    (CamPlane.y + CamPlane.x - z * (CamPlane.y - CamPlane.x));    
}

void main() {    
  
  vec3 normal = vec3(0,1,0);//normalize(NormalWorld);
  vec3 cam2Pos = normalize(PositionWorld - CamPos);
  vec3 refl = reflect(cam2Pos, normal);
  //refl = vec3(0,1,-1);

  vec3 center = world2NDC01(PositionWorld);
  vec3 refl_p = world2NDC01(PositionWorld+refl);
  vec2 ss_dir = refl_p.xy - center.xy;

  int axis = ss_dir.x>ss_dir.y ? 0:1;
  vec3 dir = (refl_p.xyz - center.xyz) / ss_dir[axis];

  vec3 pos = center;
  int level = 0; float cdep = 0.0;
  int maxloop = 256;
  while(level >= 0 && inScreen(pos.xy) && --maxloop>0) {
    cdep = textureLod(DepthMap, pos.xy, float(level)).r;
    if(cdep >= pos.z) {
      pos += 4*dir*PixSizes[level][axis];
      level = min(level+1, MaxLevel);
    }
    else level --;
    level = 0;
  }

  vec3 col = vec3(0.5);
  float linearDepSub = abs(LinearizeDepth(cdep) - LinearizeDepth(pos.z));
  if(inScreen(pos.xy) && linearDepSub < 0.8) {
    col=mix(texture(ColorMap, pos.xy).rgb, col, 0.5);
  }
  FragColor = vec4(col, 1.0);
  
}