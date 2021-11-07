/****************************************************************************
 *
 *      $Id: io_multia.h,v 1.2 2002/05/31 05:00:49 danielp Exp $
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

/*
 *        Project:  L4/Alpha serial driver.
 *        Created:  23/08/2000 22:09:02 by Simon Winwood (sjw)
 *  Last Modified:  06/12/2000 11:31:26 by Simon Winwood (sjw)
 *   Version info:  $Revision: 1.2 $ 
 *    Description:
 *         Generic IO routines
 *       Comments:
 *         - Copied straight from asm-alpha/core_lca.h
 *
 *         This file should probably be a symlink to 
 *     io_lca.h, io_ruffian.h etc, ... later ;)
 *
 * $Log: io_multia.h,v $
 * Revision 1.2  2002/05/31 05:00:49  danielp
 * License header. GPL.
 *
 * Revision 1.1  2002/02/05 03:47:35  brettn
 * Mammoth update:
 * Added pager mappings for MIPS clock into pager
 * Removed dodgy PRINT macro in kernel/src/clock.c
 * Split user and kernel clocks.
 *                                                 (from andrewb)
 * Added multiple user task support (Makefile and subdir.mk)
 * Moved init.c
 * Console driver update/rewrite for alpha.
 * Fix type of pdx_t function
 * Removed \n => \n\r
 * Fixed upcall UserPrint for mips and alpha
 * Add handling for 'mishell' and 'init' instead of mungiapp
 * Added more capabilities for init.
 * Death of aprintf, moved to kprintf.
 * Add character input to startup.
 * Fstat removed from mungi and added to POSIX.
 * Added new apps and mishell.
 * Added alpha serial driver, posix compatibility library and mlib.
 *                                                 (from alexs and cgray)
 * Fixed allocation bug in rpager (will not alloc kernel memory now)
 * Misc rpager cleanups - more needed.
 *                                                 (benno, cgray & me)
 *
 * Revision 1.1  2001/03/27 12:10:35  sjw
 * Added some existing code
 *
 */

#ifndef IO_MULTIA_H
#define IO_MULTIA_H

#include "compiler.h"

/* FIXME (sjw Wed Aug 23 22:27:32 2000 ) --- Alpha specific ... remove into somewhere more generic */
#define mb() \
__asm__ __volatile__("mb": : :"memory")

#define rmb() \
__asm__ __volatile__("mb": : :"memory")

#define wmb() \
__asm__ __volatile__("wmb": : :"memory")


/*
 * Low Cost Alpha (LCA) definitions (these apply to 21066 and 21068,
 * for example).
 *
 * This file is based on:
 *
 *	DECchip 21066 and DECchip 21068 Alpha AXP Microprocessors
 *	Hardware Reference Manual; Digital Equipment Corp.; May 1994;
 *	Maynard, MA; Order Number: EC-N2681-71.
 */

/*
 * NOTE: The LCA uses a Host Address Extension (HAE) register to access
 *	 PCI addresses that are beyond the first 27 bits of address
 *	 space.  Updating the HAE requires an external cycle (and
 *	 a memory barrier), which tends to be slow.  Instead of updating
 *	 it on each sparse memory access, we keep the current HAE value
 *	 cached in variable cache_hae.  Only if the cached HAE differs
 *	 from the desired HAE value do we actually updated HAE register.
 *	 The HAE register is preserved by the interrupt handler entry/exit
 *	 code, so this scheme works even in the presence of interrupts.
 *
 * Dense memory space doesn't require the HAE, but is restricted to
 * aligned 32 and 64 bit accesses.  Special Cycle and Interrupt
 * Acknowledge cycles may also require the use of the HAE.  The LCA
 * limits I/O address space to the bottom 24 bits of address space,
 * but this easily covers the 16 bit ISA I/O address space.
 */

/*
 * NOTE 2! The memory operations do not set any memory barriers, as
 * it's not needed for cases like a frame buffer that is essentially
 * memory-like.  You need to do them by hand if the operations depend
 * on ordering.
 *
 * Similarly, the port I/O operations do a "mb" only after a write
 * operation: if an mb is needed before (as in the case of doing
 * memory mapped I/O first, and then a port I/O operation to the same
 * device), it needs to be done by hand.
 *
 * After the above has bitten me 100 times, I'll give up and just do
 * the mb all the time, but right now I'm hoping this will work out.
 * Avoiding mb's may potentially be a noticeable speed improvement,
 * but I can't honestly say I've tested it.
 *
 * Handling interrupts that need to do mb's to synchronize to
 * non-interrupts is another fun race area.  Don't do it (because if
 * you do, I'll have to do *everything* with interrupts disabled,
 * ugh).
 */

/*
 * Memory Controller registers:
 */
#define LCA_MEM_BCR0		(0x120000000UL)
#define LCA_MEM_BCR1		(0x120000008UL)
#define LCA_MEM_BCR2		(0x120000010UL)
#define LCA_MEM_BCR3		(0x120000018UL)
#define LCA_MEM_BMR0		(0x120000020UL)
#define LCA_MEM_BMR1		(0x120000028UL)
#define LCA_MEM_BMR2		(0x120000030UL)
#define LCA_MEM_BMR3		(0x120000038UL)
#define LCA_MEM_BTR0		(0x120000040UL)
#define LCA_MEM_BTR1		(0x120000048UL)
#define LCA_MEM_BTR2		(0x120000050UL)
#define LCA_MEM_BTR3		(0x120000058UL)
#define LCA_MEM_GTR		(0x120000060UL)
#define LCA_MEM_ESR		(0x120000068UL)
#define LCA_MEM_EAR		(0x120000070UL)
#define LCA_MEM_CAR		(0x120000078UL)
#define LCA_MEM_VGR		(0x120000080UL)
#define LCA_MEM_PLM		(0x120000088UL)
#define LCA_MEM_FOR		(0x120000090UL)

/*
 * I/O Controller registers:
 */
#define LCA_IOC_HAE		(0x180000000UL)
#define LCA_IOC_CONF		(0x180000020UL)
#define LCA_IOC_STAT0		(0x180000040UL)
#define LCA_IOC_STAT1		(0x180000060UL)
#define LCA_IOC_TBIA		(0x180000080UL)
#define LCA_IOC_TB_ENA		(0x1800000a0UL)
#define LCA_IOC_SFT_RST		(0x1800000c0UL)
#define LCA_IOC_PAR_DIS		(0x1800000e0UL)
#define LCA_IOC_W_BASE0		(0x180000100UL)
#define LCA_IOC_W_BASE1		(0x180000120UL)
#define LCA_IOC_W_MASK0		(0x180000140UL)
#define LCA_IOC_W_MASK1		(0x180000160UL)
#define LCA_IOC_T_BASE0		(0x180000180UL)
#define LCA_IOC_T_BASE1		(0x1800001a0UL)
#define LCA_IOC_TB_TAG0		(0x188000000UL)
#define LCA_IOC_TB_TAG1		(0x188000020UL)
#define LCA_IOC_TB_TAG2		(0x188000040UL)
#define LCA_IOC_TB_TAG3		(0x188000060UL)
#define LCA_IOC_TB_TAG4		(0x188000070UL)
#define LCA_IOC_TB_TAG5		(0x1880000a0UL)
#define LCA_IOC_TB_TAG6		(0x1880000c0UL)
#define LCA_IOC_TB_TAG7		(0x1880000e0UL)

/*
 * Memory spaces:
 */
#define LCA_IACK_SC		(0x1a0000000UL)
#define LCA_CONF		(0x1e0000000UL)
#define LCA_IO			(0x1c0000000UL)
#define LCA_SPARSE_MEM		(0x200000000UL)
#define LCA_DENSE_MEM		(0x300000000UL)

/*
 * Bit definitions for I/O Controller status register 0:
 */
#define LCA_IOC_STAT0_CMD		0xf
#define LCA_IOC_STAT0_ERR		(1<<4)
#define LCA_IOC_STAT0_LOST		(1<<5)
#define LCA_IOC_STAT0_THIT		(1<<6)
#define LCA_IOC_STAT0_TREF		(1<<7)
#define LCA_IOC_STAT0_CODE_SHIFT	8
#define LCA_IOC_STAT0_CODE_MASK		0x7
#define LCA_IOC_STAT0_P_NBR_SHIFT	13
#define LCA_IOC_STAT0_P_NBR_MASK	0x7ffff

#define LCA_HAE_ADDRESS		LCA_IOC_HAE

/* LCA PMR Power Management register defines */
#define LCA_PMR_ADDR	(0x120000098UL)
#define LCA_PMR_PDIV    0x7                     /* Primary clock divisor */
#define LCA_PMR_ODIV    0x38                    /* Override clock divisor */
#define LCA_PMR_INTO    0x40                    /* Interrupt override */
#define LCA_PMR_DMAO    0x80                    /* DMA override */
#define LCA_PMR_OCCEB   0xffff0000L             /* Override cycle counter - even bits */
#define LCA_PMR_OCCOB   0xffff000000000000L     /* Override cycle counter - even bits */
#define LCA_PMR_PRIMARY_MASK    0xfffffffffffffff8

/* LCA PMR Macros */

#define LCA_READ_PMR        (*(volatile unsigned long *)LCA_PMR_ADDR)
#define LCA_WRITE_PMR(d)    (*((volatile unsigned long *)LCA_PMR_ADDR) = (d))

#define LCA_GET_PRIMARY(r)  ((r) & LCA_PMR_PDIV)
#define LCA_GET_OVERRIDE(r) (((r) >> 3) & LCA_PMR_PDIV)
#define LCA_SET_PRIMARY_CLOCK(r, c) ((r) = (((r) & LCA_PMR_PRIMARY_MASK)|(c)))

/* LCA PMR Divisor values */
#define LCA_PMR_DIV_1   0x0
#define LCA_PMR_DIV_1_5 0x1
#define LCA_PMR_DIV_2   0x2
#define LCA_PMR_DIV_4   0x3
#define LCA_PMR_DIV_8   0x4
#define LCA_PMR_DIV_16  0x5
#define LCA_PMR_DIV_MIN DIV_1
#define LCA_PMR_DIV_MAX DIV_16


#ifndef __EXTERN_INLINE
//#define __EXTERN_INLINE extern inline
#define __EXTERN_INLINE static inline
#define __IO_EXTERN_INLINE
#endif

/*
 * I/O functions:
 *
 * Unlike Jensen, the Noname machines have no concept of local
 * I/O---everything goes over the PCI bus.
 *
 * There is plenty room for optimization here.  In particular,
 * the Alpha's insb/insw/extb/extw should be useful in moving
 * data to/from the right byte-lanes.
 */

#define vip	volatile int *
#define vuip	volatile unsigned int *
#define vulp	volatile unsigned long *

__EXTERN_INLINE void lca_set_hae(unsigned long new_hae)
{
	*(unsigned long *) LCA_IOC_HAE = new_hae;
	mb();
	/* seedy ;) */
	new_hae = *(unsigned long *) LCA_IOC_HAE;
}

#define set_hae(a)      lca_set_hae(a)

__EXTERN_INLINE unsigned int lca_inb(unsigned long addr)
{
	long result = *(vip) ((addr << 5) + LCA_IO + 0x00);
	return __kernel_extbl(result, addr & 3);
}

__EXTERN_INLINE void lca_outb(unsigned char b, unsigned long addr)
{
	unsigned long w;

	w = __kernel_insbl(b, addr & 3);
	*(vuip) ((addr << 5) + LCA_IO + 0x00) = w;
	mb();
}

__EXTERN_INLINE unsigned int lca_inw(unsigned long addr)
{
	long result = *(vip) ((addr << 5) + LCA_IO + 0x08);
	return __kernel_extwl(result, addr & 3);
}

__EXTERN_INLINE void lca_outw(unsigned short b, unsigned long addr)
{
	unsigned long w;

	w = __kernel_inswl(b, addr & 3);
	*(vuip) ((addr << 5) + LCA_IO + 0x08) = w;
	mb();
}

__EXTERN_INLINE unsigned int lca_inl(unsigned long addr)
{
	return *(vuip) ((addr << 5) + LCA_IO + 0x18);
}

__EXTERN_INLINE void lca_outl(unsigned int b, unsigned long addr)
{
	*(vuip) ((addr << 5) + LCA_IO + 0x18) = b;
	mb();
}


/*
 * Memory functions.  64-bit and 32-bit accesses are done through
 * dense memory space, everything else through sparse space.
 */

__EXTERN_INLINE unsigned long lca_readb(unsigned long addr)
{
	unsigned long result, msb;

	addr -= LCA_DENSE_MEM;
	if (addr >= (1UL << 24)) {
		msb = addr & 0xf8000000;
		addr -= msb;
		set_hae(msb);
	}
	result = *(vip) ((addr << 5) + LCA_SPARSE_MEM + 0x00);
	return __kernel_extbl(result, addr & 3);
}

__EXTERN_INLINE unsigned long lca_readw(unsigned long addr)
{
	unsigned long result, msb;

	addr -= LCA_DENSE_MEM;
	if (addr >= (1UL << 24)) {
		msb = addr & 0xf8000000;
		addr -= msb;
		set_hae(msb);
	}
	result = *(vip) ((addr << 5) + LCA_SPARSE_MEM + 0x08);
	return __kernel_extwl(result, addr & 3);
}

__EXTERN_INLINE unsigned long lca_readl(unsigned long addr)
{
	return *(vuip)addr;
}

__EXTERN_INLINE unsigned long lca_readq(unsigned long addr)
{
	return *(vulp)addr;
}

__EXTERN_INLINE void lca_writeb(unsigned char b, unsigned long addr)
{
	unsigned long msb;
	unsigned long w;

	addr -= LCA_DENSE_MEM;
	if (addr >= (1UL << 24)) {
		msb = addr & 0xf8000000;
		addr -= msb;
		set_hae(msb);
	}
	w = __kernel_insbl(b, addr & 3);
	*(vuip) ((addr << 5) + LCA_SPARSE_MEM + 0x00) = w;
}

__EXTERN_INLINE void lca_writew(unsigned short b, unsigned long addr)
{
	unsigned long msb;
	unsigned long w;

	addr -= LCA_DENSE_MEM;
	if (addr >= (1UL << 24)) {
		msb = addr & 0xf8000000;
		addr -= msb;
		set_hae(msb);
	}
	w = __kernel_inswl(b, addr & 3);
	*(vuip) ((addr << 5) + LCA_SPARSE_MEM + 0x08) = w;
}

__EXTERN_INLINE void lca_writel(unsigned int b, unsigned long addr)
{
	*(vuip)addr = b;
}

__EXTERN_INLINE void lca_writeq(unsigned long b, unsigned long addr)
{
	*(vulp)addr = b;
}

__EXTERN_INLINE unsigned long lca_ioremap(unsigned long addr)
{
	return addr + LCA_DENSE_MEM;
}

__EXTERN_INLINE int lca_is_ioaddr(unsigned long addr)
{
	return addr >= 0x120000000UL;
}

#define inb(p)		lca_inb((unsigned long)(p))
#define inw(p)		lca_inw((unsigned long)(p))
#define inl(p)		lca_inl((unsigned long)(p))
#define outb(x,p)	lca_outb((x),(unsigned long)(p))
#define outw(x,p)	lca_outw((x),(unsigned long)(p))
#define outl(x,p)	lca_outl((x),(unsigned long)(p))
#define readb(a)	lca_readb((unsigned long)(a))
#define readw(a)	lca_readw((unsigned long)(a))
#define readl(a)	lca_readl((unsigned long)(a))
#define readq(a)	lca_readq((unsigned long)(a))
#define writeb(x,a)	lca_writeb((x),(unsigned long)(a))
#define writew(x,a)	lca_writew((x),(unsigned long)(a))
#define writel(x,a)	lca_writel((x),(unsigned long)(a))
#define writeq(x,a)	lca_writeq((x),(unsigned long)(a))
#define ioremap(a)	lca_ioremap((unsigned long)(a))
#define is_ioaddr(a)	lca_is_ioaddr((unsigned long)(a))

#define raw_readl(a)	readl(a)
#define raw_readq(a)	readq(a)
#define raw_writel(v,a)	writel((v),(a))
#define raw_writeq(v,a)	writeq((v),(a))

#endif  /* IO_MULTIA_H */
