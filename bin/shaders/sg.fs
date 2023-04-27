
#version 330 core
out vec4 FragColor;

const float PI = 3.14159265359;
const float LOG210 = 3.3219280949;
const float EPSILON = 1e-6;
const int LightSGNum = 32;
const int PCAcoeffNum = 32;

in vec2 TexCoords;
in vec3 NormalWorld;
in vec3 PositionWorld;

//in float PCA_coeff[32];//

struct SG {
  vec3 Amplitude;
  vec3 Axis;
  float Sharpness;
};

uniform vec3 CamPos;
uniform mat4 V;
uniform SG lSGs[LightSGNum];
// axis project and lambda to 0~1 and 2pi/lbd*(1-exp(-lbd))
//uniform vec4 Axis2DCoord[LightSGNum]; // CPU calculate

uniform bool use_svBRDF;

uniform vec3 _albedo;
uniform float _metallic;
uniform float _rough;

uniform sampler2D Albedo;
uniform sampler2D Metallic;
uniform sampler2D Roughness;

// uniform sampler2DArray PCA_components;//
// uniform sampler2D PCA_mean;//
// uniform sampler2D Fh_tab;//

vec3 EvaluateSG(SG sg, vec3 dir) {
  float cosAngle = dot(dir, sg.Axis);
  return sg.Amplitude * exp(sg.Sharpness * (cosAngle - 1.0f));
}

SG SGProduct(SG x, SG y) {
  vec3 um = (x.Sharpness * x.Axis + y.Sharpness * y.Axis) /
    (x.Sharpness + y.Sharpness);
  float umLength = length(um);
  float lm = x.Sharpness + y.Sharpness;

  SG res;
  res.Axis = um * (1.0f / umLength);
  res.Sharpness = lm * umLength;
  res.Amplitude = x.Amplitude * y.Amplitude * exp(lm * (umLength - 1.0f));

  return res;
}

vec3 SGSphereInnerProduct(SG x, SG y) {
  float umLength = length(x.Sharpness * x.Axis + y.Sharpness * y.Axis);
  vec3 expo = exp(umLength - x.Sharpness - y.Sharpness) * x.Amplitude * y.Amplitude;
  float other = 1.0f - exp(-2.0f * umLength);
  return (2.0f * PI * expo * other) / umLength;
}

vec3 SGSphereIntegral(SG sg) {
  float expTerm = 1.0f - exp(-2.0f * sg.Sharpness);
  return 2 * PI * (sg.Amplitude / sg.Sharpness) * expTerm;
}

// implement from: Hemispherical Gaussians for Accurate Light Integration
vec3 SGHemisphereIntegral(SG sg, vec3 normal) {
  float cos_beta = dot(sg.Axis, normal);
  float lambda_val = max(sg.Sharpness, EPSILON);
  float inv_lambda_val = 1. / lambda_val;
  float t = sqrt(lambda_val) * (1.6988 + 10.8438 * inv_lambda_val) / 
  (1. + 6.2201 * inv_lambda_val + 10.2415 * inv_lambda_val * inv_lambda_val);

  float inv_a = exp(-t);
  float s = 0;
  if(cos_beta >= 0) {
    float inv_b = exp(-t * cos_beta);
    s = (1. - inv_a * inv_b) / (1. - inv_a + inv_b - inv_a * inv_b);
  }
  else {
    float b = exp(t * cos_beta);
    s = (b - inv_a) / ((1. - inv_a) * (b + 1.));
  }

  float A_b = 2. * PI / lambda_val * (exp(-lambda_val) - exp(-2. * lambda_val));
  float A_u = 2. * PI / lambda_val * (1. - exp(-lambda_val));

  float int_res = A_b * (1. - s) + A_u * s;
  return sg.Amplitude * int_res;
}

// SG_cos = G(dir, 32.7, 0.0315) - 31.7003
SG GetCosineSG(vec3 dir) {
  SG sg;
  sg.Amplitude = vec3(32.7080);
  sg.Sharpness = 0.0315;
  sg.Axis = dir;
  return sg;
}


//////For BRDF
SG DistributionTermSG(vec3 direction, float roughness) {
  SG distribution;
  distribution.Axis = direction;
  float m2 = roughness * roughness;
  distribution.Sharpness = 2 / m2;
  distribution.Amplitude = vec3(1.0f / (PI * m2));

  return distribution;
}

SG WarpDistributionSG(SG ndf, vec3 view) {
  SG warp;

  warp.Axis = reflect(-view, ndf.Axis);
  warp.Amplitude = ndf.Amplitude;
  warp.Sharpness = ndf.Sharpness;
  warp.Sharpness /= (4.0f * max(dot(ndf.Axis, view), EPSILON));

  return warp;
}

// Inte (hemi(n+)) L*cos dtheta
vec3 SGIrradiance(SG lSG, vec3 normal) {
  SG cosSG = GetCosineSG(normal);
  SG lcosSG = SGProduct(lSG, cosSG);

  // vec3 irradiance = SGSphereIntegral(lcosSG) - 
  //   31.7003 * SGSphereIntegral(lSG);

  vec3 irradiance = SGHemisphereIntegral(lcosSG, normal) - 
    31.7003 * SGHemisphereIntegral(lSG, normal);
  irradiance = max(irradiance, 0.0);
  return irradiance;
}

// Inte (hemi(n+)) L*cos dtheta === cos*Inte (hemi(n+)) L dtheta
vec3 SGIrradianceSimple(SG lSG, vec3 normal) {
  float cosTheta = max(0.0, dot(normal, lSG.Axis));

  vec3 irradiance = cosTheta * SGSphereIntegral(lSG);
  irradiance = max(irradiance, 0.0);
  return irradiance;
}

// F, G terms
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
  return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}  
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
  return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}   

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

/*
SG applyVisibility(int lid) {
  vec3 lidpos = Axis2DCoord[lid];
  float ssdf = texture(PCA_mean, lidpos.xy).r;
  for(int i = 0; i<PCAcoeffNum; i++) {
    ssdf += PCA_coeff[i] * texture(PCA_components, vec3(lidpos.xy, i)).r;
  }
  ssdf = 0.5*(ssdf/(PI/2)) + 0.5;
  float lbd = lidpos.z;
  float fh = texture(Fh_tab, vec2(ssdf, lbd)).r;
  SG sg = lSGs[lid];
  sg.Amplitude *= (fh/lidpos.w);
  return sg;
}
*/

vec3 SGRender(int lid, vec3 vdir, vec3 normal, vec3 albedo, float rough, float metallic) {
  SG ndf = DistributionTermSG(normal, rough);
  SG warpedNDF = WarpDistributionSG(ndf, vdir);
  SG lSG = lSGs[lid];//applyVisibility(lid);
  SG ldSG = SGProduct(lSG, ndf);
  // Inte (hemi(n+)) L*D*cos dtheta
  vec3 specIrr = SGIrradiance(ldSG, normal);
  // Inte (hemi(n+)) L*cos dtheta
  vec3 diffIrr = SGIrradiance(lSG, normal);

  vec3 wo = vdir;
  vec3 wi = warpedNDF.Axis; // ldSG.Axis ?
  vec3 wh = normalize(wo+wi);
  float NdotV = max(dot(normal, wo), 0.0);
  float NdotL = max(dot(normal, wi), 0.0);

  vec3 F0 = mix(vec3(0.04), albedo, metallic);
  vec3 F = fresnelSchlick(max(0, dot(wh, wo)), F0);
  float G = GeometrySchlickGGX(NdotV, rough)*GeometrySchlickGGX(NdotL, rough);

  vec3 Moi = F * G / (4 * NdotL * NdotV + EPSILON);
  vec3 spec_col = Moi*specIrr;
  vec3 diff_col = albedo / PI * diffIrr;

  vec3 kS = fresnelSchlickRoughness(NdotV, F0, rough); 
  vec3 kD = (vec3(1.0) - kS)*(vec3(1.0) - metallic);
  return kD*diff_col + spec_col;
}


void main() {   
  vec3 col = vec3(0.0);
  vec3 normal = normalize(NormalWorld);
  vec3 albedo; float rough; float metallic;
  if(use_svBRDF) {
    albedo = texture(Albedo, TexCoords).rgb;
    rough = texture(Roughness, TexCoords).r;
    metallic = texture(Metallic, TexCoords).r;
  }
  else {
    albedo = _albedo;
    rough = _rough;
    metallic = _metallic;
  }
  for(int i = 0; i<LightSGNum; i++) {
    vec3 vdir = normalize(CamPos - PositionWorld);
    col += SGRender(i, vdir, normal, albedo, rough, metallic);
  }

  //col = col/(col+vec3(1.0)); // HDR: tone mapping
  //col = pow(col, vec3(1.0/2.2)); // gamma correction
  FragColor.rgb = col;

  float depth = -(V*vec4(PositionWorld, 1.0)).z;
  FragColor.a = depth;
}