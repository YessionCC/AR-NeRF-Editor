#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 NormalWorld;
in vec3 TangentWorld;
in vec3 PositionWorld;

uniform vec3 LightPos;
uniform vec3 CamPos;
uniform vec3 LightRadiance;

uniform samplerCube EnvDiffMap;
uniform samplerCube EnvSpecMap1;
uniform sampler2D EnvSpecMap2;

uniform sampler2D Albedo;
uniform sampler2D Metallic;
uniform sampler2D Roughness;
uniform sampler2D AO;
uniform sampler2D NormalMap;

const float PI = 3.14159265359;
/*
F
costheta: dot(view, n), also can be calc as dot(h, n)
F0: surface reflection at zero incidence
*/ 
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
  return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}  
/***/

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
  return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}   

/*D*/ 
float DistributionGGX(vec3 N, vec3 H, float roughness) {
  float a      = roughness*roughness;
  float a2     = a*a;
  float NdotH  = max(dot(N, H), 0.0);
  float NdotH2 = NdotH*NdotH;

  float nom   = a2;
  float denom = (NdotH2 * (a2 - 1.0) + 1.0);
  denom = PI * denom * denom;

  return nom / denom;
}
/***/

/*G*/
float GeometrySchlickGGX(float NdotV, float roughness) {
  float r = (roughness + 1.0);
  float k = (r*r) / 8.0;

  float nom   = NdotV;
  float denom = NdotV * (1.0 - k) + k;

  return nom / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
  float NdotV = max(dot(N, V), 0.0);
  float NdotL = max(dot(N, L), 0.0);
  float ggx2  = GeometrySchlickGGX(NdotV, roughness);
  float ggx1  = GeometrySchlickGGX(NdotL, roughness);

  return ggx1 * ggx2;
}
/***/

// fr = c/pi + DFG/[4(n.wi)(n.wo)]
// integrate w: fr*cos*L*dw
void main() {  
  vec3 albedo = texture(Albedo, TexCoords).rgb;
  vec3 metallic = texture(Metallic, TexCoords).rgb;
  vec3 ao = texture(AO, TexCoords).rgb;
  float roughness = texture(Roughness, TexCoords).r;
  vec3 normalmap = texture(NormalMap, TexCoords).xyz;

  vec3 normal = normalize(NormalWorld);
  vec3 tangent = normalize(TangentWorld);
  vec3 btangent = cross(normal, tangent);
  mat3 TBN = mat3(tangent, btangent, normal);
  // NOTICE: When use normalMap, must transform value[0, 1] to [-1, 1]
  // and also need to normalize the vec after multipy be TBN
  normal = normalize(TBN*(normalmap*2.0 - 1.0));

  vec3 frag2Cam = CamPos - PositionWorld;
  vec3 frag2Lt = LightPos - PositionWorld;
  vec3 wo = normalize(frag2Cam);
  vec3 wi = normalize(frag2Lt);
  vec3 wh = normalize(wo+wi); // the micro-normal when reflect 
  float frag2LtDist = length(frag2Lt);

  vec3 F0 = mix(vec3(0.04), albedo, metallic);
  vec3 F = fresnelSchlick(max(0, dot(wh, wo)), F0);
  float D = DistributionGGX(normal, wh, roughness);
  float G = GeometrySmith(normal, wo, wi, roughness);

  vec3 reflGGX = D*G*F/(0.001+4*max(0, dot(normal, wo)*dot(normal, wi)));
  vec3 c = (vec3(1.0) - F)*(vec3(1.0)-metallic)*albedo;

  vec3 fr = c/PI + reflGGX;
  vec3 Lo = fr*max(0, dot(normal, wi))*LightRadiance; 
  // above is the direct light part (point light delta distribution integration)

  vec3 envDiff = texture(EnvDiffMap, normal).rgb;
  vec3 kS = fresnelSchlickRoughness(max(dot(normal, wo), 0.0), F0, roughness); 
  vec3 kD = (vec3(1.0) - kS)*(vec3(1.0) - metallic);
  vec3 ambient_diff = kD*envDiff*albedo;

  vec3 R = reflect(-wo, normal);
  vec3 spec1 = textureLod(EnvSpecMap1, R, roughness*4).rgb;
  vec2 spec2 = texture(EnvSpecMap2, vec2(dot(normal, wo), roughness)).rg;
  vec3 ambient_spec = spec1*(kS*spec2.x+spec2.y);

  vec3 color = Lo+(ambient_diff+ambient_spec)*ao;

  color = color/(color+vec3(1.0)); // HDR: tone mapping
  color = pow(color, vec3(1.0/2.2)); // gamma correction
  FragColor = vec4(color, 1.0);
}