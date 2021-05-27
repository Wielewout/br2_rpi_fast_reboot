#extension GL_OES_EGL_image_external : require

uniform samplerExternalOES tex;
uniform samplerExternalOES tex2;

varying vec2 texcoord;

void main(void) {
  
  float roundedy = (floor((texcoord.y * 1088.0)/16.0) / 1088.0) * 16.0;
  
  vec4 c;
  vec4 lv, rv;

  float thresh = %4.2f;
  float l, r, y, y_inc, x_inc;
  float diff = 0.0;
  float diffbit = 1.0/256.0;
  int count = 0;
  vec2 coord;

  y = roundedy;
  y_inc = 1.0/1088.0;

  coord = vec2(texcoord.x, y);

  for (int j = 0; j < 8; j++)
  {

    lv = texture2D(tex, coord);
    rv = texture2D(tex2, coord);
    y += y_inc;
    coord = vec2(texcoord.x, y);

    l = dot(lv.rgb, vec3(0.299, 0.587, 0.114));
    r = dot(rv.rgb, vec3(0.299, 0.587, 0.114));

    if (abs(l - r) > thresh)
    {
      diff = diff + diffbit;
      count++;
    }

    diffbit += diffbit;
  }

  c[0] = diff + 1.0/512.0;
  diff = 0.0;
  diffbit = 1.0/256.0;
  
  for (int j = 8; j < 16; j++) {
    lv = texture2D(tex, coord);
    rv = texture2D(tex2, coord);
    y += y_inc;
    coord = vec2(texcoord.x, y);

    l = dot(lv.rgb, vec3(0.299, 0.587, 0.114));
    r = dot(rv.rgb, vec3(0.299, 0.587, 0.114));

    if (abs(l - r) > thresh)
    {
      diff = diff + diffbit;
      count++;
    }

    diffbit += diffbit;
  }

  c[1] = diff + 1.0/512.0;
  
  //if (count < 6)
  ///{
  //  c[0] = 0.0;
  //  c[1] = 0.0;
  //}
  //else if (count > 10)
  //{
  //  c[0] = 1.0;
  //  c[1] = 1.0;
  //}

  c[2] = 0.0;
  c[3] = 0.0;
  gl_FragColor = c;
  
}
