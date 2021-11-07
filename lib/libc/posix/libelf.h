/****************************************************************************
 *
 *      $Id: libelf.h,v 1.2 2002/05/31 07:43:52 danielp Exp $
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

/* libelf.h
 * a simple library to help with interpreting 64-bit mips elf executables
 * author: Luke Deller (luked@cse.unsw.edu.au)
 * date:   24/Sep/1999
 */

#ifndef __LIBELF_H__
#define __LIBELF_H__

/* basic types */
typedef unsigned long long	Elf64_Addr; /* for memory addresses */
typedef unsigned short		Elf64_Half;
typedef unsigned long long	Elf64_Off;  /* for file offsets */
typedef signed int		Elf64_Sword;
typedef signed long long	Elf64_Sxword;
typedef unsigned int		Elf64_Word;
typedef unsigned long long	Elf64_Xword;
typedef unsigned char		Elf64_Byte;
typedef unsigned short		Elf64_Section;

/* file header */
typedef struct Elf64_Header_s {
  unsigned char e_ident [16];
  Elf64_Half e_type;      /* Relocatable=1, Executable=2 (+ some more ..) */
  Elf64_Half e_machine;   /* Target architecture: MIPS=8 */
  Elf64_Word e_version;   /* Elf version (should be 1) */
  Elf64_Addr e_entry;     /* Code entry point */
  Elf64_Off e_phoff;      /* Program header table */
  Elf64_Off e_shoff;      /* Section header table */
  Elf64_Word e_flags;     /* Flags */
  Elf64_Half e_ehsize;    /* ELF header size */
  Elf64_Half e_phentsize; /* Size of one program segment header */
  Elf64_Half e_phnum;     /* Number of program segment headers */
  Elf64_Half e_shentsize; /* Size of one section header */
  Elf64_Half e_shnum;     /* Number of section headers */
  Elf64_Half e_shstrndx;  /* Section header index of the string table
                             for section header names */
} Elf64_Header_t;

/* program segment header */
typedef struct Elf64_Phdr_s {
  Elf64_Word p_type;      /* Segment type: Loadable segment = 1 */
  Elf64_Word p_flags;     /* Flags: logical "or" of PF_ constants below */
  Elf64_Off p_offset;     /* Offset of segment in file */
  Elf64_Addr p_vaddr;     /* Reqd virtual address of segment when loading */
  Elf64_Addr p_paddr;     /* Reqd physical address of segment (ignore) */
  Elf64_Xword p_filesz;   /* How many bytes this segment occupies in file */
  Elf64_Xword p_memsz;    /* How many bytes this segment should occupy in
                           * memory (when loading, expand the segment by
                           * concatenating enough zero bytes to it) */
  Elf64_Xword p_align;    /* Reqd alignment of segment in memory */
} Elf64_Phdr_t;

/* constants for Elf64_Phdr_t.p_flags */
#define PF_X		1 /* readable segment */
#define PF_W		2 /* writeable segment */
#define PF_R		4 /* executable segment */

/* constants for indexing into Elf64_Header_t.e_ident */
#define EI_MAG0		0
#define EI_MAG1		1
#define EI_MAG2		2
#define EI_MAG3		3
#define EI_CLASS	4
#define EI_DATA		5
#define EI_VERSION	6



/* Section header.  */
typedef struct
{
  Elf64_Word    sh_name;                /* Section name (string tbl index) */
  Elf64_Word    sh_type;                /* Section type */
  Elf64_Xword   sh_flags;               /* Section flags */
  Elf64_Addr    sh_addr;                /* Section virtual addr at execution */
  Elf64_Off     sh_offset;              /* Section file offset */
  Elf64_Xword   sh_size;                /* Section size in bytes */
  Elf64_Word    sh_link;                /* Link to another section */
  Elf64_Word    sh_info;                /* Additional section information */
  Elf64_Xword   sh_addralign;           /* Section alignment */               
  Elf64_Xword   sh_entsize;             /* Entry size if section holds table */ 
} Elf64_Shdr_t;

typedef struct
{
  Elf64_Addr    r_offset;               /* Address */
  Elf64_Xword   r_info;                 /* Relocation type and symbol index */
} Elf64_Rel_t;


/* functions provided by libelf.c */

int
checkElfFile (void *elfFile);
/* Checks that elfFile points to a valid elf file.
 * Returns 0 if the elf file is valid, < 0 if invalid.
 */

Elf64_Phdr_t *
getProgramSegmentTable (void *elfFile);
/* Returns a pointer to the program segment table, which is an array of
 * ELF64_Phdr_t structs.  The size of the array can be found by calling
 * getNumProgramSegments.
 */

unsigned
getNumProgramSegments (void *elfFile);
/* Returns the number of program segments in this elf file.
 */

#endif

