/****************************************************************************
 *
 *      $Id: syscalls.h,v 1.17.2.1 2002/08/29 04:31:53 cgray Exp $
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

/* Mungi system calls */

#ifndef __MUNGI_SYSCALLS_H
#define __MUNGI_SYSCALLS_H

#include <sys/types.h>
#include <exception.h>


/***********\
 * Objects *
\***********/


void * 
ObjCreate ( size_t size, passwd_t passwd, const objinfo_t * info );
/*
 * Allocates object of "size" bytes with owner password "passwd" and object
 * info "*info".
 *
 * Return: object address if successful, NULL otherwise.
 * errors: ST_NOMEM, ST_INFO, ST_BANK.
 */

int 
ObjDelete ( void * obj );
/*
 * Deallocates object which contains address "obj".
 *
 * Return: zero if successful, otherwise !=0
 * errors: ST_PROT.
 */

int
ObjResize ( void * obj, size_t new_size );
/*
 * Resizes object which contains address "obj" to new size "new_size". 
 * Call will fail if object cannot be extended in situ.
 *
 * Return: zero if successful, otherwise !=0
 * errors: ST_PROT, ST_INFO, ST_NOGROW, ST_NOMEM.
 */

int
ObjPasswd ( cap_t cap, access_t mode );
/*
 * Registers a new capability conferring rights "mode".
 * or if mode is zero, deletes an existing capability matching cap.passwd.
 *
 * Return: zero if successful, otherwise !=0
 * errors: ST_PROT, ST_OVFL.
 */

int 
ObjInfo ( const void * obj, int flags, objinfo_t * info /* INOUT */ );
/*
 * Update the object table entry for "obj". The parameter "flags" specifies
 * which fields are changed.
 * Returns, through "info", the pre-call attribute settings.
 *
 * Return: zero if successful, otherwise !=0
 * errors: ST_PROT, ST_INFO.
 */

int
ObjCrePdx(cap_t cap, const clist_t * clist, uintptr_t unused1,
	  uintptr_t unused2, uintptr_t unused3, uint n_entrypt,
	  pdx_t entry_pnts[]);
/*
 * Registers a new PDX capability cap.
 * When called, the PDX procedure will extend the callers protection
 * domain by the specified "clist". PDX calls via the new capability are valid
 * to one of the "n_entrypt" specified entry points in entry_pnts[].
 *
 * Return: zero if successful, otherwise !=0
 * errors: ST_PROT, ST_OVFL, ST_NULL, ST_CLIST, ST_PDX, ST_LOCK.
 */

int
ObjNewPager ( void * obj, pager_t pager );
/*
 * Registers PDX function "pager" as the page fault handler for "obj".
 *
 * Return: zero if successful, otherwise !=0
 * errors: ST_PROT, ST_PDX, ST_LOCK.
 */



/*******\
 * APD *
\*******/

int
ApdInsert ( apdpos_t pos, const clist_t * clist );
/*
 * Insert at position "pos" in the kernel's APD data structure a capability
 * for object "clist". The slot "pos" must not be locked. Slot "pos" and all 
 * further slots are shifted downwards.
 *
 * Return: zero if successful, otherwise !=0
 * errors: ST_PROT, ST_POS, ST_OVFL, ST_LOCK.
 */

int
ApdDelete ( apdpos_t pos );
/*
 * Pop slot "pos" (which must not be locked) from the kernel's APD data
 * structure (shifting up any further entries).
 *
 * Return: zero if successful, otherwise !=0
 * errors: ST_POS, ST_LOCK.
 */

int
ApdGet ( apddesc_t * apd /* OUT */);
/*
 * Return a copy of the kernel's APD data structure through "apd". Only the
 * address part of Clist caps are returned (no passwords).
 * "apd->n_locked" returns the locking status for the APD.
 * (slot i is locked iff (i <= n_locked) ).
 *
 * Return: zero if successful, otherwise !=0
 * errors: ST_PROT.
 */

int
ApdLock ( apdpos_t pos );
/*
 * Lock the slot pos in the caller's APD, imposing restrictions
 * on a number of system calls. 
 * A slot value of -1 locks the whole APD. 
 * A locked APD (slot) cannot be unlocked. 
 * Locking an already locked APD (slot) has no effect.
 * All slots below pos are also locked.
 *
 * Return: zero if successful, otherwise !=0 
 * errors: ST_POS.
 */

cap_t *
ApdLookup ( const void * address, access_t minrights );
/*
 * Validate access to "address" of type "minrights".
 * If the access is allowed, the validation is cached as a side effect. 
 *
 * Return: NULL if this access is not possible, otherwise the address 
 *		(within a C-list) of the capability granting at least the 
 *		requested rights.
 * errors: ST_PROT.
 */

void
ApdFlush ( void );
/*
 * Flush the validation cache, forcing revalidation to occur for all future
 * object accesses. Also perform re-validation of Clist caps in the APD.
 */

int
PdxCall ( pdx_t proc, cap_t param, cap_t *ret /* OUT */, const apddesc_t *pd );
/*
 * Call "proc" as a PDX procedure. 
 *
 * If "pd" is PD_MERGE, the PDX procedure executes in a protection domain 
 * which is the union of the caller's APD with the Clist registered for 
 * the PDX. 
 * If "pd" is PD_EMPTY, no protection domain is passed, and the PDX executes in
 * an APD consisting solely of its registered Clist. Otherwise, the extended
 * APD is constructed from the registered Clist and the protection
 * domain explicitly passed via pd (a pointer to a PD object). 
 * "param" specifies an arbitrary parameter passed to the PDX by value.
 * "ret" is the return value from the PDX call.
 *
 * Return: zero if successful, otherwise !=0
 * errors: ST_PROT, ST_PDX, ST_LOCK.
 */



/**********\
 * Threads *
\**********/

mthreadid_t
ThreadCreate(thread_t ip, void *param, const threadinfo_t *info, 
		const apddesc_t *pd);
/*
 * Start a new thread starting execution at address "ip" with parameter "param"
 * "info" specifies optional attributes for the thread creation. A value of
 * NULL will use default thread attributes.
 * "pd" specifies optional APD information. If pd is NULL the newthread shares
 * the callers APD (ie changes done by the new thread affect the 
 * calling threads APD).
 *
 * Return: The new thread ID or THREAD_NULL on error.
 * errors: ST_CLIST, ST_INFO, ST_PROT, ST_OVFL.
 */

int
ThreadInfo ( mthreadid_t thread, threadinfo_t * info /* INOUT */ );
/*
 * Get information about a child thread, copied into the structure pointed to
 * by info.
 *
 * Return: zero if successful, non-zero otherwise.
 * errors: ST_THR, ST_PROT.
 */

int
ThreadDelete ( mthreadid_t thread, int status, bool adopt );
/*
 * Terminates "thread" with an exit value of "status", THREAD_MYSELF
 * terminates the caller.
 * adopt specifies whether children of this thread will still live.
 *
 * Return: zero if successful, otherwise !=0
 * errors: ST_PROT, ST_THR.
 */

int
ThreadSleep ( mthreadid_t thread, time_t time );
/*
 * Make the thread sleep for the amount specified in time in 
 * nanoseconds. A value of zero specifies a thread yield. A value of 
 * SLEEP_INFINITY suspends the thread until a ThreadResume() system call is 
 * performed on the thread.
 *
 * Return: zero if successful, otherwise !=0
 * errors: ST_PROT, ST_THR.
 */

int
ThreadResume ( mthreadid_t thread );
/*
 * Resumes sleeping "thread"
 *
 * Return: zero if successful, otherwise !=0
 * errors: ST_PROT, ST_THR.
 */

mthreadid_t
ThreadWait ( mthreadid_t thread, int * status /* OUT */ );
/*
 * Waits for "thread" to exit, THREAD_ANY means any thread.
 * Returns through "status" the exit status, as specified in the 
 * ThreadDelete system call. 
 *
 * Return: ID of the exited thread. THREAD_NULL on error.
 * errors: ST_PROT, ST_THR.
 */

mthreadid_t
ThreadMyID ( void );
/*
 * Returns ID of calling thread 
 *
 * Return: Callers thread ID.
 * errors: Always successful.
 */



/**************\
 * Exceptions *
\**************/

excpthndlr_t
ExcptReg ( excpt_t exception, excpthndlr_t handler );
/*
 * Registers "handler" for "exception".
 *
 * Return: previous handler or NULL if none was registered.
 * errors: ST_EXCPT
 */

int
GetLastError(void);
/*
 * Returns error status from last system call for the thread.
 * errors: Always successful.
 */


/*****************\
 * Page Mappings *
\*****************/


int
PageCopy ( const void * from, void * to, uint n_pages );
/*
 * Copy memory starting at address "from" to the destination "to", both of
 * which must be page aligned. A total of "n_pages" pages are
 * copied. Copy-on-write is used where possible.  The "from" and "to" ranges
 * must be disjoint. Each range must be fully contained in a single object.
 *
 * Return: zero if successful, otherwise !=0
 * errors: ST_PROT, ST_RNG, ST_NULL.
 */

int
PageMap ( const void * from, void * to, uint n_pages,
          access_t mode, bool fault_in );
/*
 * Make "n_pages" of virtual address space starting at "to" an alias for the
 * virtual address space starting at "from". The address range starting at "to"
 * will be accessible according to "mode". If "fault_in" is true, non-resident
 * pages in the "from" range are faulted in, otherwise the operation is a no-op
 * on such pages.  The two address ranges must either be disjoint or
 * identical. Each range must be fully contained in a single object.
 *
 * Return: zero if successful, otherwise !=0
 * errors: ST_PROT, ST_RNG, ST_NULL. 
 */

int
PageUnMap ( void * page, uint n_pages, pagedisp_t disp );
/*
 * Invalidate virtual memory mappings for "n_pages" pages starting at
 * "page". If the pages belong to an object handled by a user-level pager,
 * their mappings simply vanish, otherwise "disp" determines the semantics of
 * the operation. If disp is P_DSP_ZERO, the pages are zeroed (by
 * disassociating them from any backing store). If disp is P_DSP_REPLACE, the
 * physical frames are freed (without first flushing dirty pages) but the
 * association with backing store is kept. If disp is P_DSP_KEEP, the mappings
 * are retained but marked as invalid, forcing a page fault on the next access.
 * All pages of the range must belong to the same object. If disp is
 * P_DSP_UNALIAS all aliases from the specificied page range (which must be
 * part of a single object) are removed. This is a no-op on unaliased pages.
 *
 * Return: zero if successful, otherwise !=0
 * errors: ST_PROT, ST_RNG, ST_NULL. 
 */

int
PageFlush ( const void * page, uint n_pages );
/*
 * Ensure that the designated range of pages is clean, i.e. all changes are
 * flushed to disk. Also ensures that association with backing store of pages
 * handled by the default pager is recorded on stable storage. The whole page
 * range must belong to a single object.
 *
 * Return: zero if successful, otherwise !=0
 * errors: ST_PROT, ST_RNG, ST_NULL. 
 */



/**************\
 * Semaphores *
\**************/

int
SemCreate ( void * address, int value, int flags );
/*
 * Creates a semaphore identified by "address" and initialised to "value".
 * RW access is requried at address.
 *
 * Return: zero if successful, otherwise !=0
 * errors: ST_USE, ST_PROT, ST_SEMLMT.
 */

int
SemDelete ( void * address );
/*
 * Destroys the semaphore identified by "address".
 *
 * Return: zero if successful, otherwise !=0
 * errors: ST_PROT, ST_SEMA.
 */

int
SemWait ( void * address );
/*
 * Performs a wait operation on the semaphore identified by "address".
 *
 * Return: zero if successful, otherwise !=0
 * errors: ST_PROT, ST_SEMA, ST_SDEL.
 */

int
SemSignal ( void * address );
/*
 * Performs a signal operation on the semaphore identified by "address".
 *
 * Return: zero if successful, otherwise !=0
 * errors: ST_PROT, ST_SEMA.
 */


void
SeedyPrint ( char *msg );


#endif /* __MUNGI_SYSCALLS_H */
