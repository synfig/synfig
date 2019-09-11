/* === S Y N F I G ========================================================= */
/*!    \file
**    \brief PixelFormat and conversions
**
**    $Id$
**
**    \legal
**    Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**    Copyright (c) 2007, 2008 Chris Moore
**    Copyright (c) 2012-2013 Carlos López
**    Copyright (c) 2015 Diego Barrios Romero
**    ......... ... 2018 Ivan Mahonin
**
**    This package is free software; you can redistribute it and/or
**    modify it under the terms of the GNU General Public License as
**    published by the Free Software Foundation; either version 2 of
**    the License, or (at your option) any later version.
**
**    This package is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**    General Public License for more details.
**    \endlegal
*/
/* ========================================================================= */

#include "pixelformat.h"

using namespace synfig;

namespace {
	struct Color2PFParams {
		unsigned char *dst;
		const Color *src;
		PixelFormat pf;
		const Gamma *gamma;
		int width;
		int height;
		int dst_stride_extra;
		int src_stride_extra;

		explicit inline Color2PFParams(
			unsigned char *dst = NULL,
			const Color *src = NULL,
			PixelFormat pf = 0,
			const Gamma *gamma = NULL,
			int width = 0,
			int height = 0,
			int dst_stride_extra = 0,
			int src_stride_extra = 0
		):
			dst(dst),
			src(src),
			pf(pf),
			gamma(gamma),
			width(width),
			height(height),
			dst_stride_extra(dst_stride_extra),
			src_stride_extra(src_stride_extra) { }
	};


	static inline unsigned char*
	color2pf_raw(
		unsigned char *dst,
		const Color &src,
		const Gamma* )
	{
		// just copy raw color data
		*reinterpret_cast<Color*>(dst) = src;
		return dst + sizeof(src);
	}


	template<
		bool bgr,
		bool alpha,
		bool alpha_start >
	static inline unsigned char*
	color2pf_simple(
		unsigned char *dst,
		const Color &src,
		const Gamma* )
	{
		const Color color = src.clamped();

		// put alpha before color channels if need
		if (alpha && alpha_start)
			*dst = (unsigned char)(color.get_a()*ColorReal(255.9)), ++dst;

		// put color channels
		if (bgr) {
			*dst = (unsigned char)(color.get_b()*ColorReal(255.9)), ++dst;
			*dst = (unsigned char)(color.get_g()*ColorReal(255.9)), ++dst;
			*dst = (unsigned char)(color.get_r()*ColorReal(255.9)), ++dst;
		} else {
			*dst = (unsigned char)(color.get_r()*ColorReal(255.9)), ++dst;
			*dst = (unsigned char)(color.get_g()*ColorReal(255.9)), ++dst;
			*dst = (unsigned char)(color.get_b()*ColorReal(255.9)), ++dst;
		}

		// put alpha after color channels if need
		if (alpha && !alpha_start)
			*dst = (unsigned char)(color.get_a()*ColorReal(255.9)), ++dst;

		return dst;
	}


	ColorReal clamp(ColorReal c)
		{ return c > ColorReal(0.0) ? (c < ColorReal(1.0) ? c : ColorReal(1.0)): ColorReal(0.0); }


	template<
		bool with_gamma,
		bool gray,
		bool bgr,
		bool alpha,
		bool alpha_start,
		bool alpha_premult >
	static inline unsigned char*
	color2pf(
		unsigned char *dst,
		const Color &src,
		const Gamma *gamma )
	{
		// get color values
		int ri, gi, bi, ac;
		if (with_gamma) {
			ri = gamma->r_F32_to_U16(clamp(src.get_r()));
			gi = gamma->g_F32_to_U16(clamp(src.get_g()));
			bi = gamma->b_F32_to_U16(clamp(src.get_b()));
			if (alpha)
				ac = gamma->a_F32_to_U8(clamp(src.get_a()));
		} else {
			ri = (int)(clamp(src.get_r()*65535.9f));
			gi = (int)(clamp(src.get_g()*65535.9f));
			bi = (int)(clamp(src.get_b()*65535.9f));
			if (alpha)
				ac = (int)(clamp(src.get_a())*ColorReal(255.9));
		}

		// put alpha before color channels if need
		if (alpha && alpha_start)
			*dst = ac, ++dst;

		// put color channels
		if (alpha && alpha_premult) {
			int ai = ac + 1;
			if (gray) {
				const int yuv_r = (int)(EncodeYUV[0][0]*256.f);
				const int yuv_g = (int)(EncodeYUV[0][1]*256.f);
				const int yuv_b = 256 - yuv_r - yuv_g;
				*dst = (unsigned char)((
						    ((ri*ai) >> 8)*yuv_r
						  + ((gi*ai) >> 8)*yuv_g
						  + ((bi*ai) >> 8)*yuv_b ) >> 16);
				++dst;
			} else
			if (bgr) {
				*dst = (unsigned char)((bi*ai) >> 16), ++dst;
				*dst = (unsigned char)((gi*ai) >> 16), ++dst;
				*dst = (unsigned char)((ri*ai) >> 16), ++dst;
			} else {
				*dst = (unsigned char)((ri*ai) >> 16), ++dst;
				*dst = (unsigned char)((gi*ai) >> 16), ++dst;
				*dst = (unsigned char)((bi*ai) >> 16), ++dst;
			}
		} else {
			if (gray) {
				const int yuv_r = (int)(EncodeYUV[0][0]*256.f);
				const int yuv_g = (int)(EncodeYUV[0][1]*256.f);
				const int yuv_b = 256 - yuv_r - yuv_g;
				*dst = (unsigned char)((ri*yuv_r + gi*yuv_g + bi*yuv_b) >> 16), ++dst;
			} else
			if (bgr) {
				*dst = (unsigned char)(bi >> 8), ++dst;
				*dst = (unsigned char)(gi >> 8), ++dst;
				*dst = (unsigned char)(ri >> 8), ++dst;
			} else {
				*dst = (unsigned char)(ri >> 8), ++dst;
				*dst = (unsigned char)(gi >> 8), ++dst;
				*dst = (unsigned char)(bi >> 8), ++dst;
			}
		}

		// put alpha after color channels if need
		if (alpha && !alpha_start)
			*dst = ac, ++dst;

		return dst;
	}


	template<unsigned char* func(unsigned char*, const Color&, const Gamma*)>
	static unsigned char*
	color2pf_image(Color2PFParams params) {
		while(params.height-- > 0) {
			for(int i = 0; i < params.width; ++i)
				params.dst = func(params.dst, *params.src, params.gamma), ++params.src;
			params.dst += params.dst_stride_extra;
			params.src += params.src_stride_extra;
		}
		return params.dst;
	}


	template<bool with_gamma, bool gray, bool bgr>
	static inline unsigned char*
	color2pf_image_partauto(const Color2PFParams &params) {
		if (!FLAGS(params.pf, PF_A))
			return     color2pf_image< color2pf<with_gamma, gray, bgr, false, false, false> >(params);
		if (FLAGS(params.pf, PF_A_PREMULT)) {
			if (FLAGS(params.pf, PF_A_START))
				return color2pf_image< color2pf<with_gamma, gray, bgr, true,  true,  true>  >(params);
			return     color2pf_image< color2pf<with_gamma, gray, bgr, true,  false, true>  >(params);
		}
		if (FLAGS(params.pf, PF_A_START))
			return     color2pf_image< color2pf<with_gamma, gray, bgr, true,  true,  false> >(params);
		return         color2pf_image< color2pf<with_gamma, gray, bgr, true,  false, false> >(params);
	}


	static inline unsigned char*
	color2pf_image_auto(const Color2PFParams &params) {
		if (FLAGS(params.pf, PF_RAW_COLOR))
			return color2pf_image<color2pf_raw>(params);

		bool with_gamma    = (bool)params.gamma;
		bool gray          = FLAGS(params.pf, PF_GRAY);
		bool bgr           = !gray && FLAGS(params.pf, PF_BGR);
		bool alpha         = FLAGS(params.pf, PF_A);
		bool alpha_premult = alpha && FLAGS(params.pf, PF_A_PREMULT);

		if (!gray && !alpha_premult && !with_gamma) {
			// simple
			bool alpha_start = alpha && FLAGS(params.pf, PF_A_START);
			if (bgr) {
				if (alpha_start) return color2pf_image< color2pf_simple<true,  true,  true>  >(params);
				if (alpha)       return color2pf_image< color2pf_simple<true,  true,  false> >(params);
				return                  color2pf_image< color2pf_simple<true,  false, false> >(params);
			}
			if (alpha_start) return     color2pf_image< color2pf_simple<false, true,  true>  >(params);
			if (alpha)       return     color2pf_image< color2pf_simple<false, true,  false> >(params);
			return                      color2pf_image< color2pf_simple<false, false, false> >(params);
		}

		if (with_gamma) {
			if (gray) return color2pf_image_partauto<true,  true,  false>(params);
			if (bgr)  return color2pf_image_partauto<true,  false, true >(params);
			return           color2pf_image_partauto<true,  false, false>(params);
		}
		if (gray) return     color2pf_image_partauto<false, true,  false>(params);
		if (bgr)  return     color2pf_image_partauto<false, false, true >(params);
		return               color2pf_image_partauto<false, false, false>(params);
	}
} // namespace

namespace {
	struct PF2ColorParams {
		Color *dst;
		const unsigned char *src;
		PixelFormat pf;
		int width;
		int height;
		int dst_stride_extra;
		int src_stride_extra;

		explicit inline PF2ColorParams(
			Color *dst = NULL,
			const unsigned char *src = NULL,
			PixelFormat pf = 0,
			int width = 0,
			int height = 0,
			int dst_stride_extra = 0,
			int src_stride_extra = 0
		):
			dst(dst),
			src(src),
			pf(pf),
			width(width),
			height(height),
			dst_stride_extra(dst_stride_extra),
			src_stride_extra(src_stride_extra) { }
	};


	static inline const unsigned char*
	pf2color_raw(
		Color &dst,
		const unsigned char *src )
	{
		// just copy raw color data
		dst = *reinterpret_cast<const Color*>(src);
		return src + sizeof(dst);
	}


	template<
		bool gray,
		bool bgr,
		bool alpha,
		bool alpha_start,
		bool alpha_premult >
	inline const unsigned char*
	pf2color(
		Color &dst,
		const unsigned char *src )
	{
		const ColorReal k(1.0/255.0);

		if (!alpha) dst.set_a(1.0);

		// read alpha at begin if need
		if (alpha && alpha_start) dst.set_a(k*ColorReal(*src)), ++src;

		// read color channels
		if (gray) {
			dst.set_yuv(k*ColorReal(*src), 0, 0), ++src;
		} else
		if (bgr) {
			dst.set_b(k*ColorReal(*src)), ++src;
			dst.set_g(k*ColorReal(*src)), ++src;
			dst.set_r(k*ColorReal(*src)), ++src;
		} else {
			dst.set_r(k*ColorReal(*src)), ++src;
			dst.set_g(k*ColorReal(*src)), ++src;
			dst.set_b(k*ColorReal(*src)), ++src;
		}

		// read alpha at end if need
		if (alpha && !alpha_start) dst.set_a(k*ColorReal(*src)), ++src;

		// demult alpha
		if (alpha && alpha_premult) dst = dst.demult_alpha();

		return src;
	}


	template<const unsigned char* func(Color&, const unsigned char*)>
	static const unsigned char*
	pf2color_image(PF2ColorParams params) {
		while(params.height-- > 0) {
			for(int width = params.width; width > 0; --width)
				params.src = func(*params.dst, params.src), ++params.dst;
			params.dst += params.dst_stride_extra;
			params.src += params.src_stride_extra;
		}
		return params.src;
	}


	template<bool gray, bool bgr>
	static inline const unsigned char*
	pf2color_image_partauto(const PF2ColorParams &params) {
		if (!FLAGS(params.pf, PF_A))
			return     pf2color_image< pf2color<gray, bgr, false, false, false> >(params);
		if (FLAGS(params.pf, PF_A_PREMULT)) {
			if (FLAGS(params.pf, PF_A_START))
				return pf2color_image< pf2color<gray, bgr, true,  true,  true>  >(params);
			return     pf2color_image< pf2color<gray, bgr, true,  false, true>  >(params);
		}
		if (FLAGS(params.pf, PF_A_START))
			return     pf2color_image< pf2color<gray, bgr, true,  true,  false> >(params);
		return         pf2color_image< pf2color<gray, bgr, true,  false, false> >(params);
	}

	static inline const unsigned char*
	pf2color_image_auto(const PF2ColorParams &params) {
		if (FLAGS(params.pf, PF_RAW_COLOR))
			return pf2color_image<pf2color_raw>(params);
		if (FLAGS(params.pf, PF_GRAY))
			return pf2color_image_partauto<true,  false>(params);
		if (FLAGS(params.pf, PF_BGR))
			return pf2color_image_partauto<false, true >(params);
		return     pf2color_image_partauto<false, false>(params);
	}
};


//! Returns the size of bytes of pixel in given PixelFormat
size_t
synfig::pixel_size(PixelFormat x)
{
    if (FLAGS(x, PF_RAW_COLOR))
    	return sizeof(Color);
    int chan = FLAGS(x, PF_GRAY) ? 1 : 3;
    if (FLAGS(x, PF_A)) ++chan;
    return chan;
}


unsigned char*
synfig::color_to_pixelformat(
	unsigned char *dst,
	const Color *src,
	PixelFormat pf,
	const Gamma *gamma,
	int width,
	int height,
	int dst_stride,
	int src_stride )
{
	assert(src_stride % sizeof(Color) == 0);
	return color2pf_image_auto(Color2PFParams(
		dst, src, pf, gamma, width, height,
		dst_stride ? dst_stride - width*pixel_size(pf) : 0,
		src_stride ? src_stride/sizeof(Color) - width  : 0 ));
}


const unsigned char*
synfig::pixelformat_to_color(
	Color *dst,
	const unsigned char *src,
	PixelFormat pf,
	int width,
	int height,
	int dst_stride,
	int src_stride )
{
	assert(dst_stride % sizeof(Color) == 0);
	return pf2color_image_auto(PF2ColorParams(
		dst, src, pf, width, height,
		dst_stride ? dst_stride/sizeof(Color) - width  : 0,
		src_stride ? src_stride - width*pixel_size(pf) : 0 ));
	assert(src_stride % sizeof(Color) == 0);
}
