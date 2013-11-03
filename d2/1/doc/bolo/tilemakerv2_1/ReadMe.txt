===============
TileMaker v 2.1
By Paul SIRAMY
Freeware
08/15/2004
===============

TileMaker2.exe takes an image and split it into Diablo II FLOOR Tiles,
making 2 files in the process, the DT1 and the associated DS1.

As this is a command-line program, you need to gives parameters to the
program under a ms-dos console window, or with a .BAT. The syntaxe of
the program is as follow :


   TileMaker2 <image> <main index> <act> [<floor layer>]


<image>       is the path of an image (BMP, PCX or TGA) that MUST be in a
              256 colors mode, using a Diablo II palette.

<main index>  is a value between 0 and 63 which is the starting main-index
              value the program will use for the tiles numeration.

<act>         is the Act value to put into the DS1 file : 1, 2, 3, 4, or 5

<floor layer> is optional. If present, it can be 1 or 2, this is to set the
              floor layer where the tiles will be put into the DS1. By
              default tiles will be on floor layer 1.


Exemple 1 :
===========

   @echo off
   TileMaker2 "Crypt.pcx" 50 1 > "Crypt.txt"

will create Crypt.ds1, Crypt.dt1 and Crypt.txt files. The image will be
split into tiles that use the main-indexes 50, 51, 52... The generated
DS1 will be a Act 1 map, and all tiles will be put into the Floor layer 1


Exemple 2 :
===========

   @echo off
   TileMaker2 "Crypt.pcx" 50 5 2 > "Crypt.txt"

will create Crypt.ds1, Crypt.dt1 and Crypt.txt files. The image will be
split into tiles that use the main-indexes 50, 51, 52... The generated
DS1 will be a Act 5 map, and all tiles will be put into the Floor layer 2
