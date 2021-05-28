/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/opengl/internal/glsl/.glsl_to_cpp_defs.hpp
**	\brief Fake C++ header which helps to edit .glsl files
**         in IDE like .cpp files
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


// keywords

#define uniform
#define in
#define out
#define inout
#define layout(...)
#define noperspective
#define flat
#define smooth


// predeclarations

struct sampler2D { };
struct mat2 { };
struct mat3 { };
struct mat4 { };


namespace __defs {
	template<typename T> struct vec2;
	template<typename T> struct vec3;
	template<typename T> struct vec4;

	template<typename T>
	struct vec2 {
		T x, y;
		vec2<T> xy, yx;
		T r, g;
		vec2<T> rg, gr;
		T s, t;
		vec2<T> st, ts;
		T& operator[] (int);
		const T& operator[] (int) const;
		bool operator == (vec2);
		bool operator != (vec2);
		vec2& operator += (vec2);
		vec2& operator -= (vec2);
		vec2& operator *= (vec2);
		vec2& operator /= (vec2);
		vec2 operator + (vec2);
		vec2 operator - (vec2);
		vec2 operator * (vec2);
		vec2 operator / (vec2);
		vec2 operator * (T);
		vec2 operator / (T);
		explicit vec2(T);
		explicit vec2(T, T);
		explicit vec2(vec3<T>);
		explicit vec2(vec4<T>);
	};

	template<typename T>
	struct vec3 {
		T x, y, z;
		vec2<T> xy, xz, yx, yz, zx, zy;
		vec3<T> xyz, xzy, yxz, yzx, zxy, zyx;
		T r, g, b;
		vec2<T> rg, rb, gr, gb, br, bg;
		vec3<T> rgb, rbg, grb, gbr, brg, bgr;
		T s, t, p;
		vec2<T> st, sp, ts, tp, ps, pt;
		vec3<T> stp, spt, tsp, tps, pst, pts;
		T& operator[] (int);
		const T& operator[] (int) const;
		bool operator == (vec3);
		bool operator != (vec3);
		vec3& operator += (vec3);
		vec3& operator -= (vec3);
		vec3& operator *= (vec3);
		vec3& operator /= (vec3);
		vec3 operator + (vec3);
		vec3 operator - (vec3);
		vec3 operator * (vec3);
		vec3 operator / (vec3);
		vec3 operator * (T);
		vec3 operator / (T);
		explicit vec3(T);
		explicit vec3(T, T);
		explicit vec3(T, T, T);
		explicit vec3(vec2<T>);
		explicit vec3(vec2<T>, T);
		explicit vec3(vec4<T>);
	};

	template<typename T>
	struct vec4 {
		T x, y, z, w;
		vec2<T> xy, xz, xw, yx, yz, yw, zx, zy, zw, wx, wy, wz;
		vec3<T> xyz, xzy, xwy, yxz, yzx, ywx, zxy, zyx, zwx, wxy, wyx, wzx,
		        xyw, xzw, xwz, yxw, yzw, ywz, zxw, zyw, zwy, wxz, wyz, wzy;
		vec4<T> xyzw, xzyw, xwyz, yxzw, yzxw, ywxz, zxyw, zyxw, zwxy, wxyz, wyxz, wzxy,
		        xywz, xzwy, xwzy, yxwz, yzwx, ywzx, zxwy, zywx, zwyx, wxzy, wyzx, wzyx;
		T r, g, b, a;
		vec2<T> rg, rb, ra, gr, gb, ga, br, bg, ba, ar, ag, ab;
		vec3<T> rgb, rbg, rag, grb, gbr, gar, brg, bgr, bar, arg, agr, abr,
		        rga, rba, rab, gra, gba, gab, bra, bga, bag, arb, agb, abg;
		vec4<T> rgba, rbga, ragb, grba, gbra, garb, brga, bgra, barg, argb, agrb, abrg,
		        rgab, rbag, rabg, grab, gbar, gabr, brag, bgar, bagr, arbg, agbr, abgr;
		T s, t, p, q;
		vec2<T> st, sp, sq, ts, tp, tq, ps, pt, pq, qs, qt, qp;
		vec3<T> stp, spt, sqt, tsp, tps, tqs, pst, pts, pqs, qst, qts, qps,
		        stq, spq, sqp, tsq, tpq, tqp, psq, ptq, pqt, qsp, qtp, qpt;
		vec4<T> stpq, sptq, sqtp, tspq, tpsq, tqsp, pstq, ptsq, pqst, qstp, qtsp, qpst,
		        stqp, spqt, sqpt, tsqp, tpqs, tqps, psqt, ptqs, pqts, qspt, qtps, qpts;
		T& operator[] (int);
		const T& operator[] (int) const;
		bool operator == (vec4);
		bool operator != (vec4);
		vec4& operator += (vec4);
		vec4& operator -= (vec4);
		vec4& operator *= (vec4);
		vec4& operator /= (vec4);
		vec4 operator + (vec4);
		vec4 operator - (vec4);
		vec4 operator * (vec4);
		vec4 operator / (vec4);
		vec4 operator * (T);
		vec4 operator / (T);
		explicit vec4(T);
		explicit vec4(T, T);
		explicit vec4(T, T, T);
		explicit vec4(T, T, T, T);
		explicit vec4(vec2<T>);
		explicit vec4(vec2, T);
		explicit vec4(vec2, T, T);
		explicit vec4(vec3<T>);
		explicit vec4(vec3, T);
	};
}

#define __defs_vectypedefs(T, p)  \
	typedef __defs::vec2<T> p ## vec2; \
	typedef __defs::vec3<T> p ## vec3; \
	typedef __defs::vec4<T> p ## vec4;

#define __defs_func_1(f, t) \
	t f(t);

#define __defs_func_2(f, t) \
	t f(t, t);

#define __defs_func_3(f, t) \
	t f(t, t, t);

#define __defs_func(args, f, t) \
	__defs_func_ ## args(f, t)

#define __defs_vecfunc(args, f, p) \
	__defs_func(args, f, p ## vec2) \
	__defs_func(args, f, p ## vec3) \
	__defs_func(args, f, p ## vec4)

#define __defs_vecsfunc(args, f, t, p) \
	__defs_func(args, f, t) \
	__defs_vecfunc(args, f, p)

#define __defs_vecoper(sign, t, p) \
	p ## vec2 operator sign(t, p ## vec2); \
	p ## vec3 operator sign(t, p ## vec3); \
	p ## vec4 operator sign(t, p ## vec4);


// built-in types

__defs_vectypedefs(float,)
__defs_vectypedefs(int,i)
__defs_vectypedefs(bool,b)


// built-in vector operators

__defs_vecoper(*, float,)
__defs_vecoper(*, int, i)
__defs_vecoper(*, bool, b)


// built-in functions

__defs_vecsfunc(1, floor, float,)
__defs_vecsfunc(1, ceil, float,)
__defs_vecsfunc(1, fract, float,)
__defs_vecsfunc(1, sin, float,)
__defs_vecsfunc(1, cos, float,)
__defs_vecsfunc(1, abs, float,)
__defs_vecsfunc(1, abs, int, i)
__defs_vecsfunc(3, clamp, float,)
__defs_vecsfunc(3, clamp, int, i)
__defs_vecfunc(2, dot,)

vec4 texture(sampler2D, vec2);
vec4 texture(sampler2D, vec2, float);
ivec2 textureSize(sampler2D, int);
vec4 texelFetch(sampler2D, ivec2, int);

// built-in variables

const vec4 gl_FragCoord;
const vec4 gl_Position;

