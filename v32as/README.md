# v32as

This is a  proof-of-concept BASH-based Vircon32 assembler.  Yes, you read
that right:  an obscenely  high-level shell  script performing  among the
lowest-level of data transactions.

It is powered and heavily reliant  upon Regular Expressions, which do the
bulk of the heavy lifting.

Ultimately  performance  doesn't  really   matter,  as  assembling  isn't
something needing to be done multiple times a second.

## USAGE

```
             v32as - Vircon32 assembler written in BASH, powered by
                     Regular Expressions

              usage: v32as [OPTION..] [ASMFILE..]

            option - description
 =================   ===================================================
              bios   assemble for BIOS
              cart   assemble for CART
              none   standalone instruction assembly (default)
              step   pause after each instruction
            binary   output resulting binary machine code to STDOUT
           verbose   increase verbosity
              help   display usage information and exit

 NOTE:  It   is  meant   for  v32pp,   the  v32tools   Vircon32  Assembly
 Pre-Processor,  to  be  run  prior  to passing  the  code  through  this
 assembler. Furthermore,  for the  production of an  actual VBIN  file, a
 further tool  is required. This  assembler will ONLY convert  to machine
 code, and nothing more.
```

The assembler can take  data via STDIN, or by specifying  one or more ASM
files on the command-line.  It will process them one at  a time, in order
of specification, and output the combined results to STDOUT.

Note that,  having the  preprocessor be a  separate tool,  `v32as` cannot
handle ANY preprocessor  directives in its input.  Things like `%include`
and `%define`, when encountered, will result in errors being triggered.

## BUILDING

Since this is a BASH script, there  is no building step, simply make sure
you have all the needed prerequisite tools, and install it.

## DEPENDENCIES

In order for `v32as` to run, you will need the following:

### BINCODE

You need  to build and install  the `bincode` tool, also  from within the
`v32tools` suite. This  tool will do the actual generation  of the binary
data; the `v32as` script proper generates purely text data.

### BC

Many numerical  manipulations are  mathematically performed via  the `bc`
"binary calculator" tool. Typically used for base conversions.

### STANDARD UNIX TOOLS

  * `cut`(1)
  * `grep`(1) and `egrep`(1)
  * `sed`(1)
  * `tr`(1)
  * `wc`(1)

## INSTALL

To install, from this subdirectory, run: `make install`

Tool will install into current user's `~/bin/` directory.
