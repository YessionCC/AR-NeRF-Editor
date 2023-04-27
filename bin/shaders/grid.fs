#version 330 core
out vec4 FragColor;

in vec3 worldCoordNear;
in vec3 worldCoordFar;

uniform float scale;
uniform mat4 VP;

void main() {    
  float nearY = worldCoordNear.y;
  float farY = worldCoordFar.y;
  float t = -nearY/(farY - nearY);

  vec3 itscPos = worldCoordNear + t*(worldCoordFar - worldCoordNear);
  itscPos *= scale;
  vec2 screenPartial = fwidth(itscPos.xz);

  vec2 gridPos = floor(itscPos.xz + 0.5);
  vec2 gridDist = abs(fract(itscPos.xz - 0.5) - 0.5) / screenPartial;

  float dist = min(gridDist.x, gridDist.y);

  float fade = max(1.0 - max(t / 0.5, 0.0), 0.0);
  float aa = 1.0 - min(dist, 1.0);

  vec4 clipCoord = VP * vec4(itscPos, 1.0);
  /*
  NOTICE !!
    in NDC space, xyz~[-1, 1]
    to the screen space, xy ~ [0~width], [0~height]
    and z(depth) will be mapped to [0, 1],
    the transform above runs auto by opengl

    the result after transform will input frag shader as gl_FragCoord
    and gl_FragCoord.z will be the output depth auto
    so the gl_FragDepth range in [0, 1],

    we manually calc depth using VP*pos
    so we need to manually handle these auto processing
    like: clip space -> ndc space (z/w)
          ndc space -> output depth (0.5*z+0.5)
  */
  float depth = 0.5*(clipCoord.z / clipCoord.w) + 0.5;

  if(gridPos.x == 0.0 && dist == gridDist.x)
    FragColor = vec4(0.6, 0.0, 0.0, fade*aa)*float(t>0);
  else if(gridPos.y == 0.0 && dist == gridDist.y)
    FragColor = vec4(0.0, 0.6, 0.0, fade*aa)*float(t>0);
  else
    FragColor = vec4(0.3, 0.3, 0.3, fade*aa)*float(t>0);
  gl_FragDepth = depth;
}