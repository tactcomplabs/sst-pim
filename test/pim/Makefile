#
# Makefile pim
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#
# Synopsis: This include file allows running simulations using
#           the REV CPU or Appx components. Select the REV CPU
#           by using 'export REV=1'. Default is Appx

# Project root
PROJHOME ?= $(realpath ../..)

# PIM Compatible REV
# PIM_REV_HOME ?= $(HOME)/work/rev-pim
ifndef PIM_REV_HOME
 $(error PIM_REV_HOME must be set to a PIM compatable REV)
endif

# SST
ifdef VALGRIND
 SST ?= valgrind sst
else
 SST = sst
endif

# SST configuration
SSTCFG ?= $(PROJHOME)/test/configs/pim-simple.py

# Path to pim backend sst lib
SSTOPTS += --add-lib-path=$(PROJHOME)/sstcomp/PIMBackend

# MPI
ifdef MPI_RANKS
 MPIOPTS ?= mpirun -n $(MPI_RANKS) --use-hwthread-cpus
 SSTOPTS += --parallel-output=1
endif

# Graph PDF
ifdef DOT
SSTOPTS += --output-dot=$(dotfile) --dot-verbosity=10
DOT2PDF = dot -Tpdf $(dotfile) -o $(pdffile)
endif

# Include application driver recipes
ifndef REV
 include mkinc/appx.mk
else
 include mkinc/rev.mk
endif

