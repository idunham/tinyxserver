/*
 * $XFree86: xc/programs/Xserver/fb/fbcompose.c,v 1.13 2001/10/28 03:33:08 tsi Exp $
 *
 * Copyright © 2000 Keith Packard, member of The XFree86 Project, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "fb.h"
#include "../render/picturestr.h"
#include "../render/mipict.h"
#include "fbpict.h"

/*
 * General purpose compositing code optimized for minimal memory
 * references
 *
 * All work is done on canonical ARGB values, functions for fetching
 * and storing these exist for each format.
 */

/*
 * Combine src and mask using IN
 */

CARD32
fbCombineMaskU (FbCompositeOperand   *src,
		FbCompositeOperand   *msk)
{
    CARD32  x;
    CARD16  a;
    CARD16  t;
    CARD32  m,n,o,p;

    if (!msk)
	return (*src->fetch) (src);
    
    a = (*msk->fetch) (msk) >> 24;
    if (!a)
	return 0;
    
    x = (*src->fetch) (src);
    if (a == 0xff)
	return x;
    
    m = FbInU(x,0,a,t);
    n = FbInU(x,8,a,t);
    o = FbInU(x,16,a,t);
    p = FbInU(x,24,a,t);
    return m|n|o|p;
}

FbCompSrc
fbCombineMaskC (FbCompositeOperand   *src,
		FbCompositeOperand   *msk)
{
    FbCompSrc	s;
    CARD32	x;
    CARD32	a;
    CARD16	xa;
    CARD16	t;
    CARD32	m,n,o,p;

    if (!msk)
    {
	x = (*src->fetch) (src);
	s.value = x;
	x = x >> 24;
	x |= x << 8;
	x |= x << 16;
	s.alpha = x;
	return s;
    }
    
    a = (*msk->fetcha) (msk);
    if (!a)
    {
	s.value = 0;
	s.alpha = 0;
	return s;
    }
    
    x = (*src->fetch) (src);
    if (a == 0xffffffff)
    {
	s.value = x;
	x = x >> 24;
	x |= x << 8;
	x |= x << 16;
	s.alpha = x;
	return s;
    }
    
    m = FbInC(x,0,a,t);
    n = FbInC(x,8,a,t);
    o = FbInC(x,16,a,t);
    p = FbInC(x,24,a,t);
    s.value = m|n|o|p;
    xa = x >> 24;
    m = FbInU(a,0,xa,t);
    n = FbInU(a,8,xa,t);
    o = FbInU(a,16,xa,t);
    p = FbInU(a,24,xa,t);
    s.alpha = m|n|o|p;
    return s;
}

CARD32
fbCombineMaskValueC (FbCompositeOperand   *src,
		     FbCompositeOperand   *msk)
{
    CARD32	x;
    CARD32	a;
    CARD16	t;
    CARD32	m,n,o,p;

    if (!msk)
    {
	return (*src->fetch) (src);
    }
    
    a = (*msk->fetcha) (msk);
    if (!a)
	return a;
    
    x = (*src->fetch) (src);
    if (a == 0xffffffff)
	return x;
    
    m = FbInC(x,0,a,t);
    n = FbInC(x,8,a,t);
    o = FbInC(x,16,a,t);
    p = FbInC(x,24,a,t);
    return m|n|o|p;
}

/*
 * Combine src and mask using IN, generating only the alpha component
 */
CARD32
fbCombineMaskAlphaU (FbCompositeOperand   *src,
		     FbCompositeOperand   *msk)
{
    CARD32  x;
    CARD16  a;
    CARD16  t;

    if (!msk)
	return (*src->fetch) (src);
    
    a = (*msk->fetch) (msk) >> 24;
    if (!a)
	return 0;
    
    x = (*src->fetch) (src);
    if (a == 0xff)
	return x;
    
    return FbInU(x,24,a,t);
}

CARD32
fbCombineMaskAlphaC (FbCompositeOperand   *src,
		     FbCompositeOperand   *msk)
{
    CARD32	x;
    CARD32	a;
    CARD16	t;
    CARD32	m,n,o,p;

    if (!msk)
	return (*src->fetch) (src);
    
    a = (*msk->fetcha) (msk);
    if (!a)
	return 0;
    
    x = (*src->fetcha) (src);
    if (a == 0xffffffff)
	return x;
    
    m = FbInC(x,0,a,t);
    n = FbInC(x,8,a,t);
    o = FbInC(x,16,a,t);
    p = FbInC(x,24,a,t);
    return m|n|o|p;
}

/*
 * All of the composing functions
 */
void
fbCombineClear (FbCompositeOperand   *src,
		FbCompositeOperand   *msk,
		FbCompositeOperand   *dst)
{
    (*dst->store) (dst, 0);
}

void
fbCombineSrcU (FbCompositeOperand    *src,
	       FbCompositeOperand    *msk,
	       FbCompositeOperand    *dst)
{
    (*dst->store) (dst, fbCombineMaskU (src, msk));
}

void
fbCombineSrcC (FbCompositeOperand    *src,
	       FbCompositeOperand    *msk,
	       FbCompositeOperand    *dst)
{
    (*dst->store) (dst, fbCombineMaskValueC (src, msk));
}

void
fbCombineDst (FbCompositeOperand    *src,
	      FbCompositeOperand    *msk,
	      FbCompositeOperand    *dst)
{
    /* noop */
}

void
fbCombineOverU (FbCompositeOperand   *src,
		FbCompositeOperand   *msk,
		FbCompositeOperand   *dst)
{
    CARD32  s, d;
    CARD16  a;
    CARD16  t;
    CARD32  m,n,o,p;

    s = fbCombineMaskU (src, msk);
    a = ~s >> 24;
    if (a != 0xff)
    {
	if (a)
	{
	    d = (*dst->fetch) (dst);
	    m = FbOverU(s,d,0,a,t);
	    n = FbOverU(s,d,8,a,t);
	    o = FbOverU(s,d,16,a,t);
	    p = FbOverU(s,d,24,a,t);
	    s = m|n|o|p;
	}
	(*dst->store) (dst, s);
    }
}

void
fbCombineOverC (FbCompositeOperand   *src,
		FbCompositeOperand   *msk,
		FbCompositeOperand   *dst)
{
    FbCompSrc	cs;
    CARD32  s, d;
    CARD32  a;
    CARD16  t;
    CARD32  m,n,o,p;

    cs = fbCombineMaskC (src, msk);
    s = cs.value;
    a = ~cs.alpha;
    if (a != 0xffffffff)
    {
	if (a)
	{
	    d = (*dst->fetch) (dst);
	    m = FbOverC(s,d,0,a,t);
	    n = FbOverC(s,d,8,a,t);
	    o = FbOverC(s,d,16,a,t);
	    p = FbOverC(s,d,24,a,t);
	    s = m|n|o|p;
	}
	(*dst->store) (dst, s);
    }
}

void
fbCombineOverReverseU (FbCompositeOperand    *src,
		       FbCompositeOperand    *msk,
		       FbCompositeOperand    *dst)
{
    CARD32  s, d;
    CARD16  a;
    CARD16  t;
    CARD32  m,n,o,p;

    d = (*dst->fetch) (dst);
    a = ~d >> 24;
    if (a)
    {
	s = fbCombineMaskU (src, msk);
	if (a != 0xff)
	{
	    m = FbOverU(d,s,0,a,t);
	    n = FbOverU(d,s,8,a,t);
	    o = FbOverU(d,s,16,a,t);
	    p = FbOverU(d,s,24,a,t);
	    s = m|n|o|p;
	}
	(*dst->store) (dst, s);
    }
}

void
fbCombineOverReverseC (FbCompositeOperand    *src,
		       FbCompositeOperand    *msk,
		       FbCompositeOperand    *dst)
{
    CARD32  s, d;
    CARD32  a;
    CARD16  t;
    CARD32  m,n,o,p;

    d = (*dst->fetch) (dst);
    a = ~d >> 24;
    if (a)
    {
	s = fbCombineMaskValueC (src, msk);
	if (a != 0xff)
	{
	    m = FbOverU(d,s,0,a,t);
	    n = FbOverU(d,s,8,a,t);
	    o = FbOverU(d,s,16,a,t);
	    p = FbOverU(d,s,24,a,t);
	    s = m|n|o|p;
	}
	(*dst->store) (dst, s);
    }
}

void
fbCombineInU (FbCompositeOperand	    *src,
	      FbCompositeOperand	    *msk,
	      FbCompositeOperand	    *dst)
{
    CARD32  s, d;
    CARD16  a;
    CARD16  t;
    CARD32  m,n,o,p;

    d = (*dst->fetch) (dst);
    a = d >> 24;
    s = 0;
    if (a)
    {
	s = fbCombineMaskU (src, msk);
	if (a != 0xff)
	{
	    m = FbInU(s,0,a,t);
	    n = FbInU(s,8,a,t);
	    o = FbInU(s,16,a,t);
	    p = FbInU(s,24,a,t);
	    s = m|n|o|p;
	}
    }
    (*dst->store) (dst, s);
}

void
fbCombineInC (FbCompositeOperand	    *src,
	      FbCompositeOperand	    *msk,
	      FbCompositeOperand	    *dst)
{
    CARD32  s, d;
    CARD16  a;
    CARD16  t;
    CARD32  m,n,o,p;

    d = (*dst->fetch) (dst);
    a = d >> 24;
    s = 0;
    if (a)
    {
	s = fbCombineMaskValueC (src, msk);
	if (a != 0xff)
	{
	    m = FbInU(s,0,a,t);
	    n = FbInU(s,8,a,t);
	    o = FbInU(s,16,a,t);
	    p = FbInU(s,24,a,t);
	    s = m|n|o|p;
	}
    }
    (*dst->store) (dst, s);
}

void
fbCombineInReverseU (FbCompositeOperand  *src,
		     FbCompositeOperand  *msk,
		     FbCompositeOperand  *dst)
{
    CARD32  s, d;
    CARD16  a;
    CARD16  t;
    CARD32  m,n,o,p;

    s = fbCombineMaskAlphaU (src, msk);
    a = s >> 24;
    if (a != 0xff)
    {
	d = 0;
	if (a)
	{
	    d = (*dst->fetch) (dst);
	    m = FbInU(d,0,a,t);
	    n = FbInU(d,8,a,t);
	    o = FbInU(d,16,a,t);
	    p = FbInU(d,24,a,t);
	    d = m|n|o|p;
	}
	(*dst->store) (dst, d);
    }
}

void
fbCombineInReverseC (FbCompositeOperand  *src,
		     FbCompositeOperand  *msk,
		     FbCompositeOperand  *dst)
{
    CARD32  s, d;
    CARD32  a;
    CARD16  t;
    CARD32  m,n,o,p;

    s = fbCombineMaskAlphaC (src, msk);
    a = s;
    if (a != 0xffffffff)
    {
	d = 0;
	if (a)
	{
	    d = (*dst->fetch) (dst);
	    m = FbInC(d,0,a,t);
	    n = FbInC(d,8,a,t);
	    o = FbInC(d,16,a,t);
	    p = FbInC(d,24,a,t);
	    d = m|n|o|p;
	}
	(*dst->store) (dst, d);
    }
}

void
fbCombineOutU (FbCompositeOperand    *src,
	       FbCompositeOperand    *msk,
	       FbCompositeOperand    *dst)
{
    CARD32  s, d;
    CARD16  a;
    CARD16  t;
    CARD32  m,n,o,p;

    d = (*dst->fetch) (dst);
    a = ~d >> 24;
    s = 0;
    if (a)
    {
	s = fbCombineMaskU (src, msk);
	if (a != 0xff)
	{
	    m = FbInU(s,0,a,t);
	    n = FbInU(s,8,a,t);
	    o = FbInU(s,16,a,t);
	    p = FbInU(s,24,a,t);
	    s = m|n|o|p;
	}
    }
    (*dst->store) (dst, s);
}

void
fbCombineOutC (FbCompositeOperand    *src,
	       FbCompositeOperand    *msk,
	       FbCompositeOperand    *dst)
{
    CARD32  s, d;
    CARD16  a;
    CARD16  t;
    CARD32  m,n,o,p;

    d = (*dst->fetch) (dst);
    a = ~d >> 24;
    s = 0;
    if (a)
    {
	s = fbCombineMaskValueC (src, msk);
	if (a != 0xff)
	{
	    m = FbInU(s,0,a,t);
	    n = FbInU(s,8,a,t);
	    o = FbInU(s,16,a,t);
	    p = FbInU(s,24,a,t);
	    s = m|n|o|p;
	}
    }
    (*dst->store) (dst, s);
}

void
fbCombineOutReverseU (FbCompositeOperand *src,
		      FbCompositeOperand *msk,
		      FbCompositeOperand *dst)
{
    CARD32  s, d;
    CARD16  a;
    CARD16  t;
    CARD32  m,n,o,p;

    s = fbCombineMaskAlphaU (src, msk);
    a = ~s >> 24;
    if (a != 0xff)
    {
	d = 0;
	if (a)
	{
	    d = (*dst->fetch) (dst);
	    m = FbInU(d,0,a,t);
	    n = FbInU(d,8,a,t);
	    o = FbInU(d,16,a,t);
	    p = FbInU(d,24,a,t);
	    d = m|n|o|p;
	}
	(*dst->store) (dst, d);
    }
}

void
fbCombineOutReverseC (FbCompositeOperand *src,
		      FbCompositeOperand *msk,
		      FbCompositeOperand *dst)
{
    CARD32  s, d;
    CARD32  a;
    CARD16  t;
    CARD32  m,n,o,p;

    s = fbCombineMaskAlphaC (src, msk);
    a = ~s;
    if (a != 0xffffffff)
    {
	d = 0;
	if (a)
	{
	    d = (*dst->fetch) (dst);
	    m = FbInC(d,0,a,t);
	    n = FbInC(d,8,a,t);
	    o = FbInC(d,16,a,t);
	    p = FbInC(d,24,a,t);
	    d = m|n|o|p;
	}
	(*dst->store) (dst, d);
    }
}

void
fbCombineAtopU (FbCompositeOperand   *src,
		FbCompositeOperand   *msk,
		FbCompositeOperand   *dst)
{
    CARD32  s, d;
    CARD16  ad, as;
    CARD16  t,u,v;
    CARD32  m,n,o,p;
    
    s = fbCombineMaskU (src, msk);
    d = (*dst->fetch) (dst);
    ad = ~s >> 24;
    as = d >> 24;
    m = FbGen(s,d,0,as,ad,t,u,v);
    n = FbGen(s,d,8,as,ad,t,u,v);
    o = FbGen(s,d,16,as,ad,t,u,v);
    p = FbGen(s,d,24,as,ad,t,u,v);
    (*dst->store) (dst, m|n|o|p);
}

void
fbCombineAtopC (FbCompositeOperand   *src,
		FbCompositeOperand   *msk,
		FbCompositeOperand   *dst)
{
    FbCompSrc	cs;
    CARD32  s, d;
    CARD32  ad;
    CARD16  as;
    CARD16  t, u, v;
    CARD32  m,n,o,p;
    
    cs = fbCombineMaskC (src, msk);
    d = (*dst->fetch) (dst);
    s = cs.value;
    ad = cs.alpha;
    as = d >> 24;
    m = FbGen(s,d,0,as,FbGet8(ad,0),t,u,v);
    n = FbGen(s,d,8,as,FbGet8(ad,8),t,u,v);
    o = FbGen(s,d,16,as,FbGet8(ad,16),t,u,v);
    p = FbGen(s,d,24,as,FbGet8(ad,24),t,u,v);
    (*dst->store) (dst, m|n|o|p);
}

void
fbCombineAtopReverseU (FbCompositeOperand    *src,
		       FbCompositeOperand    *msk,
		       FbCompositeOperand    *dst)
{
    CARD32  s, d;
    CARD16  ad, as;
    CARD16  t, u, v;
    CARD32  m,n,o,p;
    
    s = fbCombineMaskU (src, msk);
    d = (*dst->fetch) (dst);
    ad = s >> 24;
    as = ~d >> 24;
    m = FbGen(s,d,0,as,ad,t,u,v);
    n = FbGen(s,d,8,as,ad,t,u,v);
    o = FbGen(s,d,16,as,ad,t,u,v);
    p = FbGen(s,d,24,as,ad,t,u,v);
    (*dst->store) (dst, m|n|o|p);
}

void
fbCombineAtopReverseC (FbCompositeOperand    *src,
		       FbCompositeOperand    *msk,
		       FbCompositeOperand    *dst)
{
    FbCompSrc	cs;
    CARD32  s, d, ad;
    CARD16  as;
    CARD16  t, u, v;
    CARD32  m,n,o,p;
    
    cs = fbCombineMaskC (src, msk);
    d = (*dst->fetch) (dst);
    s = cs.value;
    ad = cs.alpha;
    as = ~d >> 24;
    m = FbGen(s,d,0,as,FbGet8(ad,0),t,u,v);
    n = FbGen(s,d,8,as,FbGet8(ad,8),t,u,v);
    o = FbGen(s,d,16,as,FbGet8(ad,16),t,u,v);
    p = FbGen(s,d,24,as,FbGet8(ad,24),t,u,v);
    (*dst->store) (dst, m|n|o|p);
}

void
fbCombineXorU (FbCompositeOperand    *src,
	       FbCompositeOperand    *msk,
	       FbCompositeOperand    *dst)
{
    CARD32  s, d;
    CARD16  ad, as;
    CARD16  t, u, v;
    CARD32  m,n,o,p;
    
    s = fbCombineMaskU (src, msk);
    d = (*dst->fetch) (dst);
    ad = ~s >> 24;
    as = ~d >> 24;
    m = FbGen(s,d,0,as,ad,t,u,v);
    n = FbGen(s,d,8,as,ad,t,u,v);
    o = FbGen(s,d,16,as,ad,t,u,v);
    p = FbGen(s,d,24,as,ad,t,u,v);
    (*dst->store) (dst, m|n|o|p);
}

void
fbCombineXorC (FbCompositeOperand    *src,
	       FbCompositeOperand    *msk,
	       FbCompositeOperand    *dst)
{
    FbCompSrc	cs;
    CARD32  s, d, ad;
    CARD16  as;
    CARD16  t, u, v;
    CARD32  m,n,o,p;
    
    cs = fbCombineMaskC (src, msk);
    d = (*dst->fetch) (dst);
    s = cs.value;
    ad = ~cs.alpha;
    as = ~d >> 24;
    m = FbGen(s,d,0,as,ad,t,u,v);
    n = FbGen(s,d,8,as,ad,t,u,v);
    o = FbGen(s,d,16,as,ad,t,u,v);
    p = FbGen(s,d,24,as,ad,t,u,v);
    (*dst->store) (dst, m|n|o|p);
}

void
fbCombineAddU (FbCompositeOperand    *src,
	       FbCompositeOperand    *msk,
	       FbCompositeOperand    *dst)
{
    CARD32  s, d;
    CARD16  t;
    CARD32  m,n,o,p;

    s = fbCombineMaskU (src, msk);
    if (s == ~0)
	(*dst->store) (dst, s);
    else
    {
	d = (*dst->fetch) (dst);
	if (s && d != ~0)
	{
	    m = FbAdd(s,d,0,t);
	    n = FbAdd(s,d,8,t);
	    o = FbAdd(s,d,16,t);
	    p = FbAdd(s,d,24,t);
	    (*dst->store) (dst, m|n|o|p);
	}
    }
}

void
fbCombineAddC (FbCompositeOperand    *src,
	       FbCompositeOperand    *msk,
	       FbCompositeOperand    *dst)
{
    CARD32  s, d;
    CARD16  t;
    CARD32  m,n,o,p;

    s = fbCombineMaskValueC (src, msk);
    if (s == ~0)
	(*dst->store) (dst, s);
    else
    {
	d = (*dst->fetch) (dst);
	if (s && d != ~0)
	{
	    m = FbAdd(s,d,0,t);
	    n = FbAdd(s,d,8,t);
	    o = FbAdd(s,d,16,t);
	    p = FbAdd(s,d,24,t);
	    (*dst->store) (dst, m|n|o|p);
	}
    }
}

void
fbCombineSaturateU (FbCompositeOperand   *src,
		    FbCompositeOperand   *msk,
		    FbCompositeOperand   *dst)
{
    CARD32  s = fbCombineMaskU (src, msk), d;
#if 0
    CARD16  sa, da;
    CARD16  ad, as;
    CARD16  t;
    CARD32  m,n,o,p;
    
    d = (*dst->fetch) (dst);
    sa = s >> 24;
    da = ~d >> 24;
    if (sa <= da)
    {
	m = FbAdd(s,d,0,t);
	n = FbAdd(s,d,8,t);
	o = FbAdd(s,d,16,t);
	p = FbAdd(s,d,24,t);
    }
    else
    {
	as = (da << 8) / sa;
	ad = 0xff;
	m = FbGen(s,d,0,as,ad,t,u,v);
	n = FbGen(s,d,8,as,ad,t,u,v);
	o = FbGen(s,d,16,as,ad,t,u,v);
	p = FbGen(s,d,24,as,ad,t,u,v);
    }
    (*dst->store) (dst, m|n|o|p);
#else
    if ((s >> 24) == 0xff)
	(*dst->store) (dst, s);
    else
    {
	d = (*dst->fetch) (dst);
	if ((s >> 24) > (d >> 24))
	    (*dst->store) (dst, s);
    }
#endif
}

void
fbCombineSaturateC (FbCompositeOperand   *src,
		    FbCompositeOperand   *msk,
		    FbCompositeOperand   *dst)
{
    FbCompSrc	cs;
    CARD32  s, d;
    CARD16  sa, sr, sg, sb, da;
    CARD16  t, u, v;
    CARD32  m,n,o,p;
    
    cs = fbCombineMaskC (src, msk);
    d = (*dst->fetch) (dst);
    s = cs.value;
    sa = (cs.alpha >> 24) & 0xff;
    sr = (cs.alpha >> 16) & 0xff;
    sg = (cs.alpha >>  8) & 0xff;
    sb = (cs.alpha      ) & 0xff;
    da = ~d >> 24;
    
    if (sb <= da)
	m = FbAdd(s,d,0,t);
    else
	m = FbGen (s, d, 0, (da << 8) / sb, 0xff, t, u, v);
    
    if (sg <= da)
	n = FbAdd(s,d,8,t);
    else
	n = FbGen (s, d, 8, (da << 8) / sg, 0xff, t, u, v);
    
    if (sr < da)
	o = FbAdd(s,d,16,t);
    else
	o = FbGen (s, d, 16, (da << 8) / sr, 0xff, t, u, v);

    if (sa <= da)
	p = FbAdd(s,d,24,t);
    else
	p = FbGen (s, d, 24, (da << 8) / sa, 0xff, t, u, v);
    
    (*dst->store) (dst, m|n|o|p);
}

/*
 * All of the disjoint composing functions

 The four entries in the first column indicate what source contributions
 come from each of the four areas of the picture -- areas covered by neither
 A nor B, areas covered only by A, areas covered only by B and finally
 areas covered by both A and B.
 
		Disjoint			Conjoint
		Fa		Fb		Fa		Fb
(0,0,0,0)	0		0		0		0
(0,A,0,A)	1		0		1		0
(0,0,B,B)	0		1		0		1
(0,A,B,A)	1		min((1-a)/b,1)	1		max(1-a/b,0)
(0,A,B,B)	min((1-b)/a,1)	1		max(1-b/a,0)	1		
(0,0,0,A)	max(1-(1-b)/a,0) 0		min(1,b/a)	0
(0,0,0,B)	0		max(1-(1-a)/b,0) 0		min(a/b,1)
(0,A,0,0)	min(1,(1-b)/a)	0		max(1-b/a,0)	0
(0,0,B,0)	0		min(1,(1-a)/b)	0		max(1-a/b,0)
(0,0,B,A)	max(1-(1-b)/a,0) min(1,(1-a)/b)	 min(1,b/a)	max(1-a/b,0)
(0,A,0,B)	min(1,(1-b)/a)	max(1-(1-a)/b,0) max(1-b/a,0)	min(1,a/b)
(0,A,B,0)	min(1,(1-b)/a)	min(1,(1-a)/b)	max(1-b/a,0)	max(1-a/b,0)

 */

#define CombineAOut 1
#define CombineAIn  2
#define CombineBOut 4
#define CombineBIn  8

#define CombineClear	0
#define CombineA	(CombineAOut|CombineAIn)
#define CombineB	(CombineBOut|CombineBIn)
#define CombineAOver	(CombineAOut|CombineBOut|CombineAIn)
#define CombineBOver	(CombineAOut|CombineBOut|CombineBIn)
#define CombineAAtop	(CombineBOut|CombineAIn)
#define CombineBAtop	(CombineAOut|CombineBIn)
#define CombineXor	(CombineAOut|CombineBOut)

/* portion covered by a but not b */
CARD8
fbCombineDisjointOutPart (CARD8 a, CARD8 b)
{
    /* min (1, (1-b) / a) */
    
    b = ~b;		    /* 1 - b */
    if (b >= a)		    /* 1 - b >= a -> (1-b)/a >= 1 */
	return 0xff;	    /* 1 */
    return FbIntDiv(b,a);   /* (1-b) / a */
}

/* portion covered by both a and b */
CARD8
fbCombineDisjointInPart (CARD8 a, CARD8 b)
{
    /* max (1-(1-b)/a,0) */
    /*  = - min ((1-b)/a - 1, 0) */
    /*  = 1 - min (1, (1-b)/a) */

    b = ~b;		    /* 1 - b */
    if (b >= a)		    /* 1 - b >= a -> (1-b)/a >= 1 */
	return 0;	    /* 1 - 1 */
    return ~FbIntDiv(b,a);  /* 1 - (1-b) / a */
}

void
fbCombineDisjointGeneralU (FbCompositeOperand   *src,
			   FbCompositeOperand   *msk,
			   FbCompositeOperand   *dst,
			   CARD8		combine)
{
    CARD32  s, d;
    CARD32  m,n,o,p;
    CARD16  Fa, Fb, t, u, v;
    CARD8   sa, da;

    s = fbCombineMaskU (src, msk);
    sa = s >> 24;
    
    d = (*dst->fetch) (dst);
    da = d >> 24;
    
    switch (combine & CombineA) {
    default:
	Fa = 0;
	break;
    case CombineAOut:
	Fa = fbCombineDisjointOutPart (sa, da);
	break;
    case CombineAIn:
	Fa = fbCombineDisjointInPart (sa, da);
	break;
    case CombineA:
	Fa = 0xff;
	break;
    }
    
    switch (combine & CombineB) {
    default:
	Fb = 0;
	break;
    case CombineBOut:
	Fb = fbCombineDisjointOutPart (da, sa);
	break;
    case CombineBIn:
	Fb = fbCombineDisjointInPart (da, sa);
	break;
    case CombineB:
	Fb = 0xff;
	break;
    }
    m = FbGen (s,d,0,Fa,Fb,t,u,v);
    n = FbGen (s,d,8,Fa,Fb,t,u,v);
    o = FbGen (s,d,16,Fa,Fb,t,u,v);
    p = FbGen (s,d,24,Fa,Fb,t,u,v);
    s = m|n|o|p;
    (*dst->store) (dst, s);
}

void
fbCombineDisjointGeneralC (FbCompositeOperand   *src,
			   FbCompositeOperand   *msk,
			   FbCompositeOperand   *dst,
			   CARD8		combine)
{
    FbCompSrc	cs;
    CARD32  s, d;
    CARD32  m,n,o,p;
    CARD32  Fa;
    CARD16  Fb, t, u, v;
    CARD32  sa;
    CARD8   da;

    cs = fbCombineMaskC (src, msk);
    s = cs.value;
    sa = cs.alpha;
    
    d = (*dst->fetch) (dst);
    da = d >> 24;
    
    switch (combine & CombineA) {
    default:
	Fa = 0;
	break;
    case CombineAOut:
	m = fbCombineDisjointOutPart ((CARD8) (sa >> 0), da);
	n = fbCombineDisjointOutPart ((CARD8) (sa >> 8), da) << 8;
	o = fbCombineDisjointOutPart ((CARD8) (sa >> 16), da) << 16;
	p = fbCombineDisjointOutPart ((CARD8) (sa >> 24), da) << 24;
	Fa = m|n|o|p;
	break;
    case CombineAIn:
	m = fbCombineDisjointOutPart ((CARD8) (sa >> 0), da);
	n = fbCombineDisjointOutPart ((CARD8) (sa >> 8), da) << 8;
	o = fbCombineDisjointOutPart ((CARD8) (sa >> 16), da) << 16;
	p = fbCombineDisjointOutPart ((CARD8) (sa >> 24), da) << 24;
	Fa = m|n|o|p;
	break;
    case CombineA:
	Fa = 0xffffffff;
	break;
    }
    
    switch (combine & CombineB) {
    default:
	Fb = 0;
	break;
    case CombineBOut:
	Fb = fbCombineDisjointOutPart (da, sa);
	break;
    case CombineBIn:
	Fb = fbCombineDisjointInPart (da, sa);
	break;
    case CombineB:
	Fb = 0xff;
	break;
    }
    m = FbGen (s,d,0,FbGet8(Fa,0),Fb,t,u,v);
    n = FbGen (s,d,8,FbGet8(Fa,8),Fb,t,u,v);
    o = FbGen (s,d,16,FbGet8(Fa,16),Fb,t,u,v);
    p = FbGen (s,d,24,FbGet8(Fa,24),Fb,t,u,v);
    s = m|n|o|p;
    (*dst->store) (dst, s);
}

void
fbCombineDisjointOverU (FbCompositeOperand   *src,
			FbCompositeOperand   *msk,
			FbCompositeOperand   *dst)
{
    CARD32  s, d;
    CARD16  a;
    CARD16  t;
    CARD32  m,n,o,p;

    s = fbCombineMaskU (src, msk);
    a = s >> 24;
    if (a != 0x00)
    {
	if (a != 0xff)
	{
	    d = (*dst->fetch) (dst);
	    a = fbCombineDisjointOutPart (d >> 24, a);
	    m = FbOverU(s,d,0,a,t);
	    n = FbOverU(s,d,8,a,t);
	    o = FbOverU(s,d,16,a,t);
	    p = FbOverU(s,d,24,a,t);
	    s = m|n|o|p;
	}
	(*dst->store) (dst, s);
    }
}

void
fbCombineDisjointOverC (FbCompositeOperand   *src,
			FbCompositeOperand   *msk,
			FbCompositeOperand   *dst)
{
    fbCombineDisjointGeneralC (src, msk, dst, CombineAOver);
}

void
fbCombineDisjointOverReverseU (FbCompositeOperand    *src,
			       FbCompositeOperand    *msk,
			       FbCompositeOperand    *dst)
{
    fbCombineDisjointGeneralU (src, msk, dst, CombineBOver);
}

void
fbCombineDisjointOverReverseC (FbCompositeOperand    *src,
			       FbCompositeOperand    *msk,
			       FbCompositeOperand    *dst)
{
    fbCombineDisjointGeneralC (src, msk, dst, CombineBOver);
}

void
fbCombineDisjointInU (FbCompositeOperand	    *src,
		      FbCompositeOperand	    *msk,
		      FbCompositeOperand	    *dst)
{
    fbCombineDisjointGeneralU (src, msk, dst, CombineAIn);
}

void
fbCombineDisjointInC (FbCompositeOperand	    *src,
		      FbCompositeOperand	    *msk,
		      FbCompositeOperand	    *dst)
{
    fbCombineDisjointGeneralC (src, msk, dst, CombineAIn);
}

void
fbCombineDisjointInReverseU (FbCompositeOperand  *src,
			     FbCompositeOperand  *msk,
			     FbCompositeOperand  *dst)
{
    fbCombineDisjointGeneralU (src, msk, dst, CombineBIn);
}

void
fbCombineDisjointInReverseC (FbCompositeOperand  *src,
			     FbCompositeOperand  *msk,
			     FbCompositeOperand  *dst)
{
    fbCombineDisjointGeneralC (src, msk, dst, CombineBIn);
}

void
fbCombineDisjointOutU (FbCompositeOperand    *src,
		       FbCompositeOperand    *msk,
		       FbCompositeOperand    *dst)
{
    fbCombineDisjointGeneralU (src, msk, dst, CombineAOut);
}

void
fbCombineDisjointOutC (FbCompositeOperand    *src,
		       FbCompositeOperand    *msk,
		       FbCompositeOperand    *dst)
{
    fbCombineDisjointGeneralC (src, msk, dst, CombineAOut);
}

void
fbCombineDisjointOutReverseU (FbCompositeOperand *src,
			      FbCompositeOperand *msk,
			      FbCompositeOperand *dst)
{
    fbCombineDisjointGeneralU (src, msk, dst, CombineBOut);
}

void
fbCombineDisjointOutReverseC (FbCompositeOperand *src,
			      FbCompositeOperand *msk,
			      FbCompositeOperand *dst)
{
    fbCombineDisjointGeneralC (src, msk, dst, CombineBOut);
}

void
fbCombineDisjointAtopU (FbCompositeOperand   *src,
			FbCompositeOperand   *msk,
			FbCompositeOperand   *dst)
{
    fbCombineDisjointGeneralU (src, msk, dst, CombineAAtop);
}

void
fbCombineDisjointAtopC (FbCompositeOperand   *src,
			FbCompositeOperand   *msk,
			FbCompositeOperand   *dst)
{
    fbCombineDisjointGeneralC (src, msk, dst, CombineAAtop);
}

void
fbCombineDisjointAtopReverseU (FbCompositeOperand    *src,
			       FbCompositeOperand    *msk,
			       FbCompositeOperand    *dst)
{
    fbCombineDisjointGeneralU (src, msk, dst, CombineBAtop);
}

void
fbCombineDisjointAtopReverseC (FbCompositeOperand    *src,
			       FbCompositeOperand    *msk,
			       FbCompositeOperand    *dst)
{
    fbCombineDisjointGeneralC (src, msk, dst, CombineBAtop);
}

void
fbCombineDisjointXorU (FbCompositeOperand    *src,
		       FbCompositeOperand    *msk,
		       FbCompositeOperand    *dst)
{
    fbCombineDisjointGeneralU (src, msk, dst, CombineXor);
}

void
fbCombineDisjointXorC (FbCompositeOperand    *src,
		       FbCompositeOperand    *msk,
		       FbCompositeOperand    *dst)
{
    fbCombineDisjointGeneralC (src, msk, dst, CombineXor);
}

/* portion covered by a but not b */
CARD8
fbCombineConjointOutPart (CARD8 a, CARD8 b)
{
    /* max (1-b/a,0) */
    /* = 1-min(b/a,1) */
    
    /* min (1, (1-b) / a) */
    
    if (b >= a)		    /* b >= a -> b/a >= 1 */
	return 0x00;	    /* 0 */
    return ~FbIntDiv(b,a);   /* 1 - b/a */
}

/* portion covered by both a and b */
CARD8
fbCombineConjointInPart (CARD8 a, CARD8 b)
{
    /* min (1,b/a) */

    if (b >= a)		    /* b >= a -> b/a >= 1 */
	return 0xff;	    /* 1 */
    return FbIntDiv(b,a);   /* b/a */
}

void
fbCombineConjointGeneralU (FbCompositeOperand   *src,
			   FbCompositeOperand   *msk,
			   FbCompositeOperand   *dst,
			   CARD8		combine)
{
    CARD32  s, d;
    CARD32  m,n,o,p;
    CARD16  Fa, Fb, t, u, v;
    CARD8   sa, da;

    s = fbCombineMaskU (src, msk);
    sa = s >> 24;
    
    d = (*dst->fetch) (dst);
    da = d >> 24;
    
    switch (combine & CombineA) {
    default:
	Fa = 0;
	break;
    case CombineAOut:
	Fa = fbCombineConjointOutPart (sa, da);
	break;
    case CombineAIn:
	Fa = fbCombineConjointInPart (sa, da);
	break;
    case CombineA:
	Fa = 0xff;
	break;
    }
    
    switch (combine & CombineB) {
    default:
	Fb = 0;
	break;
    case CombineBOut:
	Fb = fbCombineConjointOutPart (da, sa);
	break;
    case CombineBIn:
	Fb = fbCombineConjointInPart (da, sa);
	break;
    case CombineB:
	Fb = 0xff;
	break;
    }
    m = FbGen (s,d,0,Fa,Fb,t,u,v);
    n = FbGen (s,d,8,Fa,Fb,t,u,v);
    o = FbGen (s,d,16,Fa,Fb,t,u,v);
    p = FbGen (s,d,24,Fa,Fb,t,u,v);
    s = m|n|o|p;
    (*dst->store) (dst, s);
}

void
fbCombineConjointGeneralC (FbCompositeOperand   *src,
			   FbCompositeOperand   *msk,
			   FbCompositeOperand   *dst,
			   CARD8		combine)
{
    FbCompSrc	cs;
    CARD32  s, d;
    CARD32  m,n,o,p;
    CARD32  Fa;
    CARD16  Fb, t, u, v;
    CARD32  sa;
    CARD8   da;

    cs = fbCombineMaskC (src, msk);
    s = cs.value;
    sa = cs.alpha;
    
    d = (*dst->fetch) (dst);
    da = d >> 24;
    
    switch (combine & CombineA) {
    default:
	Fa = 0;
	break;
    case CombineAOut:
	m = fbCombineConjointOutPart ((CARD8) (sa >> 0), da);
	n = fbCombineConjointOutPart ((CARD8) (sa >> 8), da) << 8;
	o = fbCombineConjointOutPart ((CARD8) (sa >> 16), da) << 16;
	p = fbCombineConjointOutPart ((CARD8) (sa >> 24), da) << 24;
	Fa = m|n|o|p;
	break;
    case CombineAIn:
	m = fbCombineConjointOutPart ((CARD8) (sa >> 0), da);
	n = fbCombineConjointOutPart ((CARD8) (sa >> 8), da) << 8;
	o = fbCombineConjointOutPart ((CARD8) (sa >> 16), da) << 16;
	p = fbCombineConjointOutPart ((CARD8) (sa >> 24), da) << 24;
	Fa = m|n|o|p;
	break;
    case CombineA:
	Fa = 0xffffffff;
	break;
    }
    
    switch (combine & CombineB) {
    default:
	Fb = 0;
	break;
    case CombineBOut:
	Fb = fbCombineConjointOutPart (da, sa);
	break;
    case CombineBIn:
	Fb = fbCombineConjointInPart (da, sa);
	break;
    case CombineB:
	Fb = 0xff;
	break;
    }
    m = FbGen (s,d,0,FbGet8(Fa,0),Fb,t,u,v);
    n = FbGen (s,d,8,FbGet8(Fa,8),Fb,t,u,v);
    o = FbGen (s,d,16,FbGet8(Fa,16),Fb,t,u,v);
    p = FbGen (s,d,24,FbGet8(Fa,24),Fb,t,u,v);
    s = m|n|o|p;
    (*dst->store) (dst, s);
}

void
fbCombineConjointOverU (FbCompositeOperand   *src,
			FbCompositeOperand   *msk,
			FbCompositeOperand   *dst)
{
    fbCombineConjointGeneralU (src, msk, dst, CombineAOver);
/*
    CARD32  s, d;
    CARD16  a;
    CARD16  t;
    CARD32  m,n,o,p;

    s = fbCombineMaskU (src, msk);
    a = s >> 24;
    if (a != 0x00)
    {
	if (a != 0xff)
	{
	    d = (*dst->fetch) (dst);
	    a = fbCombineConjointOutPart (d >> 24, a);
	    m = FbOverU(s,d,0,a,t);
	    n = FbOverU(s,d,8,a,t);
	    o = FbOverU(s,d,16,a,t);
	    p = FbOverU(s,d,24,a,t);
	    s = m|n|o|p;
	}
	(*dst->store) (dst, s);
    }
 */
}

void
fbCombineConjointOverC (FbCompositeOperand   *src,
			FbCompositeOperand   *msk,
			FbCompositeOperand   *dst)
{
    fbCombineConjointGeneralC (src, msk, dst, CombineAOver);
}

void
fbCombineConjointOverReverseU (FbCompositeOperand    *src,
			       FbCompositeOperand    *msk,
			       FbCompositeOperand    *dst)
{
    fbCombineConjointGeneralU (src, msk, dst, CombineBOver);
}

void
fbCombineConjointOverReverseC (FbCompositeOperand    *src,
			       FbCompositeOperand    *msk,
			       FbCompositeOperand    *dst)
{
    fbCombineConjointGeneralC (src, msk, dst, CombineBOver);
}

void
fbCombineConjointInU (FbCompositeOperand	    *src,
		      FbCompositeOperand	    *msk,
		      FbCompositeOperand	    *dst)
{
    fbCombineConjointGeneralU (src, msk, dst, CombineAIn);
}

void
fbCombineConjointInC (FbCompositeOperand	    *src,
		      FbCompositeOperand	    *msk,
		      FbCompositeOperand	    *dst)
{
    fbCombineConjointGeneralC (src, msk, dst, CombineAIn);
}

void
fbCombineConjointInReverseU (FbCompositeOperand  *src,
			     FbCompositeOperand  *msk,
			     FbCompositeOperand  *dst)
{
    fbCombineConjointGeneralU (src, msk, dst, CombineBIn);
}

void
fbCombineConjointInReverseC (FbCompositeOperand  *src,
			     FbCompositeOperand  *msk,
			     FbCompositeOperand  *dst)
{
    fbCombineConjointGeneralC (src, msk, dst, CombineBIn);
}

void
fbCombineConjointOutU (FbCompositeOperand    *src,
		       FbCompositeOperand    *msk,
		       FbCompositeOperand    *dst)
{
    fbCombineConjointGeneralU (src, msk, dst, CombineAOut);
}

void
fbCombineConjointOutC (FbCompositeOperand    *src,
		       FbCompositeOperand    *msk,
		       FbCompositeOperand    *dst)
{
    fbCombineConjointGeneralC (src, msk, dst, CombineAOut);
}

void
fbCombineConjointOutReverseU (FbCompositeOperand *src,
			      FbCompositeOperand *msk,
			      FbCompositeOperand *dst)
{
    fbCombineConjointGeneralU (src, msk, dst, CombineBOut);
}

void
fbCombineConjointOutReverseC (FbCompositeOperand *src,
			      FbCompositeOperand *msk,
			      FbCompositeOperand *dst)
{
    fbCombineConjointGeneralC (src, msk, dst, CombineBOut);
}

void
fbCombineConjointAtopU (FbCompositeOperand   *src,
			FbCompositeOperand   *msk,
			FbCompositeOperand   *dst)
{
    fbCombineConjointGeneralU (src, msk, dst, CombineAAtop);
}

void
fbCombineConjointAtopC (FbCompositeOperand   *src,
			FbCompositeOperand   *msk,
			FbCompositeOperand   *dst)
{
    fbCombineConjointGeneralC (src, msk, dst, CombineAAtop);
}

void
fbCombineConjointAtopReverseU (FbCompositeOperand    *src,
			       FbCompositeOperand    *msk,
			       FbCompositeOperand    *dst)
{
    fbCombineConjointGeneralU (src, msk, dst, CombineBAtop);
}

void
fbCombineConjointAtopReverseC (FbCompositeOperand    *src,
			       FbCompositeOperand    *msk,
			       FbCompositeOperand    *dst)
{
    fbCombineConjointGeneralC (src, msk, dst, CombineBAtop);
}

void
fbCombineConjointXorU (FbCompositeOperand    *src,
		       FbCompositeOperand    *msk,
		       FbCompositeOperand    *dst)
{
    fbCombineConjointGeneralU (src, msk, dst, CombineXor);
}

void
fbCombineConjointXorC (FbCompositeOperand    *src,
		       FbCompositeOperand    *msk,
		       FbCompositeOperand    *dst)
{
    fbCombineConjointGeneralC (src, msk, dst, CombineXor);
}

FbCombineFunc	fbCombineFuncU[] = {
    fbCombineClear,
    fbCombineSrcU,
    fbCombineDst,
    fbCombineOverU,
    fbCombineOverReverseU,
    fbCombineInU,
    fbCombineInReverseU,
    fbCombineOutU,
    fbCombineOutReverseU,
    fbCombineAtopU,
    fbCombineAtopReverseU,
    fbCombineXorU,
    fbCombineAddU,
    fbCombineDisjointOverU, /* Saturate */
    0,
    0,
    fbCombineClear,
    fbCombineSrcU,
    fbCombineDst,
    fbCombineDisjointOverU,
    fbCombineDisjointOverReverseU,
    fbCombineDisjointInU,
    fbCombineDisjointInReverseU,
    fbCombineDisjointOutU,
    fbCombineDisjointOutReverseU,
    fbCombineDisjointAtopU,
    fbCombineDisjointAtopReverseU,
    fbCombineDisjointXorU,
    0,
    0,
    0,
    0,
    fbCombineClear,
    fbCombineSrcU,
    fbCombineDst,
    fbCombineConjointOverU,
    fbCombineConjointOverReverseU,
    fbCombineConjointInU,
    fbCombineConjointInReverseU,
    fbCombineConjointOutU,
    fbCombineConjointOutReverseU,
    fbCombineConjointAtopU,
    fbCombineConjointAtopReverseU,
    fbCombineConjointXorU,
};

FbCombineFunc	fbCombineFuncC[] = {
    fbCombineClear,
    fbCombineSrcC,
    fbCombineDst,
    fbCombineOverC,
    fbCombineOverReverseC,
    fbCombineInC,
    fbCombineInReverseC,
    fbCombineOutC,
    fbCombineOutReverseC,
    fbCombineAtopC,
    fbCombineAtopReverseC,
    fbCombineXorC,
    fbCombineAddC,
    fbCombineDisjointOverC, /* Saturate */
    0,
    0,
    fbCombineClear,	    /* 0x10 */
    fbCombineSrcC,
    fbCombineDst,
    fbCombineDisjointOverC,
    fbCombineDisjointOverReverseC,
    fbCombineDisjointInC,
    fbCombineDisjointInReverseC,
    fbCombineDisjointOutC,
    fbCombineDisjointOutReverseC,
    fbCombineDisjointAtopC,
    fbCombineDisjointAtopReverseC,
    fbCombineDisjointXorC,  /* 0x1b */
    0,
    0,
    0,
    0,
    fbCombineClear,
    fbCombineSrcC,
    fbCombineDst,
    fbCombineConjointOverC,
    fbCombineConjointOverReverseC,
    fbCombineConjointInC,
    fbCombineConjointInReverseC,
    fbCombineConjointOutC,
    fbCombineConjointOutReverseC,
    fbCombineConjointAtopC,
    fbCombineConjointAtopReverseC,
    fbCombineConjointXorC,
};

/*
 * All of the fetch functions
 */

CARD32
fbFetch_a8r8g8b8 (FbCompositeOperand *op)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    return ((CARD32 *)line)[offset >> 5];
}

CARD32
fbFetch_x8r8g8b8 (FbCompositeOperand *op)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    return ((CARD32 *)line)[offset >> 5] | 0xff000000;
}

CARD32
fbFetch_a8b8g8r8 (FbCompositeOperand *op)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD32  pixel = ((CARD32 *)line)[offset >> 5];

    return ((pixel & 0xff000000) |
	    ((pixel >> 16) & 0xff) |
	    (pixel & 0x0000ff00) |
	    ((pixel & 0xff) << 16));
}

CARD32
fbFetch_x8b8g8r8 (FbCompositeOperand *op)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD32  pixel = ((CARD32 *)line)[offset >> 5];

    return ((0xff000000) |
	    ((pixel >> 16) & 0xff) |
	    (pixel & 0x0000ff00) |
	    ((pixel & 0xff) << 16));
}

CARD32
fbFetch_r8g8b8 (FbCompositeOperand *op)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD8   *pixel = ((CARD8 *) line) + (offset >> 3);
#if IMAGE_BYTE_ORDER == MSBFirst
    return (0xff000000 |
	    (pixel[0] << 16) |
	    (pixel[1] << 8) |
	    (pixel[2]));
#else
    return (0xff000000 |
	    (pixel[2] << 16) |
	    (pixel[1] << 8) |
	    (pixel[0]));
#endif
}

CARD32
fbFetch_b8g8r8 (FbCompositeOperand *op)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD8   *pixel = ((CARD8 *) line) + (offset >> 3);
#if IMAGE_BYTE_ORDER == MSBFirst
    return (0xff000000 |
	    (pixel[2] << 16) |
	    (pixel[1] << 8) |
	    (pixel[0]));
#else
    return (0xff000000 |
	    (pixel[0] << 16) |
	    (pixel[1] << 8) |
	    (pixel[2]));
#endif
}

CARD32
fbFetch_r5g6b5 (FbCompositeOperand *op)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD32  pixel = ((CARD16 *) line)[offset >> 4];
    CARD32  r,g,b;

    r = ((pixel & 0xf800) | ((pixel & 0xe000) >> 5)) << 8;
    g = ((pixel & 0x07e0) | ((pixel & 0x0600) >> 6)) << 5;
    b = ((pixel & 0x001c) | ((pixel & 0x001f) << 5)) >> 2;
    return (0xff000000 | r | g | b);
}

CARD32
fbFetch_b5g6r5 (FbCompositeOperand *op)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD32  pixel = ((CARD16 *) line)[offset >> 4];
    CARD32  r,g,b;

    b = ((pixel & 0xf800) | ((pixel & 0xe000) >> 5)) >> 8;
    g = ((pixel & 0x07e0) | ((pixel & 0x0600) >> 6)) << 5;
    r = ((pixel & 0x001c) | ((pixel & 0x001f) << 5)) << 14;
    return (0xff000000 | r | g | b);
}

CARD32
fbFetch_a1r5g5b5 (FbCompositeOperand *op)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD32  pixel = ((CARD16 *) line)[offset >> 4];
    CARD32  a,r,g,b;

    a = (CARD32) ((CARD8) (0 - ((pixel & 0x8000) >> 15))) << 24;
    r = ((pixel & 0x7c00) | ((pixel & 0x7000) >> 5)) << 9;
    g = ((pixel & 0x03e0) | ((pixel & 0x0380) >> 5)) << 6;
    b = ((pixel & 0x001c) | ((pixel & 0x001f) << 5)) >> 2;
    return (a | r | g | b);
}

CARD32
fbFetch_x1r5g5b5 (FbCompositeOperand *op)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD32  pixel = ((CARD16 *) line)[offset >> 4];
    CARD32  r,g,b;

    r = ((pixel & 0x7c00) | ((pixel & 0x7000) >> 5)) << 9;
    g = ((pixel & 0x03e0) | ((pixel & 0x0380) >> 5)) << 6;
    b = ((pixel & 0x001c) | ((pixel & 0x001f) << 5)) >> 2;
    return (0xff000000 | r | g | b);
}

CARD32
fbFetch_a1b5g5r5 (FbCompositeOperand *op)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD32  pixel = ((CARD16 *) line)[offset >> 4];
    CARD32  a,r,g,b;

    a = (CARD32) ((CARD8) (0 - ((pixel & 0x8000) >> 15))) << 24;
    b = ((pixel & 0x7c00) | ((pixel & 0x7000) >> 5)) >> 7;
    g = ((pixel & 0x03e0) | ((pixel & 0x0380) >> 5)) << 6;
    r = ((pixel & 0x001c) | ((pixel & 0x001f) << 5)) << 14;
    return (a | r | g | b);
}

CARD32
fbFetch_x1b5g5r5 (FbCompositeOperand *op)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD32  pixel = ((CARD16 *) line)[offset >> 4];
    CARD32  r,g,b;

    b = ((pixel & 0x7c00) | ((pixel & 0x7000) >> 5)) >> 7;
    g = ((pixel & 0x03e0) | ((pixel & 0x0380) >> 5)) << 6;
    r = ((pixel & 0x001c) | ((pixel & 0x001f) << 5)) << 14;
    return (0xff000000 | r | g | b);
}

CARD32
fbFetch_a8 (FbCompositeOperand *op)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD32   pixel = ((CARD8 *) line)[offset>>3];
    
    return pixel << 24;
}

CARD32
fbFetcha_a8 (FbCompositeOperand *op)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD32   pixel = ((CARD8 *) line)[offset>>3];
    
    pixel |= pixel << 8;
    pixel |= pixel << 16;
    return pixel;
}

CARD32
fbFetch_r3g3b2 (FbCompositeOperand *op)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD32   pixel = ((CARD8 *) line)[offset>>3];
    CARD32  r,g,b;
    
    r = ((pixel & 0xe0) | ((pixel & 0xe0) >> 3) | ((pixel & 0xc0) >> 6)) << 16;
    g = ((pixel & 0x1c) | ((pixel & 0x18) >> 3) | ((pixel & 0x1c) << 3)) << 8;
    b = (((pixel & 0x03)     ) | 
	 ((pixel & 0x03) << 2) | 
	 ((pixel & 0x03) << 4) |
	 ((pixel & 0x03) << 6));
    return (0xff000000 | r | g | b);
}

CARD32
fbFetch_b2g3r3 (FbCompositeOperand *op)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD32   pixel = ((CARD8 *) line)[offset>>3];
    CARD32  r,g,b;
    
    b = (((pixel & 0xc0)     ) | 
	 ((pixel & 0xc0) >> 2) |
	 ((pixel & 0xc0) >> 4) |
	 ((pixel & 0xc0) >> 6));
    g = ((pixel & 0x38) | ((pixel & 0x38) >> 3) | ((pixel & 0x30) << 2)) << 8;
    r = (((pixel & 0x07)     ) | 
	 ((pixel & 0x07) << 3) | 
	 ((pixel & 0x06) << 6)) << 16;
    return (0xff000000 | r | g | b);
}

CARD32
fbFetch_a2r2g2b2 (FbCompositeOperand *op)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD32   pixel = ((CARD8 *) line)[offset>>3];
    CARD32   a,r,g,b;

    a = ((pixel & 0xc0) * 0x55) << 18;
    r = ((pixel & 0x30) * 0x55) << 12;
    g = ((pixel & 0x0c) * 0x55) << 6;
    b = ((pixel & 0x03) * 0x55);
    return a|r|g|b;
}

CARD32
fbFetch_a2b2g2r2 (FbCompositeOperand *op)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD32   pixel = ((CARD8 *) line)[offset>>3];
    CARD32   a,r,g,b;

    a = ((pixel & 0xc0) * 0x55) << 18;
    b = ((pixel & 0x30) * 0x55) >> 6;
    g = ((pixel & 0x0c) * 0x55) << 6;
    r = ((pixel & 0x03) * 0x55) << 16;
    return a|r|g|b;
}

CARD32
fbFetch_c8 (FbCompositeOperand *op)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD32   pixel = ((CARD8 *) line)[offset>>3];

    return op->indexed->rgba[pixel];
}

#define Fetch8(l,o)    (((CARD8 *) (l))[(o) >> 3])
#if IMAGE_BYTE_ORDER == MSBFirst
#define Fetch4(l,o)    ((o) & 2 ? Fetch8(l,o) & 0xf : Fetch8(l,o) >> 4)
#else
#define Fetch4(l,o)    ((o) & 2 ? Fetch8(l,o) >> 4 : Fetch8(l,o) & 0xf)
#endif

CARD32
fbFetch_a4 (FbCompositeOperand *op)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD32  pixel = Fetch4(line, offset);
    
    pixel |= pixel << 4;
    return pixel << 24;
}

CARD32
fbFetcha_a4 (FbCompositeOperand *op)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD32  pixel = Fetch4(line, offset);
    
    pixel |= pixel << 4;
    pixel |= pixel << 8;
    pixel |= pixel << 16;
    return pixel;
}

CARD32
fbFetch_r1g2b1 (FbCompositeOperand *op)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD32  pixel = Fetch4(line, offset);
    CARD32  r,g,b;

    r = ((pixel & 0x8) * 0xff) << 13;
    g = ((pixel & 0x6) * 0x55) << 7;
    b = ((pixel & 0x1) * 0xff);
    return 0xff000000|r|g|b;
}

CARD32
fbFetch_b1g2r1 (FbCompositeOperand *op)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD32  pixel = Fetch4(line, offset);
    CARD32  r,g,b;

    b = ((pixel & 0x8) * 0xff) >> 3;
    g = ((pixel & 0x6) * 0x55) << 7;
    r = ((pixel & 0x1) * 0xff) << 16;
    return 0xff000000|r|g|b;
}

CARD32
fbFetch_a1r1g1b1 (FbCompositeOperand *op)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD32  pixel = Fetch4(line, offset);
    CARD32  a,r,g,b;

    a = ((pixel & 0x8) * 0xff) << 21;
    r = ((pixel & 0x4) * 0xff) << 14;
    g = ((pixel & 0x2) * 0xff) << 7;
    b = ((pixel & 0x1) * 0xff);
    return a|r|g|b;
}

CARD32
fbFetch_a1b1g1r1 (FbCompositeOperand *op)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD32  pixel = Fetch4(line, offset);
    CARD32  a,r,g,b;

    a = ((pixel & 0x8) * 0xff) << 21;
    r = ((pixel & 0x4) * 0xff) >> 3;
    g = ((pixel & 0x2) * 0xff) << 7;
    b = ((pixel & 0x1) * 0xff) << 16;
    return a|r|g|b;
}

CARD32
fbFetch_c4 (FbCompositeOperand *op)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD32  pixel = Fetch4(line, offset);

    return op->indexed->rgba[pixel];
}

CARD32
fbFetcha_a1 (FbCompositeOperand *op)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD32  pixel = ((CARD32 *)line)[offset >> 5];
    CARD32  a;
#if BITMAP_BIT_ORDER == MSBFirst
    a = pixel >> (0x1f - (offset & 0x1f));
#else
    a = pixel >> (offset & 0x1f);
#endif
    a = a & 1;
    a |= a << 1;
    a |= a << 2;
    a |= a << 4;
    a |= a << 8;
    a |= a << 16;
    return a;
}

CARD32
fbFetch_a1 (FbCompositeOperand *op)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD32  pixel = ((CARD32 *)line)[offset >> 5];
    CARD32  a;
#if BITMAP_BIT_ORDER == MSBFirst
    a = pixel >> (0x1f - (offset & 0x1f));
#else
    a = pixel >> (offset & 0x1f);
#endif
    a = a & 1;
    a |= a << 1;
    a |= a << 2;
    a |= a << 4;
    return a << 24;
}

CARD32
fbFetch_g1 (FbCompositeOperand *op)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD32  pixel = ((CARD32 *)line)[offset >> 5];
    CARD32  a;
#if BITMAP_BIT_ORDER == MSBFirst
    a = pixel >> (0x1f - (offset & 0x1f));
#else
    a = pixel >> (offset & 0x1f);
#endif
    a = a & 1;
    return op->indexed->rgba[a];
}

/*
 * All the store functions
 */

#define Splita(v)	CARD32	a = ((v) >> 24), r = ((v) >> 16) & 0xff, g = ((v) >> 8) & 0xff, b = (v) & 0xff
#define Split(v)	CARD32	r = ((v) >> 16) & 0xff, g = ((v) >> 8) & 0xff, b = (v) & 0xff

void
fbStore_a8r8g8b8 (FbCompositeOperand *op, CARD32 value)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    ((CARD32 *)line)[offset >> 5] = value;
}

void
fbStore_x8r8g8b8 (FbCompositeOperand *op, CARD32 value)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    ((CARD32 *)line)[offset >> 5] = value & 0xffffff;
}

void
fbStore_a8b8g8r8 (FbCompositeOperand *op, CARD32 value)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    Splita(value);
    ((CARD32 *)line)[offset >> 5] = a << 24 | b << 16 | g << 8 | r;
}

void
fbStore_x8b8g8r8 (FbCompositeOperand *op, CARD32 value)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    Split(value);
    ((CARD32 *)line)[offset >> 5] = b << 16 | g << 8 | r;
}

void
fbStore_r8g8b8 (FbCompositeOperand *op, CARD32 value)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD8   *pixel = ((CARD8 *) line) + (offset >> 3);
    Split(value);
#if IMAGE_BYTE_ORDER == MSBFirst
    pixel[0] = r;
    pixel[1] = g;
    pixel[2] = b;
#else
    pixel[0] = b;
    pixel[1] = g;
    pixel[2] = r;
#endif
}

void
fbStore_b8g8r8 (FbCompositeOperand *op, CARD32 value)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD8   *pixel = ((CARD8 *) line) + (offset >> 3);
    Split(value);
#if IMAGE_BYTE_ORDER == MSBFirst
    pixel[0] = b;
    pixel[1] = g;
    pixel[2] = r;
#else
    pixel[0] = r;
    pixel[1] = g;
    pixel[2] = b;
#endif
}

void
fbStore_r5g6b5 (FbCompositeOperand *op, CARD32 value)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD16  *pixel = ((CARD16 *) line) + (offset >> 4);
    Split(value);
    *pixel = (((r << 8) & 0xf800) |
	      ((g << 3) & 0x07e0) |
	      ((b >> 3)         ));
}

void
fbStore_b5g6r5 (FbCompositeOperand *op, CARD32 value)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD16  *pixel = ((CARD16 *) line) + (offset >> 4);
    Split(value);
    *pixel = (((b << 8) & 0xf800) |
	      ((g << 3) & 0x07e0) |
	      ((r >> 3)         ));
}

void
fbStore_a1r5g5b5 (FbCompositeOperand *op, CARD32 value)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD16  *pixel = ((CARD16 *) line) + (offset >> 4);
    Splita(value);
    *pixel = (((a << 8) & 0x8000) |
	      ((r << 7) & 0x7c00) |
	      ((g << 2) & 0x03e0) |
	      ((b >> 3)         ));
}

void
fbStore_x1r5g5b5 (FbCompositeOperand *op, CARD32 value)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD16  *pixel = ((CARD16 *) line) + (offset >> 4);
    Split(value);
    *pixel = (((r << 7) & 0x7c00) |
	      ((g << 2) & 0x03e0) |
	      ((b >> 3)         ));
}

void
fbStore_a1b5g5r5 (FbCompositeOperand *op, CARD32 value)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD16  *pixel = ((CARD16 *) line) + (offset >> 4);
    Splita(value);
    *pixel = (((a << 8) & 0x8000) |
	      ((b << 7) & 0x7c00) |
	      ((g << 2) & 0x03e0) |
	      ((r >> 3)         ));
}

void
fbStore_x1b5g5r5 (FbCompositeOperand *op, CARD32 value)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD16  *pixel = ((CARD16 *) line) + (offset >> 4);
    Split(value);
    *pixel = (((b << 7) & 0x7c00) |
	      ((g << 2) & 0x03e0) |
	      ((r >> 3)         ));
}

void
fbStore_a8 (FbCompositeOperand *op, CARD32 value)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD8   *pixel = ((CARD8 *) line) + (offset >> 3);
    *pixel = value >> 24;
}

void
fbStore_r3g3b2 (FbCompositeOperand *op, CARD32 value)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD8   *pixel = ((CARD8 *) line) + (offset >> 3);
    Split(value);
    *pixel = (((r     ) & 0xe0) |
	      ((g >> 3) & 0x1c) |
	      ((b >> 6)       ));
}

void
fbStore_b2g3r3 (FbCompositeOperand *op, CARD32 value)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD8   *pixel = ((CARD8 *) line) + (offset >> 3);
    Split(value);
    *pixel = (((b     ) & 0xe0) |
	      ((g >> 3) & 0x1c) |
	      ((r >> 6)       ));
}

void
fbStore_a2r2g2b2 (FbCompositeOperand *op, CARD32 value)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD8   *pixel = ((CARD8 *) line) + (offset >> 3);
    Splita(value);
    *pixel = (((a     ) & 0xc0) |
	      ((r >> 2) & 0x30) |
	      ((g >> 4) & 0x0c) |
	      ((b >> 6)       ));
}

void
fbStore_c8 (FbCompositeOperand *op, CARD32 value)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD8   *pixel = ((CARD8 *) line) + (offset >> 3);
    *pixel = miIndexToEnt24(op->indexed,value);
}

void
fbStore_g8 (FbCompositeOperand *op, CARD32 value)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD8   *pixel = ((CARD8 *) line) + (offset >> 3);
    *pixel = miIndexToEntY24(op->indexed,value);
}

#define Store8(l,o,v)  (((CARD8 *) l)[(o) >> 3] = (v))
#if IMAGE_BYTE_ORDER == MSBFirst
#define Store4(l,o,v)  Store8(l,o,((o) & 4 ? \
				   (Fetch8(l,o) & 0xf0) | (v) : \
				   (Fetch8(l,o) & 0x0f) | ((v) << 4)))
#else
#define Store4(l,o,v)  Store8(l,o,((o) & 4 ? \
				   (Fetch8(l,o) & 0x0f) | ((v) << 4) : \
				   (Fetch8(l,o) & 0xf0) | (v)))
#endif

void
fbStore_a4 (FbCompositeOperand *op, CARD32 value)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    Store4(line,offset,value>>28);
}

void
fbStore_r1g2b1 (FbCompositeOperand *op, CARD32 value)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD32  pixel;
    
    Split(value);
    pixel = (((r >> 4) & 0x8) |
	     ((g >> 5) & 0x6) |
	     ((b >> 7)      ));
    Store4(line,offset,pixel);
}

void
fbStore_b1g2r1 (FbCompositeOperand *op, CARD32 value)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD32  pixel;
    
    Split(value);
    pixel = (((b >> 4) & 0x8) |
	     ((g >> 5) & 0x6) |
	     ((r >> 7)      ));
    Store4(line,offset,pixel);
}

void
fbStore_a1r1g1b1 (FbCompositeOperand *op, CARD32 value)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD32  pixel;
    Splita(value);
    pixel = (((a >> 4) & 0x8) |
	     ((r >> 5) & 0x4) |
	     ((g >> 6) & 0x2) |
	     ((b >> 7)      ));
    Store4(line,offset,pixel);
}

void
fbStore_a1b1g1r1 (FbCompositeOperand *op, CARD32 value)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD32  pixel;
    Splita(value);
    pixel = (((a >> 4) & 0x8) |
	     ((b >> 5) & 0x4) |
	     ((g >> 6) & 0x2) |
	     ((r >> 7)      ));
    Store4(line,offset,pixel);
}

void
fbStore_c4 (FbCompositeOperand *op, CARD32 value)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD32  pixel;
    
    pixel = miIndexToEnt24(op->indexed,value);
    Store4(line,offset,pixel);
}

void
fbStore_g4 (FbCompositeOperand *op, CARD32 value)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD32  pixel;
    
    pixel = miIndexToEntY24(op->indexed,value);
    Store4(line,offset,pixel);
}

void
fbStore_a1 (FbCompositeOperand *op, CARD32 value)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD32  *pixel = ((CARD32 *) line) + (offset >> 5);
    CARD32  mask = FbStipMask(offset & 0x1f, 1);

    value = value & 0x80000000 ? mask : 0;
    *pixel = (*pixel & ~mask) | value;
}

void
fbStore_g1 (FbCompositeOperand *op, CARD32 value)
{
    FbBits  *line = op->line; CARD32 offset = op->offset;
    CARD32  *pixel = ((CARD32 *) line) + (offset >> 5);
    CARD32  mask = FbStipMask(offset & 0x1f, 1);

    value = miIndexToEntY24(op->indexed,value) ? mask : 0;
    *pixel = (*pixel & ~mask) | value;
}

CARD32
fbFetch_external (FbCompositeOperand *op)
{
    CARD32  rgb = (*op[1].fetch) (&op[1]);
    CARD32  a = (*op[2].fetch) (&op[2]);

    return (rgb & 0xffffff) | (a & 0xff000000);
}


CARD32
fbFetcha_external (FbCompositeOperand *op)
{
    return (*op[2].fetch) (&op[2]);
}

void
fbStore_external (FbCompositeOperand *op, CARD32 value)
{
    (*op[1].store) (&op[1], value | 0xff000000);
    (*op[2].store) (&op[2], value & 0xff000000);
}

FbAccessMap fbAccessMap[] = {
    /* 32bpp formats */
    { PICT_a8r8g8b8,	fbFetch_a8r8g8b8,	fbFetch_a8r8g8b8,	fbStore_a8r8g8b8 },
    { PICT_x8r8g8b8,	fbFetch_x8r8g8b8,	fbFetch_x8r8g8b8,	fbStore_x8r8g8b8 },
    { PICT_a8b8g8r8,	fbFetch_a8b8g8r8,	fbFetch_a8b8g8r8,	fbStore_a8b8g8r8 },
    { PICT_x8b8g8r8,	fbFetch_x8b8g8r8,	fbFetch_x8b8g8r8,	fbStore_x8b8g8r8 },

    /* 24bpp formats */
    { PICT_r8g8b8,	fbFetch_r8g8b8,		fbFetch_r8g8b8,		fbStore_r8g8b8 },
    { PICT_b8g8r8,	fbFetch_b8g8r8,		fbFetch_b8g8r8,		fbStore_b8g8r8 },

    /* 16bpp formats */
    { PICT_r5g6b5,	fbFetch_r5g6b5,		fbFetch_r5g6b5,		fbStore_r5g6b5 },
    { PICT_b5g6r5,	fbFetch_b5g6r5,		fbFetch_b5g6r5,		fbStore_b5g6r5 },

    { PICT_a1r5g5b5,	fbFetch_a1r5g5b5,	fbFetch_a1r5g5b5,	fbStore_a1r5g5b5 },
    { PICT_x1r5g5b5,	fbFetch_x1r5g5b5,	fbFetch_x1r5g5b5,	fbStore_x1r5g5b5 },
    { PICT_a1b5g5r5,	fbFetch_a1b5g5r5,	fbFetch_a1b5g5r5,	fbStore_a1b5g5r5 },
    { PICT_x1b5g5r5,	fbFetch_x1b5g5r5,	fbFetch_x1b5g5r5,	fbStore_x1b5g5r5 },

    /* 8bpp formats */
    { PICT_a8,		fbFetch_a8,		fbFetcha_a8,		fbStore_a8 },
    { PICT_r3g3b2,	fbFetch_r3g3b2,		fbFetch_r3g3b2,		fbStore_r3g3b2 },
    { PICT_b2g3r3,	fbFetch_b2g3r3,		fbFetch_b2g3r3,		fbStore_b2g3r3 },
    { PICT_a2r2g2b2,	fbFetch_a2r2g2b2,	fbFetch_a2r2g2b2,	fbStore_a2r2g2b2 },
    { PICT_c8,		fbFetch_c8,		fbFetch_c8,		fbStore_c8 },
    { PICT_g8,		fbFetch_c8,		fbFetch_c8,		fbStore_g8 },

    /* 4bpp formats */
    { PICT_a4,		fbFetch_a4,		fbFetcha_a4,		fbStore_a4 },
    { PICT_r1g2b1,	fbFetch_r1g2b1,		fbFetch_r1g2b1,		fbStore_r1g2b1 },
    { PICT_b1g2r1,	fbFetch_b1g2r1,		fbFetch_b1g2r1,		fbStore_b1g2r1 },
    { PICT_a1r1g1b1,	fbFetch_a1r1g1b1,	fbFetch_a1r1g1b1,	fbStore_a1r1g1b1 },
    { PICT_a1b1g1r1,	fbFetch_a1b1g1r1,	fbFetch_a1b1g1r1,	fbStore_a1b1g1r1 },
    { PICT_c4,		fbFetch_c4,		fbFetch_c4,		fbStore_c4 },
    { PICT_g4,		fbFetch_c4,		fbFetch_c4,		fbStore_g4 },

    /* 1bpp formats */
    { PICT_a1,		fbFetch_a1,		fbFetcha_a1,		fbStore_a1 },
    { PICT_g1,		fbFetch_g1,		fbFetch_g1,		fbStore_g1 },
};
#define NumAccessMap (sizeof fbAccessMap / sizeof fbAccessMap[0])

Bool
fbBuildOneCompositeOperand (PicturePtr		pPict,
			    FbCompositeOperand	*op,
			    INT16		x,
			    INT16		y)
{
    int	    i;
    
    for (i = 0; i < NumAccessMap; i++)
	if (fbAccessMap[i].format == pPict->format)
	{
	    op->fetch = fbAccessMap[i].fetch;
	    op->fetcha = fbAccessMap[i].fetcha;
	    op->store = fbAccessMap[i].store;
	    op->indexed = (miIndexedPtr) pPict->pFormat->indexed;
	    fbGetDrawable (pPict->pDrawable, op->line, op->stride, op->bpp,
			   op->xoff, op->yoff);
	    if (pPict->repeat && pPict->pDrawable->width == 1 && 
		pPict->pDrawable->height == 1)
	    {
		op->bpp = 0;
		op->stride = 0;
	    }
	    op->line = op->line + (y + op->yoff) * op->stride;
	    op->offset = 0;
	    return TRUE;
	}
    return FALSE;
}

Bool
fbBuildCompositeOperand (PicturePtr	    pPict,
			 FbCompositeOperand op[3],
			 INT16		    x,
			 INT16		    y)
{
    if (pPict->alphaMap)
    {
	if (!fbBuildOneCompositeOperand (pPict, &op[1], x, y))
	    return FALSE;
	if (!fbBuildOneCompositeOperand (pPict->alphaMap, &op[2],
					 x - pPict->alphaOrigin.x,
					 y - pPict->alphaOrigin.y))
	    return FALSE;
	op[0].fetch = fbFetch_external;
	op[0].fetcha = fbFetcha_external;
	op[0].store = fbStore_external;
	op[2].xoff -= pPict->alphaOrigin.x;
	op[2].yoff -= pPict->alphaOrigin.y;
	return TRUE;
    }
    else
    {
	return fbBuildOneCompositeOperand (pPict, &op[0], x, y);
    }
}

void
fbCompositeGeneral (CARD8	op,
		    PicturePtr	pSrc,
		    PicturePtr	pMask,
		    PicturePtr	pDst,
		    INT16	xSrc,
		    INT16	ySrc,
		    INT16	xMask,
		    INT16	yMask,
		    INT16	xDst,
		    INT16	yDst,
		    CARD16	width,
		    CARD16	height)
{
    FbCompositeOperand	src[3],msk[3],dst[3],*pmsk;
    FbCompositeOperand	*srcPict, *srcAlpha;
    FbCompositeOperand	*dstPict, *dstAlpha;
    FbCompositeOperand	*mskPict = 0, *mskAlpha = 0;
    FbCombineFunc	f;
    int			w;

    if (!fbBuildCompositeOperand (pSrc, src, xSrc, ySrc))
	return;
    if (!fbBuildCompositeOperand (pDst, dst, xDst, yDst))
	return;
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
	if (!fbBuildCompositeOperand (pMask, msk, xMask, yMask))
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
	
        srcPict->offset = (xSrc + srcPict->xoff) * srcPict->bpp;
	if (srcAlpha)
	    srcAlpha->offset = (xSrc + srcAlpha->xoff) * srcAlpha->bpp;
	
        dstPict->offset = (xDst + dstPict->xoff) * dstPict->bpp;
	if (dstAlpha)
	    dstAlpha->offset = (xDst + dstAlpha->xoff) * dstAlpha->bpp;

	if (pmsk)
	{
	    mskPict->offset = (xMask + mskPict->xoff) * mskPict->bpp;
	    if (mskAlpha)
		mskAlpha->offset = (xMask + mskAlpha->xoff) * mskAlpha->bpp;
	}
	while (w--)
	{
	    (*f) (src, pmsk, dst);
	    srcPict->offset += srcPict->bpp;
	    if (srcAlpha)
		srcAlpha->offset += srcAlpha->bpp;
	    dstPict->offset += dstPict->bpp;
	    if (dstAlpha)
		dstAlpha->offset += dstAlpha->bpp;
	    if (pmsk)
	    {
		mskPict->offset += mskPict->bpp;
		if (mskAlpha)
		    mskAlpha->offset += mskAlpha->bpp;
	    }
	}
	srcPict->line += srcPict->stride;
	if (srcAlpha)
	    srcAlpha->line += srcAlpha->stride;
	
	dstPict->line += dstPict->stride;
	if (dstAlpha)
	    dstAlpha->line += dstAlpha->stride;
	if (pmsk)
	{
	    mskPict->line += mskPict->stride;
	    if (mskAlpha)
		mskAlpha->line += mskAlpha->stride;
	}
    }
}
