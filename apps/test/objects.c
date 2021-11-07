/* Mungi object testing code */

#include <mungi.h>
#include <assert.h>
#include <limits.h>
#include <mlib.h>  /* make some stuff simpler */

#include <stdio.h>  /* for sprintf */

void UserPrint( char *fmt, ... );

#define printfm(x...) UserPrint(x)


void object_test( int *passed, int *tested );

/* verify a test */
static void verify_test( int check, int *passed, int *tested, char *msg );
static void 
verify_test( int check, int *passed, int *tested, char *msg )
{
	if( check )
		(*passed)++;
	else
		printfm( "FAIL on test '%s'\n", msg );

	(*tested)++;
}

static void 
fail_test( char *msg, int *tested )
{
	printfm( "FAIL on test '%s' forced\n", msg );
	(*tested)++;
}


#ifdef TESTING 

/* keep counting here, so we know if they *actually* failed
 * not thread-safe, tho!
 */
static int pdx_call_count = 0;

/* some PDX functions */
static cap_t
pdx_entry_swap( cap_t param )
{
	cap_t ret;

	pdx_call_count++;

	ret.address = (void*) param.passwd;
	ret.passwd  = (uintptr_t) param.address;

	return ret;
}

static cap_t
pdx_entry_reverse( cap_t param )
{
	cap_t ret;

	pdx_call_count++;

	ret.address = (void*) ~((uintptr_t)param.address);
	ret.passwd  = ~param.passwd;

	return ret;
}

static cap_t
pdx_entry_subtract( cap_t param )
{
	cap_t ret;

	pdx_call_count++;

	/* some random subtraction */
	ret.address = (void*) LONG_MAX - ((uintptr_t)param.address);
	ret.passwd  = LONG_MAX - param.passwd;

	return ret;
}

static cap_t
pdx_entry_zero( cap_t param )
{
	pdx_call_count++;

	param.address = (void*) param.passwd = 0;

	return param;
}

static cap_t
pdx_entry_one( cap_t param )
{
	pdx_call_count++;

	param.address = (void*) param.passwd = 1;

	return param;
}

static int free_pdx = 0, pdx_waiting = 0;

static cap_t
pdx_entry_wait( cap_t param )
{
	while( free_pdx == 0 )
	{
		pdx_waiting = 1;
		ThreadSleep( THREAD_MYSELF, 1000000000 );
	}

	return param;
}

static cap_t 
pdx_entry_free( cap_t param )
{
	free_pdx = 1;

	return param;
}

static cap_t
pdx_entry_recurse( cap_t param )
{
	int r;

	pdx_call_count++;
	if( param.passwd <= 1 )
		return param;

	param.passwd--;
	r = PdxCall( pdx_entry_recurse, param, &param, PD_EMPTY );
	assert( r == 0 );

	return param;
}

static int 
capcmp( cap_t c1, cap_t c2 )
{
	return !((c1.address == c2.address) && (c1.passwd == c2.passwd));
}

#if 0
static void
printcap( char *name, cap_t cap )
{
	printfm( "cap %s: (%p, %lx)\n", name, cap.address, cap.passwd );
}
#endif

/* check we *can't* call something */
static void
check_pdx_call_fail( pdx_t ept, char *msg, int *passed, int *tested )
{
	cap_t test;
	int r, old_calls;

	test.address = NULL;
	test.passwd = 0;

	old_calls = pdx_call_count;
	r = PdxCall( ept, test, &test, PD_EMPTY );

	/* check it was actually not called */
	verify_test( pdx_call_count == old_calls, passed, tested, msg );

	/* verify the return value */
	verify_test( r != 0, passed, tested, msg );
}

/* verifying PDX made easy! */
static void
check_pdx_call( pdx_t ept, char *msg, int *passed, int *tested )
{
	cap_t c_orig, c_test, c_check;
	int r;

	/* now do some calls... */
	c_orig.address = (void*) 0xabcde12345673769LL;
	c_orig.passwd  = 0x1234567abcde3769LL;
	
	c_check = ept( c_orig );
	r = PdxCall( ept, c_orig, &c_test, PD_EMPTY );
	
	if( r == 0 )
		verify_test( capcmp( c_check, c_test ) == 0, 
			     passed, tested, msg );
	else
		fail_test( msg, tested );
}

#if 0
/* dump out the entry point tables */
/* use this when PDX tables don't work :) */
static void
dump_pdx_info( void *ptr )
{
	int npts, i;
	objinfo_t info;

	/* get the obj info */
	i = ObjInfo( ptr, 0, &info );
	assert( i == 0 );

	printfm( "n_pdx is %d\n", info.n_pdx );

	npts = info.pdx.x_entry[info.n_pdx-1];
	printfm( "num entry pts is %d\n", npts );

	for( i = 0; i < npts; i++ )
		printfm( "entry[%2d] = %p\n", i, info.pdx.entry[i] );

}
#endif

#define PDX_PWD_1 0x1234
#define PDX_PWD_2 0x4321

/* testing ObjCrePdx */
static void create_pdx_test( clist_t *list, clist_t *our_clist, int *passed, 
			     int *tested );
static void
create_pdx_test( clist_t *clist, clist_t *our_clist, int *passed, int *tested )
{
	objinfo_t info;
	int r, initial;
	cap_t pdx_cap;
	pdx_t ept[] = { pdx_entry_swap, 
			pdx_entry_reverse, 
			pdx_entry_subtract,
			pdx_entry_zero,
			pdx_entry_one,
	};

	printfm( "-- Testing ObjCrePdx()\n" );

	/* first find out some info about our code object */
	r = ObjInfo( create_pdx_test, 0, &info );
	assert( r == 0 );

	initial = info.n_pdx;

	printfm( "%d PDXs already registered\n", initial );

	/* create the PDX */
	pdx_cap.address = create_pdx_test;
	pdx_cap.passwd  = PDX_PWD_1;
	r = ObjCrePdx( pdx_cap, clist, NULL, NULL, NULL, 3, ept );
	assert( r == 0 );

	/* give us the cap */
	add_to_clist( pdx_cap, our_clist );

	/* test calculation*/
	r = ObjInfo( create_pdx_test, 0, &info );
	assert( r == 0 );
	verify_test( info.n_pdx - initial == 1, passed, tested, 
		     "PDX Register" );

	/* test swap */
	check_pdx_call( ept[0], "PDX swap", passed, tested );
	check_pdx_call( ept[1], "PDX reverse", passed, tested );
	check_pdx_call( ept[2], "PDX subtract", passed, tested );


	/* create another PDX entry point (or two) */
	pdx_cap.passwd = PDX_PWD_2;
	r = ObjCrePdx( pdx_cap, clist, NULL, NULL, NULL, 2, &ept[3] );
	assert( r == 0 );

	/* make sure we can call it */
	add_to_clist( pdx_cap, our_clist );

	/* test the count */
	r = ObjInfo( create_pdx_test, 0, &info );
	assert( r == 0 );
	verify_test( info.n_pdx - initial == 2, passed, tested, 
		     "PDX Register 2" );
	
 	/* check if these ones work */
	check_pdx_call( ept[3], "PDX zero", passed, tested );
	check_pdx_call( ept[4], "PDX one" , passed, tested );

	/* now delete two entries */
	pdx_cap.passwd = PDX_PWD_1;  /* first registered entry */
	r = ObjCrePdx( pdx_cap, clist, NULL, NULL, NULL, 1, ept );
	assert( r == 0 );

	/* now try some failures */
	check_pdx_call( ept[0], "PDX swap (2)", passed, tested );
	check_pdx_call_fail( ept[1], "PDX reverse  (2F)", passed, tested );
	check_pdx_call_fail( ept[2], "PDX subtract (2F)", passed, tested );


	/* delete the last entry entries */
	pdx_cap.passwd = PDX_PWD_1;
	r = ObjCrePdx( pdx_cap, clist, NULL, NULL, NULL, 0, ept );
	assert( r == 0 );

	/* try and call it, and the other remaining */
	check_pdx_call_fail( ept[0], "PDX swap (3F)", passed, tested );
	check_pdx_call( ept[3], "PDX zero (2)", passed, tested );

	/* delete the last entry */
	pdx_cap.passwd = PDX_PWD_2;
	r = ObjCrePdx( pdx_cap, clist, NULL, NULL, NULL, 0, ept );
	assert( r == 0 );

	for( r = 0; r < 4; r++ )
	{
		char buf[100];
		sprintf( buf, "PDX final %d (F)", r );

		check_pdx_call_fail( ept[r], buf, passed, tested );
	}
}

#endif

static int
pdx_sleep_entry( void* param )
{
	int r;
	cap_t cap;

	cap.address = param;
	cap.passwd = 0;

	r = PdxCall( pdx_entry_wait, cap, &cap, PD_EMPTY );
	assert( r == 0 );

	return 0;
}

#define RECURSIVE_COUNT 20

static void
multiple_pdx_test( clist_t *clist, clist_t *our_clist, 
		    int *passed, int *tested )
{
	int r, init_count;
	cap_t pdx_cap, param;
	pdx_t ept[] = { pdx_entry_free, pdx_entry_wait, pdx_entry_recurse };
	mthreadid_t thread;

	/* create the PDX */
	pdx_cap.address = multiple_pdx_test;
	pdx_cap.passwd  = PDX_PWD_1;
	r = ObjCrePdx( pdx_cap, clist, NULL, NULL, NULL, 3, ept );
	assert( r == 0 );

	/* give us the cap */
	add_to_clist( pdx_cap, our_clist );

	/* give them a cap for recursive */
	add_to_clist( pdx_cap, clist );

	/* create a thread */
	thread = ThreadCreate( pdx_sleep_entry, NULL, NULL, NULL );

	if( thread == THREAD_NULL )
		fail_test( "PDX Thread Create", tested );
	else
	{
		/* wait until it's sleeping */
		while( pdx_waiting == 0 )
			ThreadSleep( THREAD_MYSELF, 1000000000 );
		
		/* now call them! */
		param.address = NULL;
		param.passwd = 0;
		r = PdxCall( pdx_entry_free, param, &param, PD_EMPTY );
		verify_test( r == 0, passed, tested, "Multiple PDX" );
	}


	/* try a recursive call */
	init_count = pdx_call_count;
	param.passwd = 20;  /* number of recursions */
	r = PdxCall( pdx_entry_recurse, param, &param, PD_EMPTY );
	
	if( r != 0 )
		fail_test( "PDX Recursive fail", tested );
	else
		verify_test( (pdx_call_count - init_count) == RECURSIVE_COUNT, 
			     passed, tested, "Recursive PDX count" );

}


static void
setup_pdx_apd( clist_t **clist, clist_t **our_clist )
{
	cap_t clist_cap, call_cap;
	cap_t *addcap;

	/* we need a clist etc. and to call ObjCrePdx */
	create_clist( 4096, &clist_cap );
	*clist = clist_cap.address;

	/* add code cap to the clist */
	addcap = ApdLookup( create_pdx_test, M_READ );
	assert( addcap != NULL );

	add_to_clist( *addcap, *clist );

	/* we need to create a clist to store PDX caps into */
	create_clist_and_add( 4096, &call_cap );
	*our_clist = call_cap.address;
}

/* main object testing entry point */
void
object_test( int *passed, int *tested )
{
	int p = 0, t = 0;
	clist_t *clist, *our_clist;

	printfm( "*** Starting object tests ***\n" );

	/* setup PDX APD and clist */
	setup_pdx_apd( &clist, &our_clist );

#ifdef TESTING
	/* test ObjCrePdx */
	create_pdx_test( clist, our_clist, &p, &t );
#endif

	/* test PDX call for multiple entries at once */
	multiple_pdx_test( clist, our_clist, &p, &t );

	printfm( "Objects passed %d/%d tests\n", p, t );

	*passed = p;
	*tested = t;
}
