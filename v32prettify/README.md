# v32prettify

A BASH-based Vircon32 assembly code prettifier

## USAGE

Basic operation will process any Vircon32 assembly source file, including
prior to  any pre-processing has  taken place (in fact,  the prettifier's
prime use is in cleaning up your actual code, which will necessarily have
all of the pre-processor directives).

The  prettifier  will either  read  code  from  STDIN  (if no  files  are
specified), or will  process a list of files, writing  the results out to
STDOUT when done.


```
 v32prettify - bash-based Vircon32 assembly language code prettifier

        usage: prettify [ASMFILE..]

     options - description
  ==========   =====================================================
    noblanks - filter out blank lines
  nocomments - filter out comments
     verbose - enable increased verbosity
        help - display this help and exit

  NOTE: prettify does not modify the original source file, and
        it will currently just display the  resulting data  to
        STDOUT
```
