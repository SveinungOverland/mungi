/****************************************************************************
 *
 *      $Id: test.c,v 1.1 2002/08/23 08:24:13 cgray Exp $
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
 * test.c
 *
 * Starting the Mungi Testing Suite
 *
 * Charles Gray <cgray@cse.unsw.edu.au>
 */

#include <mungi.h>
#include <assert.h>

void UserPrint( char *fmt, ... );

#define printfm(x...) UserPrint(x)

extern void _mungi_lib_init(void);

void object_test(int *passed, int *tested);


int 
main(void *param)
{
	int ok, tested;

	/* library init */
	_mungi_lib_init();

	/* show ourselves to the user */
	printfm( "Welcome to the Mungi Test Suite\n" );
	printfm( "-------------------------------\n" );


	/* start the object tests */
	object_test( &ok, &tested );

	printfm( "Testing done. Passed %d/%d tests\n", ok, tested );

	while(1)
		ThreadSleep( THREAD_MYSELF, 1000000000 );

}


