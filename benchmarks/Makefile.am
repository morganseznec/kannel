benchformats = \
    benchmarks/report.pdf \
    benchmarks/report.ps \
    benchmarks/report.html
benchscripts = benchmarks/run-benchmarks $(wildcard benchmarks/*.sh)
benchoutputs = \
    $(benchformats) \
    $(wildcard benchmarks/*.ps) \
    $(wildcard benchmarks/*.png) \
    $(wildcard benchmarks/*.xml)

all:

bench: all $(benchformats) $(SUBDIRS)
benchmarks/report.xml:
	benchmarks/run-benchmarks benchmarks/*.sh

clean: $(SUBDIRS)
	rm -f $(benchoutputs)
