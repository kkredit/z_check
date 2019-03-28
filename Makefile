# Credit to Stewart Hildebrand for this Makefile template.
# http://www.stew.dk/make

CC=gcc
CXX=g++

SRC:= \
	zf_check/zf_check.c \
	examples/example.c
INCDIRS:= \
	. \
	zf_log \
	zf_check

BUILDDIR:=build
EXENAME:=$(BUILDDIR)/example

vpath %.c $(dir $(SRC))
vpath %.cpp $(dir $(SRC))

OBJS:=$(patsubst %.c,$(BUILDDIR)/%.o,$(patsubst %.cpp,$(BUILDDIR)/%.o,$(notdir $(SRC))))
GCOVGCNO:=$(patsubst %.o,$(BUILDDIR)/%.gcno,$(notdir $(OBJS)))
GCOVGCDA:=$(patsubst %.o,$(BUILDDIR)/%.gcda,$(notdir $(OBJS)))

CFLAGS+=-Wall -Wextra -Wpedantic -Werror \
		-Wlogical-op -Waggregate-return -Wfloat-equal -Wcast-align \
		-Wparentheses -Wmissing-braces -Wconversion -Wsign-conversion \
		-Wwrite-strings -Wunknown-pragmas \
		-Wnested-externs -Wpointer-arith -Wswitch -Wredundant-decls \
		-Wreturn-type -Wshadow -Wstrict-prototypes -Wunused -Wuninitialized \
		-Wdeclaration-after-statement -Wmissing-prototypes \
		-Wmissing-declarations -Wundef -fstrict-aliasing -Wstrict-aliasing=3 \
		-Wformat=2 \
		-O0 -ggdb3 \
		-std=c99 -D_POSIX_C_SOURCE=200112L
#CFLAGS +=-Wunused-macros
LDFLAGS+= $(foreach dir,$(INCDIRS),-I$(dir))

.PHONY: all
all:
	$(MAKE) $(EXENAME) -j $(shell nproc)

.PHONY: coverage
coverage:
	CFLAGS=--coverage $(MAKE) $(EXENAME) -j $(shell nproc)
	./$(EXENAME)
	lcov -c -d . -o $(BUILDDIR)/$(EXENAME).info
	genhtml --legend -o $(BUILDDIR)/coveragereport $(BUILDDIR)/$(EXENAME).info

$(EXENAME): $(OBJS)
	$(CXX) -o $@ $(CFLAGS) $^ $(LDFLAGS)

$(BUILDDIR)/%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $< $(LDFLAGS)

$(BUILDDIR)/%.o: %.cpp
	$(CXX) -c -o $@ $(CFLAGS) $< $(LDFLAGS)

$(OBJS): | $(BUILDDIR)

$(BUILDDIR):
	mkdir $(BUILDDIR)

.PHONY: clean
clean:
	$(RM) $(EXENAME) $(OBJS) $(GCOVGCNO) $(GCOVGCDA) $(BUILDDIR)/$(EXENAME).info
	$(RM) -r $(BUILDDIR)/coveragereport
