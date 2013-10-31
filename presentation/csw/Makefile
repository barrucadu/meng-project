IMAGES := $(wildcard *.dot)

.PHONY: all clean

all: slides.pdf

clean:
	-@rm slides.pdf $(IMAGES:.dot=.eps)

slides.pdf: slides.tex $(IMAGES:.dot=.eps)
	@rubber -d slides.tex

%.eps: %.dot
	@dot -Teps $^ > $@
