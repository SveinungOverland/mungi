/****************************************************************************
 *
 *      $Id: elf_reloc.c,v 1.5 2002/07/22 10:17:54 cgray Exp $
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

/* CEG's testing code */

#include <mungi.h>
#include <mungi/l4_generic.h>
#include <mungi/syscallbits.h>
#include <mungi/stdarg.h>
#include <mungi/klibc.h>
#include <mungi/stdarg.h>
#include <mungi/stdarg.h>
#include <mungi/stdarg.h>

/* kinda dodgey :) */
#ifdef ALPHAENV
#include <l4/dit.h>
#else 
#include <kernel/dit.h>
#endif

/* for all the elf structs */
#include "libelf.h"

void test_dit( void* param );
void test_ceg( void* param );

void printf( char*, ... );

// #define ELF_DEBUG 1
#ifdef ELF_DEBUG
#  define elf_debug( x... ) printf(x)
#else
#  define elf_debug( x... ) ((void)0)
#endif

cap_t elf_load(void* elf_file);

/************* CAP functions ************************/

static void
local_clist_add_cap(cap_t cap)
{
	static clist_t *clist = NULL;

	if (!clist) {
		apddesc_t myapd;

		if (ApdGet(&myapd))
			assert(0);

		clist = (clist_t *)myapd.clist[0].address;
	}
	clist->caps[clist->n_caps++] = cap;
}


/************* Mini Dynamic Relocater *****************/

Elf64_Phdr_t *
getProgramSegmentTable (void *elfFile)
/* Returns a pointer to the program segment table, which is an array of
 * ELF64_Phdr_t structs.  The size of the array can be found by calling
 * getNumProgramSegments.
 */
{
	Elf64_Header_t *fileHdr = (Elf64_Header_t *) elfFile;
	return (Elf64_Phdr_t *)(fileHdr->e_phoff + (long)elfFile);
}

unsigned
getNumProgramSegments (void *elfFile)
/* Returns the number of program segments in this elf file.
 */
{
	Elf64_Header_t *fileHdr = (Elf64_Header_t *) elfFile;
	return fileHdr->e_phnum;
}


static Elf64_Phdr_t *
ElfGetSeg( void *elf, int segnum )
{
	Elf64_Phdr_t * pseg;	
	Elf64_Header_t *phdr;

	phdr = elf;
	pseg = getProgramSegmentTable( elf );
	return &pseg[ segnum ];

}

static uintptr_t
ElfGetMemSize( void *elf )
{
	Elf64_Phdr_t *pseg0, *psege;
	uintptr_t bot, top;
	int n;

	n = getNumProgramSegments( elf );
	pseg0 = ElfGetSeg( elf, 0 );
	psege = ElfGetSeg( elf, n - 1 );

	bot = pseg0->p_vaddr;
	top = psege->p_vaddr + psege->p_memsz;

	return top - bot;
}

static uintptr_t
ElfGetSegOffset( void *elf, int seg )
{
	Elf64_Phdr_t * pseg;	
	
	pseg = ElfGetSeg( elf, seg );

	return pseg->p_offset;	
}

static uintptr_t
ElfGetMemOffset( void *elf, int seg )
{
	Elf64_Phdr_t * pseg;	
	
	pseg = ElfGetSeg( elf, 0 );

	return (uintptr_t) pseg[seg].p_vaddr - (uintptr_t) pseg[0].p_vaddr;
}

static uintptr_t
ElfGetFileSize( void *elf, int seg )
{
	Elf64_Phdr_t * pseg;	
	
	pseg = ElfGetSeg( elf, seg );

	return pseg->p_filesz;	
}


static void *
RelocateElf( void *new, void *old )
{
	int num, i, j;
	Elf64_Phdr_t * pseg;	
	Elf64_Header_t *phdr;
	Elf64_Shdr_t *psec;
	uintptr_t  *re;
	char *string_tab;
	uintptr_t diff, re_new;
	uintptr_t old_vaddr_start, old_vaddr_end;

	phdr = old;

	/* work out vaddr_start and vaddr_end */
	old_vaddr_start = ElfGetSeg( old, 0 )->p_vaddr;
	old_vaddr_end   = ElfGetMemSize( old ) + old_vaddr_start;

	psec = (Elf64_Shdr_t*) (phdr->e_shoff + (long) old);
	
	elf_debug( "HDR: %s\n", phdr->e_ident );

	elf_debug( "String index: %d\n", phdr->e_shstrndx );
	elf_debug( "Setting up the string table!\n" );
	i = phdr->e_shstrndx;
	string_tab = ((char*)old) + psec[i].sh_offset;

	elf_debug( "**** SEGMENT INFO *****\n" );
	num = getNumProgramSegments( old );
	elf_debug( "getNumProgramSegments: %d\n", num );
	pseg = getProgramSegmentTable( old );
	for( i = 0; i < num ; i++ )
	{
		elf_debug( "Segment %d\n", i );
		elf_debug( "Type %d\n", pseg[i].p_type );
		elf_debug( "File Offset 0x%llx\n", pseg[i].p_offset );
		elf_debug( "vaddr 0x%llx\n", pseg[i].p_vaddr );
		elf_debug( "file sz 0x%llx\n", pseg[i].p_filesz );
		elf_debug( "mem  sz 0x%llx\n", pseg[i].p_memsz );
	}

	elf_debug( "new address is 0x%llx\n", new );
	elf_debug( "pseg->p_vad is 0x%llx\n", pseg[0].p_vaddr );
/*	diff = ((uintptr_t)new) - ((uintptr_t)pseg[0].p_vaddr); */
	diff = ((uintptr_t)pseg[0].p_vaddr) - ((uintptr_t)new);
	elf_debug( "Computed diff of 0x%llx\n", diff );

	elf_debug( "**** SECTION INFO *****\n" );
	elf_debug( "no. program sections: %d\n", phdr->e_shnum );

	num = phdr->e_shnum;

	/* now hopefully we can look at their names, too! */
	for( i = 0; i < num; i++ )
	{
		elf_debug( "Section name %s\n", &string_tab[psec[i].sh_name] );
		if( strcmp( string_tab + psec[i].sh_name, ".got" ) == 0 )
		{
			/*
			  elf_debug( "Section type %d\n", psec[i].sh_type );
			  elf_debug( "Section offset %d\n", psec[i].sh_offset );
			  elf_debug( "Section size %d\n", psec[i].sh_size );
			*/
			elf_debug( "FOUND GOT!!\n" );
			elf_debug( "Dumping!!\n" );

			/* CHECK THIS!! */
			/*
			re = (uintptr_t *)( psec[i].sh_offset + 
					  (long)new -
					  pseg[0].p_offset
					  );
			*/
			elf_debug( "sh_addr: 0x%llx\n", psec[i].sh_addr );
			elf_debug( "old    : 0x%llx\n", old );
			elf_debug( "new    : 0x%llx\n", new );
			re = (uintptr_t*)( (uintptr_t) psec[i].sh_addr 
					 - (uintptr_t) diff );

			j = 0;
			elf_debug( " re is 0x%llx\n", re );
			elf_debug( "*re is 0x%llx\n", *re );
			while( *re )
			{
				re_new = *re - diff;
				if( j++ < 10 )
					elf_debug( "e: 0x%llx -> 0x%llx\n",
						   *re,
						   re_new );
				*re = re_new;
				re++;
			}
		}

		if( strcmp( string_tab + psec[i].sh_name, ".data" ) == 0 )
		{
			int j2, max;
			elf_debug( "FOUND Data!!\n" );

			elf_debug( "sh_addr: 0x%llx\n", psec[i].sh_addr );
			elf_debug( "old    : 0x%llx\n", old );
			elf_debug( "new    : 0x%llx\n", new );
			re = (uintptr_t *)( (uintptr_t) psec[i].sh_addr 
					 - (uintptr_t) diff );
			max = psec[i].sh_size / sizeof(uintptr_t);
			elf_debug( "data between 0x%llx - 0x%llx\n", 
				   old_vaddr_start, old_vaddr_end );
			elf_debug( "max is %d\n", max );

			elf_debug( " re is 0x%llx\n", re );
			elf_debug( "*re is 0x%llx\n", *re );
			// while( *re ) 
			for (j2 = 0 ; j2 < max; j2++) {
				re_new = *re - diff;

//				elf_debug( "Checking 0x%llx\n", *re );
				if( *re >= old_vaddr_start 
				    && * re <= old_vaddr_end )
				{
					elf_debug( "K: 0x%llx -> 0x%llx\n",
						   *re,
						   re_new );
				
					*re = re_new;
				}
				re++;
			}
		}


	}

	return (void*)(((uintptr_t)phdr->e_entry) -
		       ((uintptr_t)pseg[0].p_vaddr) +
		       new );
/*	return phdr->e_entry; */
}


/*************  DIT relocation & PDX testing ***************/
#if 0
static Dit_Phdr *
find_dit_item( Dit_Dhdr *dit, char* name )
{
	uintptr_t ptr;
	size_t size;
	int i;
  
	ptr = (uintptr_t)dit + (uintptr_t)dit->d_phoff;
	size = (size_t)dit->d_phsize;
	elf_debug("searching dit...\n");
	for (i = 0; i < (int)dit->d_phnum; i++) 
	{
		Dit_Phdr *dit_p = ((Dit_Phdr *)(ptr + (uintptr_t)(i*size)));
		char *p_name = dit_p->p_name;
    
		elf_debug("DIT program %s found.\n", p_name);

		/*    
		      elf_debug("base 0x%lx, entry 0x%lx, size 0x%lx\n",
		      (long unsigned int)dit_p->p_base,
		      (long unsigned int)dit_p->p_entry,
		      (long unsigned int)dit_p->p_size);
		*/    
    
		if (!strncmp(p_name, name, 8)) 
		{
			elf_debug( "Found program!\n" );
			return dit_p;
		}
	}

	elf_debug( "Program not found!\n" );
	return NULL;
}

static void 
test_dit( void* vdit )
{
	Dit_Dhdr *dit_hdr;
	Dit_Phdr *dit_p;
	threadinfo_t ti;
	mthreadid_t mt, mtw;
	void *stack, *copy, *new_entry;
	cap_t cap;
	objinfo_t oi;
	int thread_stat;
	int mem_size, offset, file_size;

	dit_hdr = (Dit_Dhdr*) vdit;
	assert( dit_hdr );

	elf_debug( "DIT is at 0x%llx\n", dit_hdr );

	/* find the PDX object */
	dit_p = find_dit_item( dit_hdr, "apppdx" );
	assert( dit_p );

	/*  elf_debug( "p_entry is 0x%llx\n", dit_p->p_entry ); */

	/* make some object info */
	oi.userinfo = NULL;
	oi.flags = 0;
	oi.special = 0;

	/* get the ELF size out */
	mem_size = ElfGetMemSize( dit_p->p_base );

	/* duplicate the DIT part into some other place in RAM */
	elf_debug( "Creating object of size 0x%llx\n", mem_size );
	copy = ObjCreate( mem_size, 0x37, &oi );
	assert( copy );

	/* give us the cap */
	cap.address = copy;
	cap.passwd = 0x37;
	local_clist_add_cap(cap);

	/* zero out the data (fits BSS!) */
	elf_debug( "Cleaning out object\n" );
	bzero( copy, mem_size );

	/* now copy the text */
	elf_debug( "About to copy to 0x%llx!\n", copy );
	offset = ElfGetSegOffset( dit_p->p_base );
	file_size = ElfGetFileSize( dit_p->p_base );
	memcpy( copy, dit_p->p_base + offset, file_size );
	elf_debug( "Copied!\n" );


	/* now we can relocate the elf file */
	elf_debug( "Relocating ELF file GOT\n" );
	new_entry = RelocateElf( copy, dit_p->p_base );
	elf_debug( "Relocated\n" );





	/* create an object and caps for the new thread's stack */
	stack = ObjCreate( 0x2000, 0x37, &oi );
	assert( stack );

	elf_debug( "Created stack at 0x%llx\n", stack );

	/* give us the cap */
	cap.address = stack;
	local_clist_add_cap(cap);


	/* create the new thread */
	ti.flags = THREAD_STACK_ADDR | THREAD_STACK_SIZE;
	ti.prio = 0;
	ti.stack_addr = stack;
	ti.stack_size = 0x2000;
	ti.start_time = 0;
	ti.cpu_time = 0;
	ti.cpu_limit = 0;
	ti.mem_limit = 0x200000;
	ti.bank_account = NULL;
	ti.env = NULL;
	ti.env_size = 0;
  
	/*  mt = ThreadCreate( dit_p->p_entry, 0x37, &ti, NULL ); */
	/*  new_entry = dit_p->p_entry; */ /* try it without moving */
	elf_debug( "ThreadCreating w/ new_entry = 0x%llx\n", new_entry );
	mt = ThreadCreate( new_entry, 0x37, &ti, NULL );

	if( mt == THREAD_NULL )
		elf_debug( "Error creating thread!\n" );
	else
		elf_debug( "Created new thread ID 0x%llx\n", mt );

	/* do a ThreadWait */
	elf_debug( "About to do a ThreadWait\n" );

	mtw = ThreadWait( mt, &thread_stat );

	elf_debug( "Finished waiting. Got thread id %lld\n", mtw );


	elf_debug( "status is 0x%x\n", thread_stat );

}
#endif

/* load an elf file, ready for execution */
cap_t
elf_load( void* elf_file )
{
	void *copy, *new_entry;
	cap_t cap;
	uintptr_t mem_size, offset, file_size, mem_off;
	int seg, max_seg;
	
	elf_debug( "elf: Getting mem size\n" );

	/* get the ELF size out */
	mem_size = ElfGetMemSize( elf_file );

	/* duplicate the data part into some other place in RAM */
	elf_debug( "elf: Creating object of size 0x%llx\n", mem_size );
	copy = ObjCreate( mem_size, 0x37, NULL );
	assert( copy != NULL );

	/* give us the cap */
	cap.address = copy;
	cap.passwd = 0x37;
	local_clist_add_cap(cap);

	/* zero out the data (fits BSS!) */
	elf_debug( "elf: Cleaning out object\n" );
	bzero( copy, mem_size );

	/* now copy the text */
	elf_debug( "elf: About to copy to 0x%llx!\n", copy );
		
	max_seg = getNumProgramSegments( elf_file );
	for( seg = 0; seg < max_seg; seg++ )
	{
		elf_debug( "elf: copying segment %d\n", seg );
		offset  = ElfGetSegOffset( elf_file, seg );
		mem_off = ElfGetMemOffset( elf_file, seg );
		file_size = ElfGetFileSize( elf_file, seg );
		memcpy( copy + mem_off, 
			elf_file + offset, file_size );
	}
	elf_debug( "elf: Copied!\n" );

	/* now we can relocate the elf file */
	elf_debug( "elf: Relocating ELF file GOT\n" );
	new_entry = RelocateElf( copy, elf_file );
	elf_debug( "elf: Relocated, entry is 0x%llx\n", new_entry );

	cap.address = new_entry;
	return cap;
}
