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

////V_GEOM
#version 100
precision mediump float;
attribute vec2 aPos;
uniform vec2 uOrigin;
uniform vec2 uVps;
uniform vec2 uTranslate;
uniform vec2 uScale;
uniform vec2 uScreen;
uniform float uAngle;

void main()
{
	float c = cos(uAngle);
	float s = sin(uAngle);
	vec2 pos = mat2(c, s, -s, c)*(aPos*uScale)+uTranslate;
	pos = ((pos/uVps + uOrigin) - uScreen*0.5)/ (0.5*uScreen) ;
	//~ vec3 pos = uScreen*vec3(pos2, 1.0);
	gl_Position = vec4(pos, 0.0, 1.0);
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

////F_FLAT
#version 100
precision mediump float;
uniform vec4 uColor;

void main()
{
	gl_FragColor = uColor;
}

////F_DFUN
#version 100
precision highp float;
uniform sampler2D uFramebuffer;
//~ uniform vec2 uYrange;
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
	
	vec2 mm = fminmax(px.x);
	if (px.y < mm.x || px.y >= mm.y)
		discard;
	
	gl_FragColor = uColor;
}


////F_GRID
#version 100
precision highp float;
uniform vec2 uOrigin; // unit
uniform vec2 uVps; // unit/px
uniform vec2 uMajor;
uniform vec2 uMinor;
uniform vec4 uColor;

void main()
{
	vec2 px = (gl_FragCoord.xy- uOrigin) * uVps ;
	float alpha = 0.0;
	if ( mod(px.x, uMinor.x) <= uVps.x || mod(px.y, uMinor.y) <= uVps.y)
		alpha = 0.1;
	if ( mod(px.x, uMajor.x) <= uVps.x || mod(px.y, uMajor.y) <= uVps.y)
		alpha = 0.5;
	if ( abs(px.x)/uVps.x < 1.0 || abs(px.y)/uVps.y < 1.0)
		alpha = 0.8;
	gl_FragColor = vec4(uColor.rgb, alpha*uColor.a);
}


////
