#
# rev.mk
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

# PIM Type (0:none, 1:test, 2:reserved, 3:tclpim)
export PIM_TYPE=3

# REV paths
REVLIBPATH ?= $(PIM_REV_HOME)/build/src

# Test source code
SRCDIR = ./rev-test-src

# SST Libs
SSTOPTS += --add-lib-path=$(REVLIBPATH)

# Output Directories
OUTDIR = rev-output

# Test Selection
PIM_TESTS += $(basename $(wildcard rev-test-src/*.cpp))

# PIM MPI tests
# PIM_MPI_TESTS += 

ifndef MPI_RANKS
  ALL_TESTS = $(PIM_TESTS)
else
  ALL_TESTS = $(PIM_MPI_TESTS)
endif

TLIST ?= $(ALL_TESTS)

# REV Executables
SRCS   := $(basename $(notdir $(wildcard $(SRCDIR)/*.cpp)))
EXES   := $(addprefix $(OUTDIR)/bin/,$(addsuffix .exe,$(SRCS)))
DIASMS := $(patsubst %.exe,%.dis,$(EXES))
SECTIONS := $(patsubst %.exe,%.sections,$(EXES))

# Targets
LOGS   := $(addsuffix /run.log,$(addprefix $(OUTDIR)/,$(TLIST)))
TARGS  := $(LOGS) $(EXES) $(DIASMS) $(SECTIONS)

# Recipes
all: $(TARGS)
run: $(LOGS)
compile: $(EXES) $(DIASMS)

# Rev toolchain
CC=riscv64-unknown-elf-g++
LD=riscv64-unknown-elf-ld
OBJDUMP   = ${RVOBJDUMP} --source -l -dC -Mno-aliases

# Rev headers
INCLUDE  += -I$(PIM_REV_HOME)/common/syscalls
INCLUDE  += -I$(PIM_REV_HOME)/test/include

# PIM headers
INCLUDE  += -I$(PROJHOME)/sstcomp/include

ifdef DEBUG
 CCOPT       ?= -O0 -g -DDEBUG=1
 DEBUG_MODE   = 1
else
 CCOPT       ?= -O2 -g
endif

ifdef DEBUG_MODE
 CCOPT += -DDEBUG_MODE=1
endif

# Configuration
WFLAGS    := -Wall -Wextra -Wuninitialized -pedantic-errors
WFLAGS    += -Wno-unused-function -Wno-unused-parameter
#WFLAGS    += -Werror
# DEBUG_FLAGS := -DDEBUG_MODE
FLAGS     := $(CCOPT) $(WFLAGS) $(DEBUG_FLAGS) -static -lm -fpermissive
ARCH      := rv64gc
# ABI       := -mabi=lp64d

# Test Specific Customization
# if REV_EXE is not specified in OPTS it will default to the test name exe file
# $(OUTDIR)/appTest1/run.log: OPTS += ARGS M

# The magical run command
%.log: $(SSTCFG) compile
	@mkdir -p $(@D)
	@$(eval app = $(notdir $(@D)))
	$(eval exe = $(shell if [ -z ${REV_EXE} ]; then echo $(OUTDIR)/bin/$(app).exe; else echo ${REV_EXE}; fi))
	@$(eval statf = $(basename $@).status)
	@$(eval dotfile = $(basename $@).dot)
	@$(eval pdffile = $(basename $@).pdf)
	@rm -f $(statf) $(dotfile) $(pdffile)
	@echo Running $(basename $@)
	$(OPTS) OUTPUT_DIRECTORY=$(@D) ARCH=$(ARCH) REV_EXE=$(exe) \
 $(MPIOPTS) $(SST) $(SSTOPTS) \
 --output-json=$(@D)/rank.json $(SSTCFG) \
  > $@ && (echo "pass" > $(statf); $(DOT2PDF))

# To run all even if they fail use this instead
#  > $@ && (echo "pass" > $(statf); $(DOT2PDF)) || echo "fail" > $(statf)
# 	@echo "### " $@":" `cat $(statf)`

%.dis: %.exe
	$(OBJDUMP) $< > $@

%.sections: %.exe
	${RVOBJDUMP} -s $< > $@

$(OUTDIR)/bin/%.exe: $(OUTDIR)/bin/%.o $(SRCDIR)/pim.lds
	$(LD) -o $@ -T $(SRCDIR)/pim.lds $<

$(OUTDIR)/bin/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(@D)
	$(CC) $(FLAGS) -o $@ -g -c $< -march=$(ARCH) $(ABI) $(INCLUDE) $(LIBS)

# workaround for rev_openat on ubuntu
.PHONY: %.dat
%.dat:
	mkdir $(@D)
	touch $@

clean:
	rm -rf $(OUTDIR)

help:
	@echo make compile
	@echo make run
	@echo make TLIST="test1 test2 ..."
	@echo Valid TLIST selections are:
	@echo $(ALL_TESTS)
	@echo

.PHONY: run clean help

.SECONDARY:

.PRECIOUS: %.log

#-- EOF
