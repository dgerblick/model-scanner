#version 430

struct OctreeNode {
  uint hits;
  uint total;
};

struct Ray {
  vec4 origin;
  vec4 dir;
};

layout(std430, binding = 0) volatile buffer octree {
  OctreeNode nodes[];
};

uniform bool maskMode;
uniform sampler2D image;
uniform vec2 screenSize;
uniform mat4 invProj;
uniform mat4 invModelView;

out vec4 fragColor;

Ray getRay(vec2 screenCoord, mat4 invProj, mat4 invModelView) {
  vec4 screenRay = vec4(2.0 * screenCoord - 1.0, -1.0, 1.0);
  Ray ray;
  ray.origin = invModelView * vec4(0.0, 0.0, 0.0, 1.0);
  ray.dir = invModelView * invProj * screenRay;
  ray.dir = normalize(ray.dir / ray.dir.w - ray.origin);
  return ray;
}

vec4 mask() {
  vec2 screenCoord = gl_FragCoord.xy / screenSize;
  vec4 pixel = texture(image, screenCoord);
  if (pixel.r + pixel.g + pixel.b < 2)
    return pixel;
  else
    return vec4(vec3(0), 1);
}

vec4 render() {
  vec2 screenCoord = gl_FragCoord.xy / screenSize;
  Ray ray = getRay(screenCoord, invProj, invModelView);
  vec4 point = ray.origin + ray.dir * (-ray.origin.z / ray.dir.z);
  if (length(point.xy) <= 0.08333333333 / 2.0)
    return vec4(1);
  else
    return texture(image, screenCoord);
}

void main() {
  fragColor = maskMode ? mask() : render();
}