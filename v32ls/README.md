# v32ls

Like the UNIX `ls` tool, **v32ls**  will output the found V32 headers and
common  attributes for  each  section.  Much as  **v32cat**  can do,  but
designed to be in a more top-down readable and concise format.

The idea is that  you can get a quick overview  of a particular cartridge
with **v32ls**, then take that information (the `offsets`) and apply them
to your pursuits with **v32cat**.

**v32ls** should  also be more  performant than **v32cat**  in traversing
the file, due to not needing to encounter each and every byte/word inside
each section. Through the use of  `fseek(3)`, and being able to calculate
the total  size of the  current section,  seeks are performed  to quickly
jump from section to section.

### default

By default, **v32ls** will locate and display (in order from the start of
the file): each V32 header, and its starting offset.

### verbosity

With the inclusion of the `-l`, `-v`, or `--verbose` argument, additional
V32 header information will be displayed.

## BUILDING

To build this v32tool, change into the respective tool's subdirectory and
run `make`

Executable files will be stored in `bin/`.

## INSTALL

To install, also from the tool's subdirectory, run `make install`

Tool will install into current user's `~/bin/` directory.
