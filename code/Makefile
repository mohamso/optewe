CPPFLAGS += -Iinclude -Ix86_dvfs/include -Ihdeem/include -Inemi/include ${INSTRUMENTATION}
CXXFLAGS += -std=c++14 -Wall -O2 -qopenmp -ipo
LDFLAGS += -L/usr/local/lib -lx86_adapt -lhdeem -lfreeipmi -lrt

X86DVFS_SRC = \
	x86_dvfs/src/x86_dvfs.cc \

NEMI_SRC = \
	nemi/src/nemi.cc \

OPTEWEMP_SRC = \
	src/differentiators.cc \
	src/dims.cc \
	src/fd3d.cc \
	src/mem_utils.cc \
	src/main.cc \
	src/model3d.cc \
	src/print.cc \
	src/receiver3d.cc \
	src/source.cc \
	src/step_forward.cc \
	src/vtk.cc \
	${NEMI_SRC} \
	${X86DVFS_SRC} \

BINDIR=bin
OPTEWEMP=$(BINDIR)/optewe-mp
BINARY=$(OPTEWEMP)

all: $(BINARY)

$(OPTEWEMP): $(patsubst %.cc, %.o, $(OPTEWEMP_SRC))
	mkdir -p $(BINDIR)
	$(CXX) -o $@ $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $^

%.o: %.cc
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) -o $@ $<

clean:
	find . -name "*.o" | xargs rm -rf
	rm -rf $(BIN)
