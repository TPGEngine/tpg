#
# Makefile for acmart package
#
# This file is in public domain
#
# $Id: Makefile,v 1.10 2016/04/14 21:55:57 boris Exp $
#

PACKAGE=acmart

DEV=-dev # To switch dev on
#DEV=

PDF = $(PACKAGE).pdf acmguide.pdf

BIBLATEXFILES= $(wildcard *.bbx) $(wildcard *.cbx) $(wildcard *.dbx) $(wildcard *.lbx)

SAMPLEBIBLATEXFILES=$(patsubst %,samples/%,$(BIBLATEXFILES))

ACMCPSAMPLES= \
	ResearchReport/ResearchReport.pdf \

all:  ${PDF} ALLSAMPLES

%.pdf:  %.dtx   $(PACKAGE).cls
	pdflatex $<
	- bibtex $*
	pdflatex $<
	- makeindex -s gind.ist -o $*.ind $*.idx
	- makeindex -s gglo.ist -o $*.gls $*.glo
	pdflatex $<
	while ( grep -q '^LaTeX Warning: Label(s) may have changed' $*.log) \
	do pdflatex $<; done


acmguide.pdf: $(PACKAGE).dtx $(PACKAGE).cls
	pdflatex -jobname acmguide $(PACKAGE).dtx
	- bibtex acmguide
	pdflatex -jobname acmguide $(PACKAGE).dtx
	while ( grep -q '^LaTeX Warning: Label(s) may have changed' acmguide.log) \
	do pdflatex -jobname acmguide $(PACKAGE).dtx; done

%.cls:   %.ins %.dtx
	pdflatex $<

%-tagged.cls:   %.ins %.dtx
	pdflatex $<


ALLSAMPLES: $(SAMPLEBIBLATEXFILES)
	cd ResearchReport; pdflatex samples.ins; cd ..
	for texfile in ResearchReport/*.tex; do \
		pdffile=$${texfile%.tex}.pdf; \
		${MAKE} $$pdffile; \
	done

samples/%: %
	cp $^ samples


samples/$(PACKAGE).cls: $(PACKAGE).cls
samples/$(PACKAGE)-tagged.cls: $(PACKAGE)-tagged.cls
samples/ACM-Reference-Format.bst: ACM-Reference-Format.bst

samples/abbrev.bib: ACM-Reference-Format.bst
	perl -pe 's/MACRO ({[^}]*}) *\n/MACRO \1/' ACM-Reference-Format.bst \
	| grep MACRO | sed 's/MACRO {/@STRING{/' \
	| sed 's/}  *{/ = /' > samples/abbrev.bib 


samples/%.bbx: %.bbx
samples/%.cbx: %.cbx
samples/%.dbx: %.dbx
samples/%.lbx: %.lbx



ResearchReport/ResearchReport.pdf: ResearchReport/ResearchReport.tex $(SAMPLEBIBLATEXFILES)
	cd $(dir $@) && pdflatex${DEV} $(notdir $<)
	- cd $(dir $@) && biber $(notdir $(basename $<))
	cd $(dir $@) && pdflatex${DEV} $(notdir $<)
	cd $(dir $@) && pdflatex${DEV} $(notdir $<)
	while ( grep -q '^LaTeX Warning: Label(s) may have changed' $(basename $<).log) \
	  do cd $(dir $@) && pdflatex${DEV} $(notdir $<); done


.PRECIOUS:  $(PACKAGE).cfg $(PACKAGE).cls $(PACKAGE)-tagged.cls

docclean:
	$(RM)  *.log *.aux \
	*.cfg *.glo *.idx *.toc \
	*.ilg *.ind *.out *.lof \
	*.lot *.bbl *.blg *.gls *.cut *.hd \
	*.dvi *.ps *.thm *.tgz *.zip *.rpi \
	ResearchReport/$(PACKAGE).cls \
	ResearchReport/$(PACKAGE)-tagged.cls \
	ResearchReport/ACM-Reference-Format.bst \
	ResearchReport/*.log ResearchReport/*.aux ResearchReport/*.out \
	ResearchReport/*.bbl ResearchReport/*.blg ResearchReport/*.cut ResearchReport/*.toc \
	ResearchReport/acm-jdslogo.png \
	ResearchReport/*.run.xml ResearchReport/*.bcf $(SAMPLEBIBLATEXFILES)


clean: docclean
	$(RM)  $(PACKAGE).cls $(PACKAGE)-tagged.cls \
	ResearchReport/*.tex

distclean: clean
	$(RM)  *.pdf ResearchReport/ResearchReport*.pdf

#
# Archive for the distribution. Includes typeset documentation
#
archive:  all clean
	COPYFILE_DISABLE=1 tar -C .. -czvf ../$(PACKAGE).tgz --exclude '*~' --exclude '*.tgz' --exclude '*.zip'  --exclude CVS --exclude '.git*' $(PACKAGE); mv ../$(PACKAGE).tgz .

zip:  all clean
	zip -r  $(PACKAGE).zip * -x '*~' -x '*.tgz' -x '*.zip' -x CVS -x 'CVS/*'

# distros
distros: all docclean
	zip -r acm-distro.zip  \
	acmart.pdf acmguide.pdf samples *.cls ACM-Reference-Format.* \
	--exclude ResearchReport/ResearchReport*
	zip -r acmengage-distro.zip ResearchReport/ResearchReport* \
	ResearchReport/*.bib \
	acmart.pdf acmguide.pdf  *.cls ACM-Reference-Format.*

acmcp.zip: ${ACMCPSAMPLES} acmart.cls
	zip $@ $+

.PHONY: all ALLSAMPLES docclean clean distclean archive zip
