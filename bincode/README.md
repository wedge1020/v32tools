# bincode

The `bincode` tool is a handy utility for handling all kinds of low-level
binary transactions,  that may  otherwise require  a number  of different
tools and programs to accomplish, all integrated within one program.

It features:

  * display integer as binary data (`--offset` or `--word`)
  * read text file of integers as binary data, one per line (`--file`)
  * display string as binary data (`--string`)
  * display float as binary data (`--float`)
  * decode base64-encoded data (`--decode-base64`)
  * negate integer value (two's complement, `--negate`)
  * encode data in little endian format (`--little-endian`)
  * encode data in big endian format (default, `--big-endian`)
  * display as text vs binary (`--text`)

As part of `v32tools`, it also supports transactions specific to Vircon32
data,  including `--header`  to produce  a custom  "V32-XXXX" header  for
embedding within Vircon32 binary files like a VBIN or CART.

It  is   theoretically  possible  (although  certainly   inefficient)  to
construct an entire V32 CART from `bincode` transactions.

## INTEGERS

One of  the three categories  of data that `bincode`  transacts, integers
can be used as offsets or merely just machine words.

## FLOATS

Another  of the  data categories,  the purpose  of the  float transaction
within `bincode`  is to obtain the  raw hexadecimal value of  the IEEE754
float provided.

## STRINGS

The third  of the  data categories,  by default  `bincode` will  prefix a
dataword containing  the number of  datawords the string  occupies, since
strings are processed in units of bytes, packing data words.

An optional `base  64` decoding can be performed on  string data with the
`--decode-base64` argument.

## BUILDING

To build this v32tool, run `make` from within its directory.

Executable files will be stored in `bin/`.

## INSTALL

To install, also from the tool's subdirectory, run `make install`

Tool will install into current user's `~/bin/` directory.
