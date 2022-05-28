CC=g++
IDIR=./include
TEMPDEPTH=2000000
CFLAGS=-std=c++20 -I$(IDIR) -ftemplate-depth=$(TEMPDEPTH) -Ofast -Wno-narrowing -fopenmp
IDEPS=$(wildcard $(IDIR)/*)
FDEPS=
DEPS=$(IDEPS) $(FDEPS)

%: ./src/%.cpp $(DEPS)
	$(CC) $(CFLAGS) $< -lpng -o ./bin/$@

