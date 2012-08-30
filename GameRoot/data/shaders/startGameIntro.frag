uniform int radius;
uniform ivec2 center;
uniform sampler2D tex;
varying vec2 texCoords;

void main(void)
{
	// Get pixel pos
	// Compare with radius / center
	if (distance(vec2(gl_FragCoord), vec2(center)) < float(radius))
		gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
	else
		gl_FragColor = texture2D(tex, texCoords);
}
