////V_TRANSCALE
#version 100
precision mediump float;
attribute vec2 aPos;
uniform mat3 uScreen;
uniform vec2 uTranslate;
uniform vec2 uScale;
uniform float uAngle;

void main()
{
	float c = cos(uAngle);
	float s = sin(uAngle);
	vec2 pos2 = mat2(c,s,-s,c)*(aPos*uScale)+uTranslate;
	vec3 pos3 = uScreen*vec3(pos2, 1.0);
	gl_Position = vec4(pos3.xy, 0.0, 1.0);
}

////V_STRAIGHT
#version 100
precision highp float;
attribute vec2 aPos;
//~ uniform mat3 uScreen;

void main()
{
	//~ vec3 pos3 = uScreen*vec3(aPos, 1.0);
	gl_Position = vec4(aPos, 0.0, 1.0);
}


////F_DFUN
#version 100
precision highp float;
uniform sampler2D uFramebuffer;
uniform vec2 uYrange;
uniform vec4 uColor;

float mem(float x, float i)
{
	float px = x*4.0+i;
	return floor(texture2D(uFramebuffer, (vec2(mod(px,128.0), floor(px/128.0))+0.5) / 128.0).a * (256.0 - 0.5));
}

vec2 fminmax(float x)
{
	vec2 t = vec2(mem(x, 1.0), mem(x, 3.0));
	t.x += 256.0*mem(x,0.0);
	t.y += 256.0*mem(x,2.0);
	return t;
}

void main()
{

	vec2 px = vec2(floor(gl_FragCoord.x), floor(gl_FragCoord.y));
	//~ float fmin = texture2D(uFramebuffer,  vec2(px.x, 0.0)).a;
	//~ float fmax = texture2D(uFramebuffer,  vec2(px.x, 1.0)).a;
	//float m = mem(floor(px.x/30.0), mod(floor(px.y/30.0), 4.0)) / 256.0;
	
	vec2 mm = fminmax(px.x);
	if (px.y > mm.x && px.y <= mm.y) {
		gl_FragColor = vec4(0.5, 0.5, 0.8, 1.0);
	} else {
		gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
	}
	//~ if (m== 0.0 ) {
		//~ gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
	//~ } else if (m == 1.0) {
		//~ gl_FragColor = vec4(0.5, 0.5, 0.5, 1.0);
	//~ } else if (m == 2.0) {
		//~ gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
	//~ } else if (m == 127.0) {
		//~ gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
	//~ } else if (m == 128.0) {
		//~ gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
	//~ } else if (m == 129.0) {
		//~ gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);
	//~ } else {
		//~ gl_FragColor = vec4(1.0, 1.0, 0.0, 1.0);
	//~ }
	//~ gl_FragColor = vec4(mem(0.0, 0.0)/256.0, mem(0.0, 1.0)/256.0, 0.0, 1.0);
}

////
