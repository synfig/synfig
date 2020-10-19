/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/internal/glsl/blend_fragment.glsl
**	\brief Color Blend Shader
**
**	$Id$
**
**	\legal
**	......... ... 2015 Ivan Mahonin
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**	\endlegal
*/
/* ========================================================================= */

#version 330 core
#define method_#0

uniform float amount;
uniform sampler2D sampler_dest;
uniform sampler2D sampler_src;
layout(location = 0) out vec4 out_color;

const float epsilon = 1e-6;

const mat3 encode_yuv = mat3(
	0.299f, 0.587f, 0.114f,
	-0.168736, -0.331264, 0.5,
	0.5, -0.418688, -0.081312 );

const mat3 decode_yuv = mat3(
	1, 0, 1.402,
	1, -0.344136, -0.714136,
	1, 1.772, 0 );


void main()
{
	ivec2 coord = ivec2(floor(gl_FragCoord)); 
	vec4 dest = texelFetch(sampler_dest, coord, 0);
	vec4 src = texelFetch(sampler_src, coord, 0);

#ifdef method_composite

	src.a *= amount;
	dest.rgb *= dest.a;
	dest.a = src.a + dest.a - src.a*dest.a;
	if (abs(dest.a) > epsilon)
		dest.rgb = ((src.rgb - dest.rgb)*src.a + dest.rgb)/dest.a;
	else
		dest = vec4(0);

#endif
#ifdef method_behind

	dest.a *= amount;
	src.rgb *= src.a;
	src.a = src.a + dest.a - src.a*dest.a;
	if (abs(src.a) > epsilon)
		dest = vec4(((dest.rgb - src.rgb)*dest.a + src.rgb)/src.a, src.a);
	else
		dest = vec4(0);

#endif
#ifdef method_straight

	dest.rgb *= dest.a;
	dest.a = (src.a - dest.a)*amount + dest.a;
	if (abs(dest.a) > epsilon)
		dest.rgb =((src.rgb*src.a - dest.rgb)*amount + dest.rgb)/dest.a;
	else
		dest = vec4(0);

#endif
#ifdef method_onto

	dest.rgb = (src.rgb - dest.rgb)*(src.a*amount) + dest.rgb;

#endif
#ifdef method_straightonto

	dest.a = src.a*amount - amount + 1;
	if (abs(dest.a) > epsilon)
		dest.rgb =((src.rgb*src.a - dest.rgb)*amount + dest.rgb)/dest.a;
	else
		dest = vec4(0);

#endif
#ifdef method_brighten

	dest.rgb = max(dest.rgb, src.rgb*(src.a*amount));

#endif
#ifdef method_darken

	dest.rgb = min(dest.rgb, src.rgb*(src.a*amount) + vec3(1 - amount));

#endif
#ifdef method_add

	dest.a = dest.a + src.a*amount;
	dest.rgb = (dest.rgb*dest.a + src.rgb*(src.a*amount))/dest.a;

#endif
#ifdef method_subtract

	dest.a = dest.a - src.a*amount;
	dest.rgb = (dest.rgb*dest.a - src.rgb*(src.a*amount))/dest.a;

#endif
#ifdef method_difference

	dest.a = abs(dest.a - src.a*amount);
	dest.rgb = abs(dest.rgb*dest.a - src.rgb*(src.a*amount))/dest.a;

#endif
#ifdef method_multiply

	if (amount < 0) src = vec4(1) - src;
	dest.rgb = (dest.rgb*src.rgb - dest.rgb)*(src.a*abs(amount)) + dest.rgb;

#endif
#ifdef method_divide

	src.rgb = sign(sign(src.rgb) + vec3(epsilon))*max(abs(src.rgb), vec3(epsilon));
	dest.rgb = (dest.rgb/src.rgb - dest.rgb)*(src.a*amount) + dest.rgb;
	
#endif
#ifdef method_color

	vec4 tmp = vec4( vec3( dot(dest.rgb, encode_yuv[0]), 
					       dot(src.rgb, encode_yuv[1]),
					       dot(src.rgb, encode_yuv[2]) ) * decode_yuv,
					 dest.a );
	dest = (tmp - dest)*(amount*src.a) + dest;

#endif
#ifdef method_luminance

	vec4 tmp = vec4( vec3( dot(src.rgb, encode_yuv[0]), 
					       dot(dest.rgb, encode_yuv[1]),
					       dot(dest.rgb, encode_yuv[2]) ) * decode_yuv,
					 dest.a );
	dest = (tmp - dest)*(amount*src.a) + dest;
	
#endif
#ifdef method_hue

	vec4 tmp = vec4(dest.rgb*encode_yuv, dest.a); 
	tmp.yz = normalize(vec2( dot(src.rgb, encode_yuv[1]),
	                         dot(src.rgb, encode_yuv[2]) ))*length(tmp.yz);
	tmp.xyz *= decode_yuv;
	dest = (tmp - dest)*(amount*src.a) + dest;
	
#endif
#ifdef method_saturation

	vec4 tmp = vec4(dest.rgb*encode_yuv, dest.a);
	tmp.yz = length(vec2( dot(src.rgb, encode_yuv[1]),
	                      dot(src.rgb, encode_yuv[2]) ))*normalize(tmp.yz);
	dest = (tmp - dest)*(amount*src.a) + dest;

#endif
#ifdef method_alphabrighten

	if (src.a < dest.a*amount)
		dest = vec4(src.rgb, src.a*amount);

#endif
#ifdef method_alphadarken

	if (src.a*amount > dest.a)
		dest = vec4(src.rgb, src.a*amount);

#endif
#ifdef method_screen

	if (amount < 0) src = vec4(1) - src;
	src.rgb = vec3(1) - (vec3(1) - src.rgb)*(vec3(1) - dest.rgb);
	dest.rgb = (src.rgb - dest.rgb)*(src.a*abs(amount)) + dest.rgb;

#endif
#ifdef method_overlay

	if (amount < 0) src = vec4(1) - src;
	src.rgb *= (vec3(1) - src.rgb)*2*dest.rgb - src.rgb;
	dest.rgb = (src.rgb - dest.rgb)*(src.a*abs(amount)) + dest.rgb;

#endif
#ifdef method_hardlight

	if (amount < 0) src = vec4(1) - src;
	vec3 g = vec3(greaterThan(src.rgb, vec3(0.5)));
	src.rgb = g*(vec3(1) - vec3(2)*(vec3(1) - src.rgb)*(vec3(1) - dest.rgb))
	        + (vec3(1) - g)*vec3(2)*src.rgb*dest.rgb;
	dest.rgb = (src.rgb - dest.rgb)*(src.a*abs(amount)) + dest.rgb;

#endif
#ifdef method_alphaover

	dest.a = 1 - amount*src.a;
	if (abs(dest.a) > epsilon)
		dest.rgb *= (1 + amount*src.a)/dest.a;
	else
		dest = vec4(0);

#endif
#ifdef method_add_composite

	float alpha = max(0.f, min(1.f, dest.a + src.a));
	float k = abs(alpha) > 1e-8 ? 1.0/alpha : 0.0;
	dest.a = alpha;
	dest.rgb = k*(dest.rgb * dest.a + src.rgb * src.a);

#endif
#ifdef method_alpha

	vec4 rm = dest;
	rm.a = src.a * dest.a;
	src = rm;
	// copied from alpha_over
	dest.a = 1 - amount*src.a;
	if (abs(dest.a) > epsilon)
		dest.rgb *= (1 + amount*src.a)/dest.a;
	else
		dest = vec4(0);

#endif

	out_color = dest;
}
