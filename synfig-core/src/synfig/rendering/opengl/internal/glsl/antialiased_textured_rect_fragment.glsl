/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/internal/glsl/antialiased_textured_rect_fragment.glsl
**	\brief Antialiased Textured Rect Fragment Shader
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

#define interpolation_#0

uniform sampler2D sampler;
uniform vec2 aascale;

in vec2 texcoord;
in vec2 aacoord;

layout(location = 0) out vec4 out_color;

void main()
{
	#ifdef interpolation_nearest

		out_color = texture(sampler, texcoord);

	#endif
	#ifdef interpolation_linear

		out_color = texture(sampler, texcoord);

	#endif
	#ifdef interpolation_cosine

		const float pi = 3.1415926535897932384626433832795;
		ivec2 s = textureSize(sampler, 0) - ivec2(1);
		ivec2 a0 = clamp(ivec2(floor(texcoord)), ivec2(0), s);
		ivec2 a1 = clamp(a0 + ivec2(1), ivec2(0), s);
		vec2 b1 = 0.5*(vec2(1) - cos(pi*fract(texcoord)));
		vec2 b0 = vec2(1) - b1;
		out_color = b0.x*b0.y*texelFetch(sampler, a0, 0)
				  + b1.x*b0.y*texelFetch(sampler, ivec2(a1.x, a0.y), 0)
				  + b0.x*b1.y*texelFetch(sampler, ivec2(a0.x, a1.y), 0)
				  + b1.x*b1.y*texelFetch(sampler, a1, 0);

	#endif
	#ifdef interpolation_cubic

		const mat4 m = mat4(
			-1,  2, -1,  0,
			 3, -5,  0,  2,
			-3,  4,  1,  0,
			 1, -1,  0,  0 );

		ivec2 s = textureSize(sampler, 0) - ivec2(1);
		ivec2 ac = ivec2(floor(texcoord));
		ivec2 a[4] = ivec2[4](
			clamp(ac + ivec2(-1), ivec2(0), s),
			clamp(ac, ivec2(0), s),
			clamp(ac + ivec2(1), ivec2(0), s),
			clamp(ac + ivec2(2), ivec2(0), s) );

		vec2 b = fract(texcoord);
		vec4 tbx = m*vec4(b.x*b.x*b.x, b.x*b.x, b.x, 1);
		vec4 tby = m*vec4(b.y*b.y*b.y, b.y*b.y, b.y, 1);

		out_color = tby[0]*(
				        tbx[0] * texelFetch(sampler, ivec2(a[0].x, a[0].y), 0)
				      + tbx[1] * texelFetch(sampler, ivec2(a[1].x, a[0].y), 0)
				      + tbx[2] * texelFetch(sampler, ivec2(a[2].x, a[0].y), 0)
				      + tbx[3] * texelFetch(sampler, ivec2(a[3].x, a[0].y), 0) )
				  + tby[1]*(
						tbx[0] * texelFetch(sampler, ivec2(a[0].x, a[1].y), 0)
					  + tbx[1] * texelFetch(sampler, ivec2(a[1].x, a[1].y), 0)
					  + tbx[2] * texelFetch(sampler, ivec2(a[2].x, a[1].y), 0)
					  + tbx[3] * texelFetch(sampler, ivec2(a[3].x, a[1].y), 0) )
				  + tby[2]*(
						tbx[0] * texelFetch(sampler, ivec2(a[0].x, a[2].y), 0)
					  + tbx[1] * texelFetch(sampler, ivec2(a[1].x, a[2].y), 0)
					  + tbx[2] * texelFetch(sampler, ivec2(a[2].x, a[2].y), 0)
					  + tbx[3] * texelFetch(sampler, ivec2(a[3].x, a[2].y), 0) )
				  + tby[3]*(
						tbx[0] * texelFetch(sampler, ivec2(a[0].x, a[3].y), 0)
					  + tbx[1] * texelFetch(sampler, ivec2(a[1].x, a[3].y), 0)
					  + tbx[2] * texelFetch(sampler, ivec2(a[2].x, a[3].y), 0)
					  + tbx[3] * texelFetch(sampler, ivec2(a[3].x, a[3].y), 0) );

	#endif

	vec2 aa = clamp(vec2(0.5) + (vec2(1) - abs(aacoord))*aascale, vec2(0), vec2(1));
	out_color.a *= aa.x*aa.y;
}
