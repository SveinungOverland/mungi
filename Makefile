#      
#      $Id: Makefile,v 1.25.2.1 2002/08/29 04:30:49 cgray Exp $
#	Copyright (C) 1999 Distributed Systems (DiSy) Group, UNSW, Australia.
#
#      This file is part of the Mungi operating system distribution.
#
#      This program is free software; you can redistribute it and/or
#      modify it under the terms of the GNU General Public License
#      as published by the Free Software Foundation; either version 2
#      of the License, or (at your option) any later version.
#      
#      This program is distributed in the hope that it will be useful,
#      but WITHOUT ANY WARRANTY; without even the implied warranty of
#      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#      GNU General Public License for more details.
#      
#      You should have received a copy of the GNU General Public License
#      along with this program; if not, write to the Free Software
#      Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#      
#
#	Requires GNU make


# Where to find source. Each of these dirs should contain a file
# subdir.mk which defines what needs to be built
SUBDIRS = kernel/lib/klibc kernel/lib/upcall drivers kernel/src apps \
               lib/mungi kernel/src/vm alpha_serial
LIBSUBS =  lib/libc/stdlib lib/libc/string lib/libc/stdio syscalls \
                lib/libc/posix lib/mlib lib/naming

# objects to build
TARGET	= mungi
KERNEL	= mungi.kernel
APP  	= mungiapp
UPCALL  = upcall
RPAGER  = rpager
SERIAL  = serial
LIBMSYS = libmsys.a


# Build rules
.PHONY: clean tags distclean buildno

all: $(TARGET)

include Makefile.conf $(patsubst %,%/subdir.mk,$(SUBDIRS)) \
	$(patsubst %,%/subdir.mk, $(LIBSUBS))


# Make up user-land commands
UCMDS = echo "Linking " $(APPNAME) &&\
	$(LD) $(LDFLAGS) -e __start -Ttext `$(DITE) -l $(TARGET).tmp | perl -e 'printf "0x%x",(int((hex(<>)-1)/0x10000)*0x10000+0x10000);'` -o $($(APPNAME)_NAME)  \
		$(CRTS) $($(APPNAME)_OBJ) $(SUBTARGETS2)        \
		$(MLDDIRS) $(LDDIRS) $(MLDLIBS) $(LDLIBS)      &&\
                $(DITE) $(DITEFLAGS) $($(APPNAME)_NAME) $(DITEOUT) &&

UCMDSEND = echo 'Done'
USERLAND_CMDS := $(foreach APPNAME,$(USERLAND_VARS),$(UCMDS)) $(UCMDSEND)


# Main Kernel 
$(TARGET): .build.$(KTARGET) $(L4KERNEL) $(SUBTARGETS) $(SUBTARGETS2) $(APP_OBJ) $(CRTS) $(SER_OBJ)
	$(CP) $(L4KERNEL) $@.tmp
ifeq (${MUNGI_ARCH}, alpha)
	@echo
	@echo "Linking the serial driver"
	@echo
	$(LD) $(LDFLAGS) -Ttext `$(DITE) -l $@.tmp` \
		-e start -o $(SERIAL) \
		$(SER_OBJ) $(LDDIRS) $(LDLIBS) $(KLDDIRS) $(KLDLIBS)
	$(DITE) $(DITEFLAGS) -x $(SERIAL) $(DITEOUT)
endif
	@echo
	@echo "Linking the Mungi kernel"
	@echo
	$(LD) $(LDFLAGS) -Ttext `$(DITE) -l $@.tmp` \
		-e __start -o $(KERNEL) \
		$(KOBJ) $(LDDIRS) $(KLDDIRS) $(KLDLIBS) $(LDLIBS)
	$(DITE) $(DITEFLAGS) $(KERNEL) $(DITEOUT)
	@echo
	@echo "Linking the upcall lib"
	@echo
	$(LD) $(LDFLAGS) -Ttext  `$(DITE) -l $@.tmp`  -e __start -o $(UPCALL) \
		$(UPCALL_OBJ) $(LDDIRS) $(LDLIBS) $(UPCALL_USERPRINT_OBJ)
	$(DITE) $(DITEFLAGS)  $(UPCALL) $(DITEOUT)
	@echo
	@echo "Linking RAM Pager"
	@echo
	$(LD) $(LDFLAGS) -Ttext `$(DITE) -l $@.tmp` -o $(RPAGER) $(RPOBJ) \
		 $(LDDIRS) $(KLDDIRS) $(KLDLIBS) $(LDLIBS)
	$(DITE) $(DITERESOURCEFLAGS) $(RPAGER) $(DITEOUT)
	#
	#
	@echo
	@echo "Linking the user applications"
	@echo
	$(USERLAND_CMDS)
	@echo
	@echo "Creating Mungi user-land archive"
	@echo
	rm -f $(LIBMSYS)
	$(AR) cru $(LIBMSYS) $(CRTS) $(SUBTARGETS2)
	#
	chmod a+r $@.tmp
	cat $(ALPHABOOTLOADER) $@.tmp > $@
	chmod a+r $@
	$(RCP) $@ $(kerneldest)
	rm $@
	touch .build.$(KTARGET)
	#@$(MAKE) buildno

.build.$(KTARGET): 
	make clean
	touch .build.$(KTARGET)

.lastbuild.$(KTARGET) : clean

TAGS:
	$(TAGS) $(MUNGIROOT) $(L4ROOT)

clean:
	$(RM) $(TARGET) $(TARGET).tmp $(KERNEL) $(SUBTARGETS) $(SUBTARGETS2) \
		$(OBJ) log tags $(RPAGER) $(SERIAL) $(UPCALL) $(APP)         \
		$(USERLAND) *~ .build.*

distclean realclean: clean
	find . -type f -name '*.o' -exec $(RM) {} \; \
	-o -name core -exec $(RM) {} \; \
	-o -name '*.d' -exec $(RM) {} \; \
	-o -name '.*.swp' -exec $(RM) {} \; \
	-o -name '*~' -exec $(RM) {} \; \
	-o -name '*.a' -exec $(RM) {} \;

# Auto dependency generation
ifneq ($(findstring clean,$(MAKECMDGOALS)),clean)
include $(patsubst %.c,%.d,$(filter %.c,$(SRC))) \
	$(patsubst %.S,%.d,$(filter %.S,$(SRC)))
endif

%.d:	%.c
	@echo Building dependencies for $<
	@$(SHELL) -ec \
		'$(CC) -MM $(CFLAGS) $(INCLUDES) $< \
                | sed '\''s,$(*F)\.o,& $@,'\'' > $@'

%.d:	%.S
	@echo Building dependencies for $<
	@$(SHELL) -ec \
		'$(CC) -MM $(CFLAGS) $(INCLUDES) $< \
                | sed '\''s,$(*F)\.o,& $@,'\'' > $@'

%.o:	%.c %.d
	$(CC) -c  $(CFLAGS) -o $@ $(INCLUDES) $<

%.o:	%.S %.d
	$(AS) -c $(ASFLAGS) -o $@ $(INCLUDES) $<

%.o:	%.c
	@echo "Hmmm... Some C dependencies seem to be broken: " $@

%.o:	%.S
	@echo "Hmmm... Some ASM dependencies seem to be broken: " $@

# This little script updates the build number of my Mungi 
# kernel everytime that I build it.
buildno:
	@echo Updating build number
	@if [ -f $(MUNGIROOT)/kernel/include/mungi/version.h ]; then \
		echo "/* this is an automatically generated file */">.tmpver ;\
		cat $(MUNGIROOT)/kernel/include/mungi/version.h | \
		grep MUNGIVERSIONH | cut -c 23- > .hver; \
		expr 0`cat $(MUNGIROOT)/kernel/include/mungi/version.h | \
		grep MUNGIVERSIONL | cut -c 23-` + 1 > .lver; \
		echo "#define MUNGIVERSIONH" `cat .hver` >> .tmpver ; \
		echo "#define MUNGIVERSIONL" `cat .lver` >> .tmpver ; \
		mv .tmpver $(MUNGIROOT)/kernel/include/mungi/version.h ;\
		$(RM) .lver .hver ;\
	fi
