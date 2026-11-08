# v32tools -- meta Makefile
#
# Builds, tests,  installs and  removes the  Vircon32 tools  under tools/
# from one  place, each with its  own native build system.  The tool list
# and build order live in  tools.list; docs/USAGE.md has the full manual,
# `make help` the short one.
#
# Needs GNU make (on BSD/macOS-without-GNU: `gmake`; on Windows: MSYS2/MinGW).

.DEFAULT_GOAL := help
SHELL         := /bin/sh
.NOTPARALLEL:   # tools are processed one after the other; -j still reaches each tool's own make

# ---- keep our selectors out of the tools' builds ---------------------------
#
# Command-line variables normally reach  every sub-make through MAKEFLAGS
# and  would OVERRIDE  the tools'  own assignments.  Clear that  channel;
# PREFIX, DESTDIR,  DEST_LANG, ... still arrive  through the environment,
# where the tools' `?=` defaults honour them.
#
MAKEOVERRIDES =

# ---- user-facing knobs -----------------------------------------------------
TOOLS       ?= all
SKIP        ?=
BACKEND     ?= make
BUILD_TYPE  ?= Release
USER_PREFIX ?= $(HOME)
JOBS        ?=
FLEX        ?= auto

# (after the defaults: unexport on an undefined variable would define it, empty)
unexport TOOLS SKIP BACKEND BUILD_TYPE USER_PREFIX JOBS ARCHIVE TOOL FORCE

comma := ,
empty :=
space := $(empty) $(empty)

# ---- the manifest ------------------------------------------------------------------------
MANIFEST  := tools.list
ALL_TOOLS := $(shell awk '!/^[ \t]*(\#|$$)/ { print $$1 }' $(MANIFEST))
tool_field = $(shell awk -v t='$(1)' '!/^[ \t]*\#/ && $$1 == t { print $$$(2); exit }' $(MANIFEST))

$(foreach t,$(ALL_TOOLS),\
  $(eval PRESENT_$(t) := $(if $(or $(wildcard tools/$(t)/Makefile),$(wildcard tools/$(t)/CMakeLists.txt)),yes)))

# ---- which tools -------------------------------------------------------------------------
_want := $(subst $(comma),$(space),$(TOOLS))
ifneq ($(filter all,$(_want)),)
  _want := $(ALL_TOOLS)
endif
_unknown := $(filter-out $(ALL_TOOLS),$(_want))
ifneq ($(_unknown),)
  $(error unknown tool(s): $(_unknown). Known: $(ALL_TOOLS))
endif
SEL    := $(filter-out $(subst $(comma),$(space),$(SKIP)),$(filter $(_want),$(ALL_TOOLS)))
RUNSEL := $(foreach t,$(SEL),$(if $(PRESENT_$(t)),$(t)))
ABSENT := $(filter-out $(RUNSEL),$(SEL))

# ---- environment for scripts/v32t-run.sh -------------------------------------------------
export V32T_BACKEND     = $(BACKEND)
export V32T_BUILD_TYPE  = $(BUILD_TYPE)
export V32T_USER_PREFIX = $(USER_PREFIX)
export V32T_JOBS        = $(JOBS)
export V32T_FLEX        = $(FLEX)
export MAKE

RUN := scripts/v32t-run.sh

# ---- targets -----------------------------------------------------------------------------
ACTIONS := build debug clean distclean test install uninstall sysinstall sysuninstall

.PHONY: help list doctor status bootstrap update import flex flex-rebuild all $(ACTIONS) \
        $(ALL_TOOLS) $(foreach a,$(ACTIONS),$(foreach t,$(ALL_TOOLS),$(t)-$(a)))

help:
	@sed -n '/^#HELP/,/^#ENDHELP/{/^#HELP/d;/^#ENDHELP/d;s/^# \{0,1\}//;p;}' $(firstword $(MAKEFILE_LIST))

#HELP
# v32tools -- build / test / install the Vircon32 tools in tools/
#
#   make build | clean | distclean | test | install | uninstall |
#        sysinstall | sysuninstall | debug         the step, for every tool
#   make <tool>                                    build one tool
#   make <tool>-<step>                             one step of one tool   (make v32as-clean)
#
#   TOOLS="v32as v32opt"   only these (comma or space separated; default: all)
#   SKIP=NAME              leave these out
#   BACKEND=make|cmake|auto  tool build system (default make; cmake/auto use a tool's
#                          CMakeLists.txt when it has one)
#   PREFIX=/opt/vircon32   system install location (sysinstall); DESTDIR=... for packaging
#   USER_PREFIX=~/x        CMake backend's user-level `install` prefix (default $HOME)
#   BUILD_TYPE=Release     CMake backend only
#   JOBS=8                 parallel jobs inside each tool
#   FLEX=off               never build the vendored flex
#   (other variables, e.g. DEST_LANG=es or COMPONENTS=both, reach the tools unchanged)
#
#   install      per-user install (the tools' own ~/bin convention)
#   sysinstall   system-wide install (default /usr/local; may need sudo)
#
#   make list        tools, state and descriptions
#   make doctor      check the host for compilers, flex, bison, cmake, ...
#   make bootstrap   fetch tools/* as git submodules (TOOLS=... for some)
#   make update      move every submodule to its remote branch tip
#   make status      git state of every tool
#   make import ARCHIVE=v32as-project.zip [TOOL=v32as]   unpack a snapshot into tools/
#   make flex        build the vendored flex into build/deps (done automatically if needed)
#
# See docs/USAGE.md. CMake users: the same operations are `cmake --build build --target ...`.
#ENDHELP

list:
	@printf '%-8s %-8s %-11s %s\n' TOOL STATE BACKENDS DESCRIPTION
	@for t in $(ALL_TOOLS); do \
	    if [ -f tools/$$t/Makefile ] || [ -f tools/$$t/CMakeLists.txt ]; then s=present; else s=absent; fi; \
	    b=; [ -f tools/$$t/Makefile ] && b=make; [ -f tools/$$t/CMakeLists.txt ] && b=$${b:+$$b,}cmake; \
	    d=$$(awk -v t=$$t '!/^[ \t]*#/ && $$1==t { $$1=$$2=$$3=$$4=""; sub(/^ +/,""); print; exit }' $(MANIFEST)); \
	    printf '%-8s %-8s %-11s %s\n' $$t $$s "$${b:--}" "$$d"; \
	done

doctor:
	@scripts/v32t-doctor.sh

status:
	@git submodule status 2>/dev/null || true
	@for t in $(ALL_TOOLS); do [ -e tools/$$t/.git ] || continue; \
	    printf '%-8s %s\n' $$t "$$(git -C tools/$$t describe --always --dirty 2>/dev/null) ($$(git -C tools/$$t rev-parse --abbrev-ref HEAD 2>/dev/null))"; done

bootstrap:
	@scripts/v32t-bootstrap.sh $(if $(filter-out all,$(TOOLS)),$(SEL))

update:
	git submodule update --init --remote --merge $(foreach t,$(SEL),tools/$(t))

import:
	@test -n "$(ARCHIVE)" || { echo "usage: make import ARCHIVE=path/to/v32as-project.zip [TOOL=name] [FORCE=1]"; exit 2; }
	@FORCE="$(FORCE)" scripts/v32t-import.sh "$(ARCHIVE)" $(TOOL)

flex:
	@scripts/v32t-flex.sh
flex-rebuild:
	@scripts/v32t-flex.sh --force

# ---- per-step aggregates and per-tool rules ----------------------------------------------
.PHONY: _note
_note:
	@$(if $(ABSENT),echo "note: skipping $(ABSENT): tools/<name> is empty (make bootstrap | make import)",:)

all: build

define AGGREGATE
$(1): _note $$(foreach t,$$(RUNSEL),$$(t)-$(1))
endef
$(foreach a,$(ACTIONS),$(eval $(call AGGREGATE,$(a))))

define TOOL_RULE
$(1)-$(2):
	+@$$(RUN) $(1) $(2)
endef
$(foreach t,$(ALL_TOOLS),$(foreach a,$(ACTIONS),$(eval $(call TOOL_RULE,$(t),$(a)))))
$(foreach t,$(ALL_TOOLS),$(eval $(t): $(t)-build))

clean:
	@rm -f ../v32tools-project.zip *.zip *.patch

archive: clean
	@zip -r ../v32tools-project.zip *
