# $Id: Makefile,v 1.9 2002/04/29 09:39:48 gernot Exp $
# Makefile for Mungi API doc. Written by Gernot.
#
# This is only meant to be used at UNSW, it depends on local style and
# bibliography files.

INSTALL=/home/disy/Mungi
ROOT=api

SRC=$(ROOT).tex syscalls.h types.h status.h exception.h
DOCDIR=$(INSTALL)/doc
INCLDIR=../../include
API=$(DOCDIR)/$(ROOT).pdf.gz
DEFTEXINPUTS=/home/disy/lib/TeX//:/home/disy/lib/ps
DEFBIBINPUTS=/home/disy/lib/BibTeX
DEFDVIPSHEADERS=/home/disy/lib/ps

all:		$(ROOT).pdf

syscalls.h:	$(INCLDIR)/syscalls.h
		expand <$(INCLDIR)/$@ >$@

status.h:	$(INCLDIR)/status.h
		expand <$(INCLDIR)/$@ >$@

exception.h:	$(INCLDIR)/exception.h
		expand <$(INCLDIR)/$@ >$@

types.h:	$(INCLDIR)/sys/types.h
		expand <$(INCLDIR)/sys/$@ >$@

install:	$(ROOT).pdf
		-mkdir -p $(DOCDIR)
		gzip --best <$(ROOT).pdf >$(UMAN)

$(ROOT).pdf:	$(ROOT).ps
		ps2pdf12 $(ROOT).ps

$(ROOT).ps:	$(ROOT).dvi
		DVIPSHEADERS=$$DVIPSHEADERS::$(DEFDVIPSHEADERS) \
			dvips -Pwww -o $(ROOT).ps $(ROOT)

$(ROOT).dvi:	$(SRC)
		TEXINPUTS=$$TEXINPUTS::$(DEFTEXINPUTS) latex $(ROOT)
		BIBINPUTS=$$BIBINPUTS::$(DEFBIBINPUTS) bibtex $(ROOT)
		TEXINPUTS=$$TEXINPUTS::$(DEFTEXINPUTS) latex $(ROOT)
		TEXINPUTS=$$TEXINPUTS::$(DEFTEXINPUTS) latex $(ROOT)

clean:
		rm -f *.log *.dvi *.aux *.brf *.out *.ps *.pdf *.toc *.blg \
			*.cb *~ *.h

realclean:	clean
		-rm -fr  $(DOCDIR)

depend:
