#version 430

#define STACK_SIZE (64)

struct OctreeNode {
  uint hits;
  uint total;
  uint depth;
  uint _unused;
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

struct RaycastHit {
  float dist;
  uint nodeIdx;
  Ray normal;
};

layout(std430, binding = 0) volatile buffer OctreeBuffer {
  uint depth;
  uint size;
  OctreeNode nodes[];
}
octree;

uniform bool maskMode;
uniform sampler2D image;
uniform vec2 screenSize;
uniform mat4 invProj;
uniform mat4 invModelView;
uniform float threshold;

out vec4 fragColor;

Box getBox(uint nodeIdx) {
  uint idx = clamp(nodeIdx, 0, octree.size - 1);
  Box box;
  box.nodeIdx = idx;
  box.parentIdx = uint((float(idx) - 1.0) / 8.0);
  for (uint i = 0; i < 8; ++i) {
    box.childrenIdx[i] = 8 * idx + i + 1;
    if (box.childrenIdx[i] >= octree.size)
      box.childrenIdx[i] = idx;
  }
  box.minPoint = octree.nodes[idx].minPoint;
  box.maxPoint = octree.nodes[idx].maxPoint;
  return box;
}

vec4 rayAt(Ray ray, float t) {
  return ray.origin + ray.dir * t;
}

RaycastHit boxIntersect(Box box, Ray ray) {
  RaycastHit hit;
  hit.dist = 0.0;
  hit.nodeIdx = box.nodeIdx;
  hit.normal.origin = vec4(0.0);
  hit.normal.dir = vec4(0.0);
  vec3 tminVals = (box.minPoint.xyz - ray.origin.xyz) / ray.dir.xyz;
  vec3 tmaxVals = (box.maxPoint.xyz - ray.origin.xyz) / ray.dir.xyz;
  vec4 point;
  point = rayAt(ray, tminVals.x);
  if (box.minPoint.y <= point.y && point.y <= box.maxPoint.y &&
      box.minPoint.z <= point.z && point.z <= box.maxPoint.z &&
      (hit.dist == 0.0 || (0 < tminVals.x && tminVals.x < hit.dist))) {
    hit.dist = tminVals.x;
    hit.normal.dir = vec4(-1.0, 0.0, 0.0, 0.0);
  }
  point = rayAt(ray, tminVals.y);
  if (box.minPoint.x <= point.x && point.x <= box.maxPoint.x &&
      box.minPoint.z <= point.z && point.z <= box.maxPoint.z &&
      (hit.dist == 0.0 || (0 < tminVals.y && tminVals.y < hit.dist))) {
    hit.dist = tminVals.y;
    hit.normal.dir = vec4(0.0, -1.0, 0.0, 0.0);
  }
  point = rayAt(ray, tminVals.z);
  if (box.minPoint.x <= point.x && point.x <= box.maxPoint.x &&
      box.minPoint.y <= point.y && point.y <= box.maxPoint.y &&
      (hit.dist == 0.0 || (0 < tminVals.z && tminVals.z < hit.dist))) {
    hit.dist = tminVals.z;
    hit.normal.dir = vec4(0.0, 0.0, -1.0, 0.0);
  }
  point = rayAt(ray, tmaxVals.x);
  if (box.minPoint.y <= point.y && point.y <= box.maxPoint.y &&
      box.minPoint.z <= point.z && point.z <= box.maxPoint.z &&
      (hit.dist == 0.0 || (0 < tmaxVals.x && tmaxVals.x < hit.dist))) {
    hit.dist = tmaxVals.x;
    hit.normal.dir = vec4(1.0, 0.0, 0.0, 0.0);
  }
  point = rayAt(ray, tmaxVals.y);
  if (box.minPoint.x <= point.x && point.x <= box.maxPoint.x &&
      box.minPoint.z <= point.z && point.z <= box.maxPoint.z &&
      (hit.dist == 0.0 || (0 < tmaxVals.y && tmaxVals.y < hit.dist))) {
    hit.dist = tmaxVals.y;
    hit.normal.dir = vec4(0.0, 1.0, 0.0, 0.0);
  }
  point = rayAt(ray, tmaxVals.z);
  if (box.minPoint.x <= point.x && point.x <= box.maxPoint.x &&
      box.minPoint.y <= point.y && point.y <= box.maxPoint.y &&
      (hit.dist == 0.0 || (0 < tmaxVals.z && tmaxVals.z < hit.dist))) {
    hit.dist = tmaxVals.z;
    hit.normal.dir = vec4(0.0, 0.0, 1.0, 0.0);
  }
  hit.normal.origin = rayAt(ray, hit.dist);

  return hit;
}

RaycastHit octreeIntersect(Ray ray) {
  Ray normal;

  uint stack[STACK_SIZE] = uint[STACK_SIZE](0);
  int stackIdx = 0;
  stack[0] = 0;

  RaycastHit bestHit;
  bestHit.dist = 0.0;
  bestHit.nodeIdx = 0;
  bestHit.normal.origin = vec4(0.0);
  bestHit.normal.dir = vec4(0.0);

  while (stackIdx >= 0) {
    uint nodeIdx = stack[stackIdx--];
    Box box = getBox(nodeIdx);

    RaycastHit hit = boxIntersect(box, ray);
    if (hit.dist > 0) {
      float ratio =
          float(octree.nodes[nodeIdx].hits) / octree.nodes[nodeIdx].total;
      if (ratio >= threshold &&
          (bestHit.dist == 0.0 || hit.dist < bestHit.dist)) {
        bestHit = hit;
      } else if (ratio < threshold &&
                 octree.nodes[nodeIdx].depth != octree.depth) {
        for (uint i = 0; i < 8; i++) {
          stackIdx++;
          if (stackIdx >= STACK_SIZE) {
            fragColor = vec4(1.0, 0.0, 0.0, 1.0);
            return bestHit;
          }
          stack[stackIdx] = box.childrenIdx[i];
        }
      }
    }
  }

  return bestHit;
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
  bool isBackground = pixel.r + pixel.g + pixel.b < 1.5;
  Ray ray = getRay(screenCoord, invProj, invModelView);

  uint stack[STACK_SIZE] = uint[STACK_SIZE](0);
  int stackIdx = 0;
  stack[0] = 0;

  RaycastHit bestHit;
  bestHit.dist = 0.0;
  bestHit.nodeIdx = 0;
  bestHit.normal.origin = vec4(0.0);
  bestHit.normal.dir = vec4(0.0);

  while (stackIdx >= 0) {
    uint nodeIdx = stack[stackIdx--];
    Box box = getBox(nodeIdx);

    RaycastHit hit = boxIntersect(box, ray);
    if (hit.dist > 0) {
      atomicAdd(octree.nodes[nodeIdx].total, 1);
      if (isBackground)
        atomicAdd(octree.nodes[nodeIdx].hits, 1);
      if (octree.nodes[nodeIdx].depth != octree.depth) {
        for (uint i = 0; i < 8; i++) {
          stackIdx++;
          if (stackIdx >= STACK_SIZE) {
            fragColor = vec4(1.0, 0.0, 0.0, 1.0);
            return fragColor;
          }
          stack[stackIdx] = box.childrenIdx[i];
        }
      }
    }
  }

  if (isBackground)
    return pixel;
  else
    return vec4(vec3(0), 1);
}

vec4 render() {
  vec2 screenCoord = gl_FragCoord.xy / screenSize;
  Ray ray = getRay(screenCoord, invProj, invModelView);
  RaycastHit hit = octreeIntersect(ray);

  if (hit.dist == 0.0)
    return texture(image, screenCoord);

  vec4 light = normalize(vec4(0.0, 0.0, 1.0, 0.0));
  float ambient = 0.5;
  float level = ambient + (1 - ambient) * dot(hit.normal.dir, light);
  return vec4(vec3(level), 1.0);
}

void main() {
  fragColor = vec4(0);
  vec4 color = maskMode ? mask() : render();
  if (fragColor == vec4(0))
    fragColor = color;
}
