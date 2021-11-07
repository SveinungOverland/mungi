/****************************************************************************
 *
 *      $Id: mishell.c,v 1.13.2.1 2002/08/29 04:31:51 cgray Exp $
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
 * mishell.c
 *
 * Basic shell for Mungi.
 *
 * Written by Alex Sayle <alexs@cse.unsw.edu.au>
 */

/* I love headers  */
#include <mungi.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>

/* for open() */
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

/* for read/write */
/* #include <unistd.h> */  /* hmmm... causes many compile warnings... */

/* local headers */
#include "mungi/l4_generic.h"
#include "mishell.h"

#include <mlib.h>
#include <clock.h>

/*FIXME: SEELDY includes*/
#include "../../../lib/libc/posix/oftab.h"
#include "../../../lib/libc/posix/env.h"

extern void _mungi_lib_init(void);

#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif

#define WHITESPACE " \t\v"

/* testing */

static void cmd_cat(int argc, char **argv);
static void cmd_ls(int argc, char **argv);
static void cmd_mkdir(int argc, char **argv);
static void cmd_rmdir(int argc, char **argv);
static void cmd_cd(int argc, char **argv);
static void cmd_cp(int argc, char **argv);
static void cmd_pwd(int argc, char **argv);
static void cmd_raw(int argc, char **argv);
static void cmd_clear(int argc, char** argv);
static void cmd_args(int argc, char** argv);

#ifdef CLOCK
static void cmd_pdx(int argc, char** argv);
#endif

static void cmd_env(int argc, char** argv);
static void cmd_echo(int argc, char** argv);
static void cmd_export(int argc, char** argv);
static void cmd_help(int argc, char** argv);
static void cmd_rbt(int argc, char** argv);


void read_line(char* buf);

char *prompt = "mishell%% "; 

/**
 * built it command table 
 *
 */
struct { 
	char *cmd; 
	void (*fn)(int argc ,char** argv); 
	char *cmd_help;
} commands[] = {
	{ "ls",      cmd_ls,    "list the the contexnt of a dir"},
	{ "mkdir",   cmd_mkdir, "Creates a directory"},
	{ "rmdir",   cmd_rmdir, "Removes a directory file"},
	{ "cd",      cmd_cd,    "Change working directory"},
	{ "cp",      cmd_cp,    "Copies files"},
	{ "cat",     cmd_cat,   "Read/write files"},
	{ "pwd",     cmd_pwd,   "Display the current working directory"},
	{ "clear",   cmd_clear, "clear the screen"},
	{ "args",    cmd_args,  "parses the args given to this command"},
	{ "env" ,    cmd_env,   "_should_ print out the env verialbes"},
	{ "echo" ,   cmd_echo,  "echo something."},
	{ "export" , cmd_export,"_should_ export and env veriable_"},
	{ "help",    cmd_help,  "shows the help info."},
	{ "raw",     cmd_raw,   "display raw character codes."},
#ifdef CLOCK
	{ "pdx",     cmd_pdx,   "time some PDX calls."},
#endif
	{ "reboot",  cmd_rbt,   "drop to L4 debugger."},
	{ NULL ,NULL}
};


/*
 * a not so seedy way to read a line.
 */
void read_line( char* buf ){
	
	printf(prompt);
	fflush(stdout);
	fgets(buf, 90, stdin);
}


/**
 * 
 * env 
 *  
 */
static void 
cmd_env(int argc , char** argv)
{       
	printf( "\t --:: env ::--\n");
	printf( "\t --:: end env ::--\n");
	return;
}

/**
 * 
 * echo
 *  
 */
static void 
cmd_echo( int argc , char** argv  )
{       
	printf( "\t --:: echo ::--\n");
	printf( "\t --:: end echo ::--\n");
	return;
}

/**
 * 
 * export
 *  
 */
static void 
cmd_export( int argc , char** argv  )
{       
	printf( "\t --:: export ::--\n");
	printf( "\t --:: end export ::--\n");
	return;
}





/**
 * 
 * clear 
 *  
 */
static void 
cmd_clear( int argc , char** argv  )
{
	printf( "\033[0m\033[2J\033[1;1H" );
}

/**
 * 
 * args
 *  
 */
static void 
cmd_args( int argc , char** argv)
{
	int i ;
	char * st ;
	printf("argc is : %d\n",argc);
	
	for(i = 0 ; i <argc ;i++){
		st =  argv[i] ;
		printf("%d : %s \n ", i , st);
	}	
}

/**
 * 
 * help : print help for a single command, or all commands.
 *  
 */
static void 
cmd_help( int argc , char** argv){
	int i =0;

	if(argv[1] == NULL || strcmp(argv[1],"all")== 0){
		printf("This is the Mungi Interactive Shell, aka Mishell\n");
		printf("\t the available commands for Mishell are..\n");
		for( i = 0; commands[i].cmd != NULL ; i++ ){
			printf("%s : %s\n",commands[i].cmd,
			       commands[i].cmd_help );
		}
		printf("\nif you want more commands, prod mungi.\n");
	}
	else
	{
		for( i = 0; commands[i].cmd != NULL ; i++ )
		{
			if( strcmp( commands[i].cmd, argv[1] ) == 0 )
			{
				printf("%s:\t%s\n",commands[i].cmd,
				       commands[i].cmd_help );
				return;
			}
		}

		printf( "help: command '%s' not found\n", argv[1] );
			
	}
}

/**
 * 
 * ls
 *  
 */
static void 
cmd_ls( int argc , char** argv  )
{       
	/* FIXME: clean this up! */
	DIR *dir;
	struct dirent *ent;
	
	if( argc == 1 )
		dir = opendir( "." );
	else
		dir = opendir( argv[1] );

	if( dir == NULL )
	{
		printf( "error on opendir()\n" );
		return;
	}
		
	ent = readdir( dir );

	while( ent != NULL )
	{
		printf( "%s\n", ent->d_name );
		ent = readdir( dir );
	}		
		
	closedir( dir );
}



/**
 * 
 * mkdir
 *  
 */
static void 
cmd_mkdir( int argc , char** argv  )
{
	if( argc != 2 )
	{
		printf( "usage: mkdir <name>\n" );
		return;
	}

	mkdir( argv[1], 0 );
}

/**
 * 
 * rmdir
 *  
 */
static void 
cmd_rmdir( int argc , char** argv  )
{
	int r;

	if( argc != 2 )
	{
		printf( "usage: rmdir <name>\n" );
		return;
	}

        r = rmdir( argv[1] );

	if( r != 0 )
		printf( "rmdir: could not rmdir '%s'\n", argv[1] );
}

/**
 * 
 * cat
 *  
 */
#define MYSIZE 10
static void 
cmd_cat( int argc , char** argv  )
{
	int in = 0;
	int out = 0;
	int app = 0;
	int fd;
	int flags;
	char buf[MYSIZE+1];
	int ret;

	if( argc != 3 )
		goto cat_usage;

	if( strcmp( argv[1], "<" ) == 0 )
		in = 1;
	if( strcmp( argv[1], ">" ) == 0 )
		out = 1;
	if( strcmp( argv[1], ">>" ) == 0 )
		app = out = 1;

	if( in == 0 && out == 0 )
		goto cat_usage;

	/* now we can do stuff */
	if( in != 0 )
		flags = O_RDONLY;
	else
	{
		flags = O_CREAT | O_WRONLY;

		if( app != 0 )
			flags |= O_APPEND;
		else
			flags |= O_TRUNC;
	}

	fd = open( argv[2], flags );

	if( fd == -1 )
	{
		printf( "cat: error opening file '%s'\n", argv[2] );
		return;
	}

	buf[MYSIZE] = '\0';
	if( in )
	{
		while( (ret = read( fd, buf, MYSIZE )) != 0 )
		{
			if( ret == -1 )
			{
				printf( "cat: error on read\n" );
				close( fd );
				return;
			}

			buf[ret] = '\0';
			printf( "%s", buf );
		}
	}
	else
	{
		while( (buf[0] = (char) getchar()) != 4 )
		{
			ret = write( fd, buf, 1 );

			if( ret == 0 )
			{
				printf( "cat: write returned EOF??\n" );
				close( fd );
				return;
			}
			
			if( ret == -1 )
			{
				printf( "cat: write returned error\n" );
				close( fd );
				return;
			}
		}
	}

	close( fd );
	return;

cat_usage:
	printf( "usage: cat (<|>) <name>\n" );
	return;	
}


/**
 * 
 * cd
 *  
 */
static void 
cmd_cd( int argc , char** argv  )
{
	int r;

	if( argc != 2 )
	{
		printf( "usage: cd <name>\n" );
		return;
	}

	r = chdir( argv[1] );

	if( r != 0 )
		printf( "cd: error using chdir()\n" );
}

/**
 * 
 * cp
 *  
 */
static void 
cmd_cp( int argc , char** argv  )
{
	int r, k;
	int src, dst;
	char buf[MYSIZE];
	
	if( argc != 3 )
	{
		printf( "usage: cp <file1> <file2>\n" );
		return;
	}

	src = open( argv[1], O_RDONLY );
	if( src == -1 )
	{
		printf( "cp: error opening '%s'\n", argv[1] );
		return;
	}

	dst = open( argv[2], O_WRONLY | O_CREAT | O_TRUNC );
	if( src == -1 )
	{
		printf( "cp: error opening '%s'\n", argv[2] );
		close( src );
		return;
	}

	while( ( r = read( src, buf, MYSIZE ) ) > 0 )
	{
		while( r > 0 )
		{
			k = write( dst, buf, r );
			if( k == -1 )
			{
				printf( "cp: error on write\n" );
				close( src );
				close( dst );
				return;
			}
			r -= k;
		}
	}

	close( src );
	close( dst );
}

/**
 * 
 * pwd
 *  
 */
static void 
cmd_pwd( int argc , char** argv  )
{
	char buf[PATH_MAX+1];
	
	if( getwd( buf ) == NULL )
		printf( "pwd: could not getwd()\n" );
	else
		printf( "%s\n", buf );
}


/**
 * 
 * raw
 *  
 */
static void 
cmd_raw( int argc , char** argv  )
{
	int c = 0;

	printf( "Doing raw stuff??\n" );

	while( (c = getchar()) != 4 )
		printf( "char %d (0x%x)\n", c, c );
}

#ifdef CLOCK
static uint32_t startval = 0;

void mclock_init(void);
void mclock_init(void)
{
	_clock_init();
}

void mclock_start(void);
void mclock_start(void)
{
	_clock_start();
	startval = _clock_stop(0);
}

/* stop timer, print message, return time since started */
uint32_t mclock_stop(const char *s);
uint32_t mclock_stop(const char *s)
{
	return _clock_stop(0) - startval;
}


 
/** 
  *
 * PDX functions
 *  
 */
cap_t pdx_func( cap_t param );

cap_t 
pdx_func( cap_t param )
{
	return param;
}

static void 
cmd_pdx( int argc , char** argv  )
{
	static cap_t pcap = {NULL,0}, clist;
	int i;

	printf( "Doing PDX timing\n" );

	/* check/create the PDX entry point */
	if( pcap.address == NULL )
	{
		void *ptr;
		cap_t *p;
		p = ApdLookup( pdx_func, M_OWNER );

		if( p == NULL )
		{
			printf( "Error getting owner cap\n" );
			return;
		}

		create_clist_and_add( L4_PAGESIZE, &clist );
		add_to_clist( pcap, clist.address );
		
		pcap = *p;
		ptr = pdx_func;
		ObjCrePdx( pcap, clist.address, 0, 0, 0, 1, (pdx_t*)&ptr );
	}

	/* sleep so printing etc. can die down */
	ThreadSleep( THREAD_MYSELF, 1000000000 );

	/* init the clock */
	mclock_init();

	/* call it n times */
	for( i = 0; i <  10; i++ )
	{
		int r;
		r = PdxCall( pdx_func, pcap, &pcap, PD_MERGE );

		if( r != 0 )
		{
			printf( "Error on PDX call (%d)\n", r );
			break;
		}
	}

	/* start timer */
	mclock_start();

	/* call it n times */
	for( i = 0; i <  9; i++ )
	{
		int r;
		r = PdxCall( pdx_func, pcap, &pcap, PD_MERGE );

		if( r != 0 )
		{
			printf( "Error on PDX call (%d)\n", r );
			break;
		}
	}

	/* enable clock output */
	clock_set_dump_enabled(1);
	PdxCall( pdx_func, pcap, &pcap, PD_MERGE );
	/* disable clock output */
	clock_set_dump_enabled(0);


	/* stop timer */
	{
		uint32_t ber = mclock_stop( "PDX calls" );
		printf( "PDX time: %d\n", ber );
	}
}

#endif /* CLOCK */

/**
 * 
 * reboot - alpha only
 *  
 */
static void cmd_rbt(int argc, char** argv)
{
#if ALPHAENV
	asm( "call_pal 0xbf" );
#else
	assert(!"reboot requested" );
#endif
}

 
/** 
 *
 *
 *  main loop
 *
 */
int 
main(void *param){
	char buf[READLINE_BUF_SIZE];
        char * argv[20];
        char *p;

	int r,i,cmd_exist;

	int argc;
	l4_threadid_t me;

	/* do library initialisations */
	_mungi_lib_init();

	/* print a welcome message */
	{
		char *welcome;
		welcome = "Welcome to MiShell\n";
		SeedyPrint( welcome );
		// asm( "call_pal 0xbf" );
		UserPrint( welcome );
	}

        me = l4_myself();
	UserPrint( "MiShell id is 0x%llx\n", (void*)(me.ID) );
	ThreadSleep( THREAD_MYSELF, 1000000000 );
	r = env_decode( param);
	assert(r==0);
     
        while( 1 ) {
                read_line( buf );
		
		cmd_exist = FALSE;
		/*  Parse String  */
		argc = 0 ;	
		argv[argc] = strtok( buf, WHITESPACE);

		while( argv[argc] != NULL){
                        argc++;
                        argv[argc] = strtok( NULL, WHITESPACE);
		}
		
                /* strip off the \n on the tail*/
                p = argv[argc-1];
                while (*p != '\n' && *p != 0)
                        p ++;
                *p = 0; 
			
		/* Deal with the result  */
		if( (argv[0] == NULL) || (*argv[0] == '\0') ){
                        argc  = 0;
			continue;
		}else{
                        /* loop on the command table for the command */
                        for( i = 0; commands[i].cmd != NULL ; i++ ){
				if( strcmp( argv[0], commands[i].cmd ) == 0 ){
                                        commands[i].fn( argc, argv );
					cmd_exist = TRUE;
					break ;
				}
                        }
			if(cmd_exist == FALSE){
                                fprintf( stderr, "command not found: %s\n", 
					 argv[0] );
                        }
		}
	}

        printf( "done\n" );
        while(1)
                ThreadSleep(THREAD_MYSELF, 1000000000);

}


