TEXFILES := $(wildcard *.tex)
BIBFILES := references.bib
GLOFILES := glossary.glo

.PHONY: all clean

all: dissertation.pdf

dissertation.pdf: $(TEXFILES) $(BIBFILES) $(GLOFILES)
	@xelatex dissertation.tex >/dev/null
	@bibtex dissertation >/dev/null
	@makeglossaries dissertation >/dev/null
	@xelatex dissertation.tex >/dev/null
	@xelatex dissertation.tex >/dev/null

clean:
	-@rm -r auto *.aux *.bbl *.blg *.glg dissertation.glo *.gls *.idx *.log *.out *.pdf *.toc *.xdy &>/dev/null
