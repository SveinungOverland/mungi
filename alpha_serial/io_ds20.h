/****************************************************************************
 *
 *      $Id: io_ds20.h,v 1.2 2002/05/31 05:00:48 danielp Exp $
 *      Copyright (C) 2002 Operating Systems Research Group, UNSW, Australia.
 *
 *      This file is part of the Mungi operating system distribution.
 *
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *	version 2 as published by the Free Software Foundation.
 *	A copy of this license is included in the top level directory of 
 *	the Mungi distribution.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 ****************************************************************************/

#ifndef __ALPHA_TSUNAMI__H__
#define __ALPHA_TSUNAMI__H__

#include "compiler.h"

/*
 * TSUNAMI/TYPHOON are the internal names for the core logic chipset which
 * provides memory controller and PCI access for the 21264 based systems.
 *
 * This file is based on:
 *
 * Tsunami System Programmers Manual
 * Preliminary, Chapters 2-5
 *
 */

/* FIXME (sjw Wed Aug 23 22:27:32 2000 ) --- Alpha specific ... remove into somewhere more generic */
#define mb() \
__asm__ __volatile__("mb": : :"memory")

#define rmb() \
__asm__ __volatile__("mb": : :"memory")

#define wmb() \
__asm__ __volatile__("wmb": : :"memory")

/* XXX: Do we need to conditionalize on this?  */
#ifdef USE_48_BIT_KSEG
#define TS_BIAS 0x80000000000UL
#else
#define TS_BIAS 0x10000000000UL
#endif

#define IDENT_ADDR 0

/*
 * Memory spaces:
 */
#define TSUNAMI_HOSE(h)		(((unsigned long)(h)) << 33)
#define TSUNAMI_BASE		(IDENT_ADDR + TS_BIAS)

#define TSUNAMI_MEM(h)		(TSUNAMI_BASE+TSUNAMI_HOSE(h) + 0x000000000UL)
#define _TSUNAMI_IACK_SC(h)	(TSUNAMI_BASE+TSUNAMI_HOSE(h) + 0x1F8000000UL)
#define TSUNAMI_IO(h)		(TSUNAMI_BASE+TSUNAMI_HOSE(h) + 0x1FC000000UL)
#define TSUNAMI_CONF(h)		(TSUNAMI_BASE+TSUNAMI_HOSE(h) + 0x1FE000000UL)

#define TSUNAMI_IACK_SC		_TSUNAMI_IACK_SC(0) /* hack! */


/* 
 * The canonical non-remaped I/O and MEM addresses have these values
 * subtracted out.  This is arranged so that folks manipulating ISA
 * devices can use their familiar numbers and have them map to bus 0.
 */

#define TSUNAMI_IO_BIAS          TSUNAMI_IO(0)
#define TSUNAMI_MEM_BIAS         TSUNAMI_MEM(0)

/* The IO address space is larger than 0xffff */
#define TSUNAMI_IO_SPACE	(TSUNAMI_CONF(0) - TSUNAMI_IO(0))


#ifndef __EXTERN_INLINE
#define __EXTERN_INLINE static
#define __IO_EXTERN_INLINE
#endif

/*
 * I/O functions:
 *
 * TSUNAMI, the 21??? PCI/memory support chipset for the EV6 (21264)
 * can only use linear accesses to get at PCI memory and I/O spaces.
 */

#define vucp	volatile unsigned char *
#define vusp	volatile unsigned short *
#define vuip	volatile unsigned int *
#define vulp	volatile unsigned long *

__EXTERN_INLINE unsigned int tsunami_inb(unsigned long addr)
{
	/* ??? I wish I could get rid of this.  But there's no ioremap
	   equivalent for I/O space.  PCI I/O can be forced into the
	   correct hose's I/O region, but that doesn't take care of
	   legacy ISA crap.  */

	addr += TSUNAMI_IO_BIAS;
	return __kernel_ldbu(*(vucp)addr);
}

__EXTERN_INLINE void tsunami_outb(unsigned char b, unsigned long addr)
{
	addr += TSUNAMI_IO_BIAS;
	__kernel_stb(b, *(vucp)addr);
	mb();
}

__EXTERN_INLINE unsigned int tsunami_inw(unsigned long addr)
{
	addr += TSUNAMI_IO_BIAS;
	return __kernel_ldwu(*(vusp)addr);
}

__EXTERN_INLINE void tsunami_outw(unsigned short b, unsigned long addr)
{
	addr += TSUNAMI_IO_BIAS;
	__kernel_stw(b, *(vusp)addr);
	mb();
}

__EXTERN_INLINE unsigned int tsunami_inl(unsigned long addr)
{
	addr += TSUNAMI_IO_BIAS;
	return *(vuip)addr;
}

__EXTERN_INLINE void tsunami_outl(unsigned int b, unsigned long addr)
{
	addr += TSUNAMI_IO_BIAS;
	*(vuip)addr = b;
	mb();
}

/*
 * Memory functions.  all accesses are done through linear space.
 */

__EXTERN_INLINE unsigned long tsunami_ioremap(unsigned long addr)
{
	return addr + TSUNAMI_MEM_BIAS;
}

__EXTERN_INLINE int tsunami_is_ioaddr(unsigned long addr)
{
	return addr >= TSUNAMI_BASE;
}

__EXTERN_INLINE unsigned long tsunami_readb(unsigned long addr)
{
	return __kernel_ldbu(*(vucp)addr);
}

__EXTERN_INLINE unsigned long tsunami_readw(unsigned long addr)
{
	return __kernel_ldwu(*(vusp)addr);
}

__EXTERN_INLINE unsigned long tsunami_readl(unsigned long addr)
{
	return *(vuip)addr;
}

__EXTERN_INLINE unsigned long tsunami_readq(unsigned long addr)
{
	return *(vulp)addr;
}

__EXTERN_INLINE void tsunami_writeb(unsigned char b, unsigned long addr)
{
	__kernel_stb(b, *(vucp)addr);
}

__EXTERN_INLINE void tsunami_writew(unsigned short b, unsigned long addr)
{
	__kernel_stw(b, *(vusp)addr);
}

__EXTERN_INLINE void tsunami_writel(unsigned int b, unsigned long addr)
{
	*(vuip)addr = b;
}

__EXTERN_INLINE void tsunami_writeq(unsigned long b, unsigned long addr)
{
	*(vulp)addr = b;
}

#undef vucp
#undef vusp
#undef vuip
#undef vulp

#define __inb(p)		tsunami_inb((unsigned long)(p))
#define __inw(p)		tsunami_inw((unsigned long)(p))
#define __inl(p)		tsunami_inl((unsigned long)(p))
#define __outb(x,p)		tsunami_outb((x),(unsigned long)(p))
#define __outw(x,p)		tsunami_outw((x),(unsigned long)(p))
#define __outl(x,p)		tsunami_outl((x),(unsigned long)(p))
#define __readb(a)		tsunami_readb((unsigned long)(a))
#define __readw(a)		tsunami_readw((unsigned long)(a))
#define __readl(a)		tsunami_readl((unsigned long)(a))
#define __readq(a)		tsunami_readq((unsigned long)(a))
#define __writeb(x,a)		tsunami_writeb((x),(unsigned long)(a))
#define __writew(x,a)		tsunami_writew((x),(unsigned long)(a))
#define __writel(x,a)		tsunami_writel((x),(unsigned long)(a))
#define __writeq(x,a)		tsunami_writeq((x),(unsigned long)(a))
#define __ioremap(a)		tsunami_ioremap((unsigned long)(a))
#define __is_ioaddr(a)		tsunami_is_ioaddr((unsigned long)(a))

#define inb(p)			__inb(p)
#define inw(p)			__inw(p)
#define inl(p)			__inl(p)
#define outb(x,p)		__outb((x),(p))
#define outw(x,p)		__outw((x),(p))
#define outl(x,p)		__outl((x),(p))
#define __raw_readb(a)		__readb(a)
#define __raw_readw(a)		__readw(a)
#define __raw_readl(a)		__readl(a)
#define __raw_readq(a)		__readq(a)
#define __raw_writeb(v,a)	__writeb((v),(a))
#define __raw_writew(v,a)	__writew((v),(a))
#define __raw_writel(v,a)	__writel((v),(a))
#define __raw_writeq(v,a)	__writeq((v),(a))

#ifdef __IO_EXTERN_INLINE
#undef __EXTERN_INLINE
#undef __IO_EXTERN_INLINE
#endif

#endif /* __ALPHA_TSUNAMI__H__ */
