#version 130

struct Ray {
  vec4 origin;
  vec4 dir;
};

Ray getRay(vec2 screenCoord, mat4 invProj, mat4 invModelView) {
  vec4 screenRay = vec4(2.0 * screenCoord - 1.0, -1.0, 1.0);
  Ray ray;
  ray.origin = invModelView * vec4(0.0, 0.0, 0.0, 1.0);
  ray.dir = invModelView * invProj * screenRay;
  ray.dir = normalize(ray.dir / ray.dir.w - ray.origin);
  return ray;
}
