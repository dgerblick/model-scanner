uniform sampler2D image;
uniform vec2 screenSize;
uniform mat4 invProj;
uniform mat4 invModelView;

void main() {
  vec2 texCoord = gl_FragCoord.xy / screenSize;
  vec4 screenRay = vec4(2.0 * gl_FragCoord.xy / screenSize - 1.0, -1.0, 1.0);
  vec4 rayOrigin = invModelView * vec4(0.0, 0.0, 0.0, 1.0);
  vec4 rayDir = invModelView * invProj * screenRay;
  rayDir = normalize(rayDir / rayDir.w - rayOrigin);

  if (rayDir.w == 1.0) {
    gl_FragColor = texture(image, texCoord);
    return;
  }
  vec4 point = rayOrigin + rayDir * (-rayOrigin.z / rayDir.z);
  if (length(point.xy) <= 0.08333333333 / 2.0)
    gl_FragColor = vec4(1);
  else
    gl_FragColor = texture(image, texCoord);

  // vec2 screenCoord = gl_FragCoord.xy / screenSize;
  // vec4 pixel = texture(image, screenCoord);
  // if (pixel.r + pixel.g + pixel.b < 2)
  //  gl_FragColor = pixel;
  // else
  //  gl_FragColor = vec4(vec3(0), 1);
}
