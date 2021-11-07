/****************************************************************************
 *
 *      $Id: syscalls.h,v 1.2 2002/05/31 08:01:36 danielp Exp $
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

#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/stat.h>

/*
int open(const char *path, int oflag, ...);
int creat(const char *path, mode_t mode);
int close(int fildes);
int write(int fildes, const void *buf, unsigned int nbyte);
int read(int fildes, void *buf, unsigned int nbyte);
int mkdir(const char *path, mode_t mode);
int rmdir(const char *path);
int unlink(const char *path);
int dup(int fildes);
int dup2(int fildes, int fildes2);
int fcntl(int fildes, int request, ...);
off_t lseek(int fildes, off_t offset, int whence);
int stat(const char *path, struct stat *buf);
int fstat(int fildes, struct stat *buf);
*/

/*
 stat, lstat, fstat are all needed.
*/


void fs_init( void );
void extend_filetable( void );
char *addDir(char *path);
void freeDir(void);

typedef struct file_table {
  char *path;
  int status;
  int position;
} file_table_t;

#define INIT_FILE_TABLE_SIZE 10

#define false 0
#define true  1

#define FS_PASSWORD 0xAABBAA00


extern int file_table_size;
extern file_table_t *filetable;
extern int fs_initialised;
extern char *curdir;

/*
typedef unsigned int     uint_t;
typedef uint_t           mode_t;

#define O_APPEND   00000010
#define O_RDONLY   00000000
#define O_WRONLY   00000001
#define O_RDWR     00000002

#define O_CREAT    00001000
#define O_TRUNC    00002000
#define O_EXCL     00004000

#define EEXIST     17
#define ENOENT     2
*/

