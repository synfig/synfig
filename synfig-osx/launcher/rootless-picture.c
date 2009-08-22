/*
 * Support for RENDER extension with rootless
 */
/*
 * Copyright (c) 2002 Torrey T. Lyons. All Rights Reserved.
 * Copyright (c) 2002 Apple Computer, Inc. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name(s) of the above copyright
 * holders shall not be used in advertising or otherwise to promote the sale,
 * use or other dealings in this Software without prior written authorization.
 */
/* This file is largely based on fbcompose.c and fbpict.c, which contain
 * the following copyright:
 *
 * Copyright © 2000 Keith Packard, member of The XFree86 Project, Inc.
 */
 /* $XFree86: xc/programs/Xserver/hw/darwin/quartz/aquaPicture.c,v 1.3 2002/09/28 00:00:03 torrey Exp $ */

#define DEFAULT_LOG_FORMATS 0

#ifdef RENDER

#include "fb.h"
#include "picturestr.h"
#include "mipict.h"
#include "fbpict.h"
#include "rootless.h"

# define mod(a,b)	((b) == 1 ? 0 : (a) >= 0 ? (a) % (b) : (b) - (-a) % (b))


// Replacement for fbStore_x8r8g8b8 that sets the alpha channel
void
RootlessStore_x8r8g8b8 (FbCompositeOperand *op, CARD32 value)
{
    FbBits  *line = op->u.drawable.line; CARD32 offset = op->u.drawable.offset;
    ((CARD32 *)line)[offset >> 5] = (value & 0xffffff) | 0xff000000;
}


// Defined in fbcompose.c
extern FbCombineFunc fbCombineFuncU[];
extern FbCombineFunc fbCombineFuncC[];

void
RootlessCompositeGeneral(
    CARD8               op,
    PicturePtr          pSrc,
    PicturePtr          pMask,
    PicturePtr          pDst,
    INT16               xSrc,
    INT16               ySrc,
    INT16               xMask,
    INT16               yMask,
    INT16               xDst,
    INT16               yDst,
    CARD16              width,
    CARD16              height)
{
    FbCompositeOperand	src[4],msk[4],dst[4],*pmsk;
    FbCompositeOperand	*srcPict, *srcAlpha;
    FbCompositeOperand	*dstPict, *dstAlpha;
    FbCompositeOperand	*mskPict = 0, *mskAlpha = 0;
    FbCombineFunc	f;
    int			w;

    if (!fbBuildCompositeOperand (pSrc, src, xSrc, ySrc, TRUE, TRUE))
	return;
    if (!fbBuildCompositeOperand (pDst, dst, xDst, yDst, FALSE, TRUE))
	return;

    // Use Rootless operands for on screen picture formats
    if (pDst->format == PICT_x8r8g8b8) {
        dst[0].store = RootlessStore_x8r8g8b8;
    }

    if (pSrc->alphaMap)
    {
	srcPict = &src[1];
	srcAlpha = &src[2];
    }
    else
    {
	srcPict = &src[0];
	srcAlpha = 0;
    }
    if (pDst->alphaMap)
    {
	dstPict = &dst[1];
	dstAlpha = &dst[2];
    }
    else
    {
	dstPict = &dst[0];
	dstAlpha = 0;
    }
    f = fbCombineFuncU[op];
    if (pMask)
    {
	if (!fbBuildCompositeOperand (pMask, msk, xMask, yMask, TRUE, TRUE))
	    return;
	pmsk = msk;
	if (pMask->componentAlpha)
	    f = fbCombineFuncC[op];
	if (pMask->alphaMap)
	{
	    mskPict = &msk[1];
	    mskAlpha = &msk[2];
	}
	else
	{
	    mskPict = &msk[0];
	    mskAlpha = 0;
	}
    }
    else
	pmsk = 0;
    while (height--)
    {
	w = width;
	
	while (w--)
	{
	    (*f) (src, pmsk, dst);
	    (*src->over) (src);
	    (*dst->over) (dst);
	    if (pmsk)
		(*pmsk->over) (pmsk);
	}
	(*src->down) (src);
	(*dst->down) (dst);
	if (pmsk)
	    (*pmsk->down) (pmsk);
    }
}

static int rootless_log_pict_formats = DEFAULT_LOG_FORMATS;

static const char *op_name (int op)
{
    static const char *ops[] = {
	"Clear", "Src", "Dst", "Over", "OverReverse", "In", "InReverse",
	"Out", "OutReverse", "Atop", "AtopReverse", "Xor", "Add",
	"Saturate", "Maximum",

	"DisjointClear", "DisjointSrc", "DisjointDst", "DisjointOver",
	"DisjointOverReverse", "DisjointIn", "DisjointInReverse",
	"DisjointOut", "DisjointOutReverse", "DisjointAtop",
	"DisjointAtopReverse", "DisjointXor", "DisjointMaximum",

	"ConjointClear", "ConjointSrc", "ConjointDst", "ConjointOver",
	"ConjointOverReverse", "ConjointIn", "ConjointInReverse",
	"ConjointOut", "ConjointOutReverse", "ConjointAtop",
	"ConjointAtopReverse", "ConjointXor", "ConjointMaximum",
    };

    if (op >= 0 && op < (int) (sizeof (ops) / sizeof (ops[0])))
	return ops[op];
    else
	return "Unknown";
}

static const char *type_name (int type)
{
    switch (type)
    {
    case PICT_TYPE_OTHER:
	return "Other";
    case PICT_TYPE_A:
	return "A";
    case PICT_TYPE_ARGB:
	return "ARGB";
    case PICT_TYPE_ABGR:
	return "ABGR";
    case PICT_TYPE_COLOR:
	return "Color";
    case PICT_TYPE_GRAY:
	return "Gray";
    default:
	return "Unknown";
    }
}

static void log_format (int op, unsigned int src,
			unsigned int dst, unsigned int mask)
{
    struct op {
	int op;
	unsigned int src, dst, mask;
    };

    static struct op *ops;
    static int n_ops, allocated_ops;

    int i;

    for (i = 0; i < n_ops; i++)
    {
	if (ops[i].op == op && ops[i].src == src
	    && ops[i].dst == dst && ops[i].mask == mask)
	{
	    return;
	}
    }

    if (n_ops == allocated_ops)
    {
	allocated_ops *= 2;
	ops = realloc (ops, allocated_ops * sizeof (struct op));
    }

    ops[n_ops].op = op;
    ops[n_ops].src = src;
    ops[n_ops].dst = dst;
    ops[n_ops].mask = mask;
    n_ops++;

    fprintf (stderr,
	     "op: %s src (%dbpp %s %04x) dst (%dbpp %s %04x) mask (%dbpp %s %04x)\n",
	     op_name (op), PICT_FORMAT_BPP (src),
	     type_name (PICT_FORMAT_TYPE (src)),
	     src & 0xffff, PICT_FORMAT_BPP (dst),
	     type_name (PICT_FORMAT_TYPE (dst)),
	     dst & 0xffff, PICT_FORMAT_BPP (mask),
	     type_name (PICT_FORMAT_TYPE (mask)),
	     mask & 0xffff);
}

void
RootlessComposite(
    CARD8           op,
    PicturePtr      pSrc,
    PicturePtr      pMask,
    PicturePtr      pDst,
    INT16           xSrc,
    INT16           ySrc,
    INT16           xMask,
    INT16           yMask,
    INT16           xDst,
    INT16           yDst,
    CARD16          width,
    CARD16          height)
{
    RegionRec	    region;
    int		    n;
    BoxPtr	    pbox;
    CompositeFunc   func;
    Bool	    srcRepeat = pSrc->repeat;
    Bool	    maskRepeat = FALSE;
    Bool	    srcAlphaMap = pSrc->alphaMap != 0;
    Bool	    maskAlphaMap = FALSE;
    Bool	    dstAlphaMap = pDst->alphaMap != 0;
    int		    x_msk, y_msk, x_src, y_src, x_dst, y_dst;
    int		    w, h, w_this, h_this;
    int		    dstDepth = pDst->pDrawable->depth;

    xDst += pDst->pDrawable->x;
    yDst += pDst->pDrawable->y;
    xSrc += pSrc->pDrawable->x;
    ySrc += pSrc->pDrawable->y;
    if (pMask)
    {
	xMask += pMask->pDrawable->x;
	yMask += pMask->pDrawable->y;
	maskRepeat = pMask->repeat;
	maskAlphaMap = pMask->alphaMap != 0;
    }

    if (rootless_log_pict_formats)
    {
	log_format (op, pSrc->format, pDst->format,
		    pMask != 0 ? pMask->format : 0);
    }

    if (!miComputeCompositeRegion (&region,
				   pSrc,
				   pMask,
				   pDst,
				   xSrc,
				   ySrc,
				   xMask,
				   yMask,
				   xDst,
				   yDst,
				   width,
				   height))
	return;

    if (pDst->pDrawable->type == DRAWABLE_WINDOW
	&& pDst->pDrawable->depth == 24
	&& pDst->pDrawable->bitsPerPixel == 32)
    {
	/* fbpict code sets bits above depth to zero. We don't want that! */

	pDst->pDrawable->depth = 32;
    }

    func = RootlessCompositeGeneral;

    if (!maskAlphaMap && !srcAlphaMap && !dstAlphaMap)
    switch (op) {
    case PictOpOver:
	if (pMask)
	{
	    if (srcRepeat && 
		pSrc->pDrawable->width == 1 &&
		pSrc->pDrawable->height == 1)
	    {
		srcRepeat = FALSE;
		if (PICT_FORMAT_COLOR(pSrc->format)) {
		    switch (pMask->format) {
		    case PICT_a8:
			switch (pDst->format) {
			case PICT_r5g6b5:
			case PICT_b5g6r5:
			    func = fbCompositeSolidMask_nx8x0565;
			    break;
			case PICT_r8g8b8:
			case PICT_b8g8r8:
			    func = fbCompositeSolidMask_nx8x0888;
			    break;
			case PICT_a8r8g8b8:
			case PICT_x8r8g8b8:
			case PICT_a8b8g8r8:
			case PICT_x8b8g8r8:
			    func = fbCompositeSolidMask_nx8x8888;
			    break;
			}
			break;
		    case PICT_a8r8g8b8:
			if (pMask->componentAlpha) {
			    switch (pDst->format) {
			    case PICT_a8r8g8b8:
			    case PICT_x8r8g8b8:
				func = fbCompositeSolidMask_nx8888x8888C;
				break;
			    case PICT_r5g6b5:
				func = fbCompositeSolidMask_nx8888x0565C;
				break;
			    }
			}
			break;
		    case PICT_a8b8g8r8:
			if (pMask->componentAlpha) {
			    switch (pDst->format) {
			    case PICT_a8b8g8r8:
			    case PICT_x8b8g8r8:
				func = fbCompositeSolidMask_nx8888x8888C;
				break;
			    case PICT_b5g6r5:
				func = fbCompositeSolidMask_nx8888x0565C;
				break;
			    }
			}
			break;
		    case PICT_a1:
			switch (pDst->format) {
			case PICT_r5g6b5:
			case PICT_b5g6r5:
			case PICT_r8g8b8:
			case PICT_b8g8r8:
			case PICT_a8r8g8b8:
			case PICT_x8r8g8b8:
			case PICT_a8b8g8r8:
			case PICT_x8b8g8r8:
			    func = fbCompositeSolidMask_nx1xn;
			    break;
			}
		    }
		}
	    }
	}
	else
	{
	    switch (pSrc->format) {
	    case PICT_a8r8g8b8:
	    case PICT_x8r8g8b8:
		switch (pDst->format) {
		case PICT_a8r8g8b8:
		case PICT_x8r8g8b8:
		    func = fbCompositeSrc_8888x8888;
		    break;
		case PICT_r8g8b8:
		    func = fbCompositeSrc_8888x0888;
		    break;
		case PICT_r5g6b5:
		    func = fbCompositeSrc_8888x0565;
		    break;
		}
		break;
	    case PICT_a8b8g8r8:
	    case PICT_x8b8g8r8:
		switch (pDst->format) {
		case PICT_a8b8g8r8:
		case PICT_x8b8g8r8:
		    func = fbCompositeSrc_8888x8888;
		    break;
		case PICT_b8g8r8:
		    func = fbCompositeSrc_8888x0888;
		    break;
		case PICT_b5g6r5:
		    func = fbCompositeSrc_8888x0565;
		    break;
		}
		break;
	    case PICT_r5g6b5:
		switch (pDst->format) {
		case PICT_r5g6b5:
		    func = fbCompositeSrc_0565x0565;
		    break;
		}
		break;
	    case PICT_b5g6r5:
		switch (pDst->format) {
		case PICT_b5g6r5:
		    func = fbCompositeSrc_0565x0565;
		    break;
		}
		break;
	    }
	}
	break;
    case PictOpAdd:
	if (pMask == 0)
	{
	    switch (pSrc->format) {
	    case PICT_a8r8g8b8:
		switch (pDst->format) {
		case PICT_a8r8g8b8:
		    func = fbCompositeSrcAdd_8888x8888;
		    break;
		}
		break;
	    case PICT_a8b8g8r8:
		switch (pDst->format) {
		case PICT_a8b8g8r8:
		    func = fbCompositeSrcAdd_8888x8888;
		    break;
		}
		break;
	    case PICT_a8:
		switch (pDst->format) {
		case PICT_a8:
		    func = fbCompositeSrcAdd_8000x8000;
		    break;
		}
		break;
	    case PICT_a1:
		switch (pDst->format) {
		case PICT_a1:
		    func = fbCompositeSrcAdd_1000x1000;
		    break;
		}
		break;
	    }
	}
	break;
    }

    n = REGION_NUM_RECTS (&region);
    pbox = REGION_RECTS (&region);
    while (n--)
    {
	h = pbox->y2 - pbox->y1;
	y_src = pbox->y1 - yDst + ySrc;
	y_msk = pbox->y1 - yDst + yMask;
	y_dst = pbox->y1;
	while (h)
	{
	    h_this = h;
	    w = pbox->x2 - pbox->x1;
	    x_src = pbox->x1 - xDst + xSrc;
	    x_msk = pbox->x1 - xDst + xMask;
	    x_dst = pbox->x1;
	    if (maskRepeat)
	    {
		y_msk = mod (y_msk, pMask->pDrawable->height);
		if (h_this > pMask->pDrawable->height - y_msk)
		    h_this = pMask->pDrawable->height - y_msk;
	    }
	    if (srcRepeat)
	    {
		y_src = mod (y_src, pSrc->pDrawable->height);
		if (h_this > pSrc->pDrawable->height - y_src)
		    h_this = pSrc->pDrawable->height - y_src;
	    }
	    while (w)
	    {
		w_this = w;
		if (maskRepeat)
		{
		    x_msk = mod (x_msk, pMask->pDrawable->width);
		    if (w_this > pMask->pDrawable->width - x_msk)
			w_this = pMask->pDrawable->width - x_msk;
		}
		if (srcRepeat)
		{
		    x_src = mod (x_src, pSrc->pDrawable->width);
		    if (w_this > pSrc->pDrawable->width - x_src)
			w_this = pSrc->pDrawable->width - x_src;
		}
		(*func) (op, pSrc, pMask, pDst,
			 x_src, y_src, x_msk, y_msk, x_dst, y_dst,
			 w_this, h_this);
		w -= w_this;
		x_src += w_this;
		x_msk += w_this;
		x_dst += w_this;
	    }
	    h -= h_this;
	    y_src += h_this;
	    y_msk += h_this;
	    y_dst += h_this;
	}
	pbox++;
    }
    REGION_UNINIT (pDst->pDrawable->pScreen, &region);

    pDst->pDrawable->depth = dstDepth;
}

#endif /* RENDER */
