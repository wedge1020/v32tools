#!/bin/bash

# define an abort function to call on error
abort_build()
{
    echo
    echo BUILD FAILED
    exit 1
}

# create obj and bin folders if non exiting, since
# the development tools will not create them themselves
mkdir -p bin
mkdir -p obj

echo
echo Compile the C code
echo --------------------------
compile  -g         trigdemo.c        -o obj/trigdemo.asm  || abort_build

echo
echo Assemble the ASM code
echo --------------------------
assemble -g program obj/trigdemo.asm  -o obj/trigdemo.vbin || abort_build

echo
echo Convert the PNG textures
echo --------------------------
png2vircon textures/texture.png       -o obj/texture.vtex  || abort_build

#echo
#echo Convert the WAV sounds
#echo --------------------------
#wav2vircon sounds/00_title.wav   -o obj/00_title.vsnd     || abort_build

echo
echo Pack the ROM
echo --------------------------
packrom trigdemo.xml         -o bin/trigdemo.v32  || abort_build

echo
echo BUILD SUCCESSFUL
