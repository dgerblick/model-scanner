uniform sampler2D image;
uniform vec2 screenSize;
uniform mat4 invProj;
uniform mat4 invModelView;

out vec4 fragColor;

void main() {
  vec2 texCoord = gl_FragCoord.xy / screenSize;
  Ray ray = getRay(texCoord, invProj, invModelView);
  vec4 point = ray.origin + ray.dir * (-ray.origin.z / ray.dir.z);
  if (length(point.xy) <= 0.08333333333 / 2.0)
    fragColor = vec4(1);
  else
    fragColor = texture(image, texCoord);

  // vec2 screenCoord = gl_FragCoord.xy / screenSize;
  // vec4 pixel = texture(image, screenCoord);
  // if (pixel.r + pixel.g + pixel.b < 2)
  //  gl_FragColor = pixel;
  // else
  //  gl_FragColor = vec4(vec3(0), 1);
}
