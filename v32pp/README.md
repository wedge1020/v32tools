# v32pp

A BASH-based  preprocessor, for  use with  Vircon32 assembly  code files,
specifically for use to `v32as` (although should be able to work with any
assembler). It makes  use of Regular Expressions and  the M4 preprocessor
to pull off its sorcery.

Currently supports:

  * `%include` for including other files (works, including for nested)
  * `%define` for defining symbols, and some added variants:
    * `%define SYMBOL` works
    * `%define SYMBOL value` works
  * `%eval` for defining symbols, the result makes use of pre-calculation
    * `%eval SYMBOL OTHERSYMBOL+SOMEVALUE` works
    * `%eval SYMBOL OTHERSYMBOL-SOMEVALUE` works
    * macros have not been looked at yet, so that's a negatory
  * `ifdef` should work (no nesting), terminated by a `%endif`
  * `ifndef` works (no nesting), terminated by a `%endif`

## USAGE

Basic operation will process  any supported pre-processor directive (with
the leading `%`), producing to STDOUT  the resulting data (which can then
be redirected to a file, or piped into `v32as` for assembly):


```
      v32pp - bash-based Vircon32 assembly language preprocessor

       usage: v32pp ASMFILE.ASM

    options - description
 ==========   ==================================================
   noblanks - filter out blank lines
 nocomments - filter out comments
    verbose - enable increased verbosity
       help - display this help and exit

 This preprocessor supports the following directives:

     %include - load file contents in place
     %define  - define symbol, replace with value
     %eval    - like define, but will do simple math
     %ifdef   - single-level preprocessor if defined
     %ifndef  - single-level preprocessor if not defined
     %endif   - close the preprocessor if block

 NOTE: v32pp does not modify the original source file, and it
       will currently  just display the resulting data,  that
       which is post-processed, to STDOUT
```
