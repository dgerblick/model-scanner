#version 430

struct OctreeNode {
  uint hits;
  uint total;
  uint _unused[2];
  vec4 minPoint;
  vec4 maxPoint;
};

struct Box {
  uint nodeIdx;
  uint parentIdx;
  uint childrenIdx[8];
  uint depth;
  vec4 minPoint;
  vec4 maxPoint;
};

struct Ray {
  vec4 origin;
  vec4 dir;
};

layout(std430, binding = 0) volatile buffer OctreeBuffer {
  uint depth;
  uint size;
  uint _unused[2];
  OctreeNode nodes[];
}
octree;

uniform bool maskMode;
uniform sampler2D image;
uniform vec2 screenSize;
uniform mat4 invProj;
uniform mat4 invModelView;

out vec4 fragColor;

Box getBox(uint nodeIdx) {
  Box box;
  box.nodeIdx = nodeIdx;
  box.parentIdx = uint((float(nodeIdx) - 1.0) / 8.0);
  box.childrenIdx[0] = 8 * nodeIdx + 1;
  box.childrenIdx[1] = 8 * nodeIdx + 2;
  box.childrenIdx[2] = 8 * nodeIdx + 3;
  box.childrenIdx[3] = 8 * nodeIdx + 4;
  box.childrenIdx[4] = 8 * nodeIdx + 5;
  box.childrenIdx[5] = 8 * nodeIdx + 6;
  box.childrenIdx[6] = 8 * nodeIdx + 7;
  box.childrenIdx[7] = 8 * nodeIdx + 8;
  box.minPoint = octree.nodes[nodeIdx].minPoint;
  box.maxPoint = octree.nodes[nodeIdx].maxPoint;
  return box;
}

vec4 rayAt(Ray ray, float t) {
  return ray.origin + ray.dir * t;
}

float boxIntersect(Box box, Ray ray) {
  float hit = 0.0;
  vec3 tminVals = (box.minPoint.xyz - ray.origin.xyz) / ray.dir.xyz;
  vec3 tmaxVals = (box.maxPoint.xyz - ray.origin.xyz) / ray.dir.xyz;
  vec4 point;
  point = rayAt(ray, tminVals.x);
  if (box.minPoint.x <= point.x && point.x <= box.maxPoint.x &&
      box.minPoint.y <= point.y && point.y <= box.maxPoint.y &&
      box.minPoint.z <= point.z && point.z <= box.maxPoint.z &&
      (hit == 0 || tminVals.x < hit)) {
    hit = tminVals.x;
  }
  point = rayAt(ray, tminVals.y);
  if (box.minPoint.x <= point.x && point.x <= box.maxPoint.x &&
      box.minPoint.y <= point.y && point.y <= box.maxPoint.y &&
      box.minPoint.z <= point.z && point.z <= box.maxPoint.z &&
      (hit == 0 || tminVals.x < hit)) {
    hit = tminVals.y;
  }
  point = rayAt(ray, tminVals.z);
  if (box.minPoint.x <= point.x && point.x <= box.maxPoint.x &&
      box.minPoint.y <= point.y && point.y <= box.maxPoint.y &&
      box.minPoint.z <= point.z && point.z <= box.maxPoint.z &&
      (hit == 0 || tminVals.x < hit)) {
    hit = tminVals.z;
  }
  point = rayAt(ray, tmaxVals.x);
  if (box.minPoint.x <= point.x && point.x <= box.maxPoint.x &&
      box.minPoint.y <= point.y && point.y <= box.maxPoint.y &&
      box.minPoint.z <= point.z && point.z <= box.maxPoint.z &&
      (hit == 0 || tminVals.x < hit)) {
    hit = tmaxVals.x;
  }
  point = rayAt(ray, tmaxVals.y);
  if (box.minPoint.x <= point.x && point.x <= box.maxPoint.x &&
      box.minPoint.y <= point.y && point.y <= box.maxPoint.y &&
      box.minPoint.z <= point.z && point.z <= box.maxPoint.z &&
      (hit == 0 || tminVals.x < hit)) {
    hit = tmaxVals.y;
  }
  point = rayAt(ray, tmaxVals.z);
  if (box.minPoint.x <= point.x && point.x <= box.maxPoint.x &&
      box.minPoint.y <= point.y && point.y <= box.maxPoint.y &&
      box.minPoint.z <= point.z && point.z <= box.maxPoint.z &&
      (hit == 0 || tminVals.x < hit)) {
    hit = tmaxVals.z;
  }

  return hit;
}

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
  if (pixel.r + pixel.g + pixel.b < 1.5)
    return pixel;
  else
    return vec4(vec3(0), 1);
}

vec4 render() {
  vec2 screenCoord = gl_FragCoord.xy / screenSize;
  Box box = getBox(0);
  Ray ray = getRay(screenCoord, invProj, invModelView);
  float t = boxIntersect(box, ray);
  if (t == 0.0)
    return texture(image, screenCoord);
  else
    return vec4(1);
}

void main() {
  fragColor = maskMode ? mask() : render();
}
