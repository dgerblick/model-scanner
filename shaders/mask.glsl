uniform sampler2D image;
uniform vec2 screenSize;
uniform mat4 invMVP;

void main() {
  vec2 screenCoord = gl_FragCoord.xy / screenSize;
  vec4 pixel = texture(image, screenCoord);
  if (pixel.r + pixel.g + pixel.b < 2)
    gl_FragColor = pixel;
  else
    gl_FragColor = vec4(vec3(0), 1);
}
