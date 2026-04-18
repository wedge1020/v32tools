# v32as

This is a  proof-of-concept BASH-based Vircon32 assembler.  Yes, you read
that right:  an obscenely  high-level shell  script performing  among the
lowest-level of data transactions.

It is powered and heavily reliant  upon Regular Expressions, which do the
bulk of the heavy lifting.

Ultimately  performance  doesn't  really   matter,  as  assembling  isn't
something needing to be done multiple times a second.

## BUILDING

Since this is a BASH script, there  is no building step, simply make sure
you have all the needed prerequisite tools, and install it.

## DEPENDENCIES

In order for `v32as` to run, you will need the following:

### BINCODE

You need  to build and install  the `bincode` tool, also  from within the
`v32tools` suite.

### BC

Many numerical  manipulations are  mathematically performed via  the `bc`
"binary calculator" tool.

### STANDARD UNIX TOOLS

  * `cut`(1)
  * `grep`(1) and `egrep`(1)
  * `sed`(1)
  * `tr`(1)
  * `wc`(1)

## INSTALL

To install, from this subdirectory, run: `make install`

Tool will install into current user's `~/bin/` directory.
