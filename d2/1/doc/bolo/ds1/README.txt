游戏小站，暗黑破坏神另类模式修改基地。

http://www.cnmods.com

Win_Ds1Editor，“所见即所得”的强力暗黑地图编辑器！

讨论论坛：

http://forum.cnmods.com

===========================================
   WIN_DS1EDIT.EXE : a WYSIWYG DS1 Editor

      version : 01 August 2004  10:04
===========================================


On-line Documentation :
   http://paul.siramy.free.fr/_divers/ds1/doc/index.html

Advanced Tutorial :
   http://paul.siramy.free.fr/_divers/ds1/doc/tut01/index.html

Download page :
   http://paul.siramy.free.fr/_divers/ds1/dl_ds1edit.html

Main link :
   http://phrozenkeep.it-point.com/forum/viewtopic.php?t=44

If trouble, try to check here :
   http://phrozenkeep.it-point.com/forum/viewtopic.php?t=270

General help can be found at :
   http://phrozenkeep.it-point.com/forum/viewtopic.php?t=724#5563





various parameters can be change in the ds1edit.ini
===================================================
(speed of scrolls, speed of mouse, paths to mpq, default gamma correction ...)
In case you delete this ini, the exe make a new default one

  +--------------------------------------------------------------------------------+
  | The first time you're launching win_ds1edit.exe, if your Diablo II directory   |
  | is not c:\program files\diablo II\ then you HAVE to edit the file ds1edit.ini  |
  |                                                                                |
  | For a first good help on how to use the editor, don't hesitate to check the    |
  | "On-line Documentation" link (see above), screenshots are there to help you,   |
  | as long as some basic "tutorials"                                              |
  +--------------------------------------------------------------------------------+




Main window
===========

ESC = quit

TAB = change current Edit Mode (Tile / Objects / Paths)

F1, F2         = toggle floor layers

F3             = toggle Animations layer
                 (3 modes : none, freeze, animate)
                 Objects can be hidden by walls so you may miss some of them, but in
                 the Object Editing Mode this problem can't occur.

F4             = toggle Object infos layer
                 (4 modes : none, "Type,Id", "Animation Speed", "Description in Obj.txt")

R              = Refresh the obj.txt (it'll read it again), and therefore update the
                 animations. Usefull while editing the obj.txt to see immediatly
                 the modification results in the editor

F5, F6, F7, F8 = toggle wall layers

F9             = toggle Special Tiles drawing priority
                 (2 modes : draw like walls, draw on top of all gfx like roofs)
                 It allow to see Special Tiles hidden by Walls and Roofs.

F10            = toggle npc's paths layer (not yet editable)

F11            = toggle Shadow layer
                 4 states :
                     * inactive
                     * normal (opaque)
                     * white (easy to view)
                     * transparent (DEFAULT)
                 This layer is draw after all the floors layers, and before
                 all the walls layers

SPACE          = toggle walkable info layer, 3 states :
                    * inactive (default)
                    * simple infos (walkable & jump infos only)
                    * complete infos. In this state a little help window
                         appear at the bottom / right corner, it's toggleable by
                         the T key.

SHIFT + F1 to F2, SHIFT + F5 to F8, SHIFT + F11
   view only this layer

CTRL + F1 to F2, CTRL + F5 to F8, CTRL + F11
   view all floor & wall layers except this one

Backspace = reset all layers states to their default value

1, 2, 3, 4, 5, 6, 7, 8, 9, 0 (up of your keyboard - NOT the keypad)
   change the current ds1 you're editing. Check at the end of this file,
   at the "Multiple ds1 loading" section.

CTRL + 1, ..., CTRL + 9, CTRL + 0 (up of your keyboard - NOT the keypad)
   Change the current SET (group) of ds1. When more than 10 ds1 are loaded
   (via a .ini) the editor split them into 10 different sets, from set 1 (CTRL + 1)
   to set 10 (CTRL + 0). In each set you have the usual 10 slots of ds1.

   For instance, to copy tiles from DS1 #22 to DS1 #84 you'll do :

      * CTRL + 3 (enter SET 3, it contains DS1 #21 to #30)
      * 2 (activate DS1 2 of SET 3, so it's DS1 index : 10*(3-1)+2 = 20 + 2 = 22)
      * select your tiles and copy them (CTRL + C)
      * CTRL + 9 (enter SET 9, it contains DS1 #81 to #90)
      * 4 (activate DS1 4 of SET 9, so it's DS1 index : 10*(9-1)+4 = 80 + 4 = 84)
      * place your tiles

   Note : when more than 10 ds1 are loaded, the menu bar contains 2 rows, as an
          additional row appear to help you to navigate through the different ds1.
          This 2nd row show you the current SET, and current DS1 you're on, as well
          as the full path of the ds1.

TILDE = Hide / Show the 2nd row of the menu bar (it's activated by default when a .ini
        was used to load all the ds1).

        F12 = more gamma correction
SHIFT + F12 = less gamma correction

Up, Down, Left, Right         = scroll (speed in the ds1edit.ini)
Mouse on the border of screen = scroll (speed in the ds1edit.ini)

Home = center the map

C, Mouse button 3 (usually the wheel when pressed)
   Center the map to the tile (or sub-tile) the mouse is pointing. It also
   automatically set the zoom to the value defined in ds1edit.ini right after.

+ - (keypad)
- = (keyboard - NOT keypad))
Mouse wheel
   zooming in or out. Available zooms are 1:1, 1:2, 1:4, 1:8 and 1:16

P = Print screen (screenshot)

Left click (maintain pressed) = select area of tiles
this is when the left button is not pressed anymore that theses
keys are checked, for the the action to do on the area : 
   no key = make a new selection
   SHIFT  = add the area to the previous selection
   CTRL   = remove the area from the previous selection
   H      = Hide the current visible layers of the area (usefull to see behind)

   The selection is base on WHAT YOU SEE. By changing the
   LAYERS TOGGLE [F1 to F8, or F11], you can select/deselect a floor,
   a wall, a shadow, multiple floor/walls/shadow, etc... 

   Tips : you can press the left button, then toggle your layers, then
   make your area selection just like you want, then change the layers toggle,
   press the action key to do (shift, ctrl or H) and released the left button
   only at the end. Most of the command still work while selecting an area.

I
SHIFT + I
CTRL  + I
    A powerfull tool : select all Identical tiles in the map, as the ones you're
    clicking the mouse onto.

    Choose a floor, press the 'I' key and maintened it pressed, click on the floor,
    and all the same floors on the whole map are selected (even if they're on another
    layers). This can work even if there's several tiles on the cell, in that case
    you'll have a selection composed of several type of tiles.

    Another usefull thing is that you can still use the Shift and Ctrl key to
    respectively add all these tiles to the current selection, or delete them all from
    the selection : choose a cell which have only 1 floor, select all the same floors
    with the 'I' key. Now, choose another floor, but press Shift + 'I' : you'll have
    in your large selection 2 type of floors now. Press Ctrl + 'I' and click on one
    floor of the selection, and you'll have only 1 type of floor in the selection.

    The tiles type that we'll be search are based on the visibility of the layers, but
    the search in the whole map to select them is done even on hidden tiles.

    Usefull to have an idea about the density of some tiles in the map too.


S = Show (opposite of Hide) all tiles of all layers

CTRL + S = save the ds1, in its current state.
   Incremental backups. First, it rename the current .ds1 to the first free name
   with numbers it found, then it save the current map.

   For instance, say that you are editing the "duriel.ds1" for the first time.
   When you'll ask to save the map, it'll rename "duriel.ds1" to "duriel-000.ds1"
   and save the current state of the map into the "duriel.ds1" file.

   Next time you'll make a save, it'll rename "duriel.ds1" to "duriel-001.ds1"
   and save the current state of the map into the "duriel.ds1" file.

   So, the "duriel-000.ds1" is always you original .ds1, and the next numbers
   are the folowing versions. You're last version is always the "duriel.ds1",
   without the numbers in the name. Numbers are for backups.


CTRL + U = Undo precedents TILES modifications, up to the very start.
   Note that when you make some multiple undo and then start to make new
   modifications, the undo buffers are reseted : the ones you have just used
   to get back to the current state of the map are lost. Also you can't undo
   an undo : there's only 1 way of undo.

N = Night preview. 2 modes : lowest quality, better quality. This is just a toy.
    While in Tile / Object / Path mode, press N and you're in the Night preview
    lowest quality. Press again N and this time you're in the better quality.
    To exit one of the 2 Night preview modes, presse Tab, or press N until you go
    back the original edit mode.


COPY (and therefore PASTE)
--------------------------
CTRL + C = copy the current selection. Right after you have pressed
these keys, a preview appear, folowing your mouse mouvements. The red
part of this preview tell that you will erase previous tiles at that
particular place, green part indicate that the tiles are the same or
there were no tiles before.

If you try to copy / paste some tiles from a ds1 to another, you can expect
big distorsions. If you want to avoid them, only copy / paste between ds1
that have the exact same Type Id (in lvlTypes.txt), and the exact same
Def Id (lvlPrest.txt). This is the way the game use tiles that make
difficult the use of certain tiles in another ds1.


DELETE
------
CTRL + X = delete tiles
    select some tiles, even in different layers, then CTRL + X will delete them.


"STRANGE TILES"
---------------
they just have 4 lines around the cell and have numbers. This is for debugging
purpose : such tiles are either tiles that weren't found in any .dt1, or they are
tiles that have the flag Hidden set, and therefore my editor don't display it
completly (but if you right-click on them you can see the original tile).





Right click = edit the current tile
===================================

click the OK or the CANCEL button to exit this edit screen.

The Wall1......Floor4 change the tabs, and jump you directly to the tile
if there is one in this layer, else to the upper-left corner of the tiles list.

The 'Shadow' button works the same way, except that it appear only if there's
some shadow tiles in the dt1 list of the ds1, else it's not here at all.

click on the tabs to see other type of tiles

F9 toggle the Special Tiles drawing priority

F12 (and Shift + F12) change the gamma correction

F11 (and Shift F11) change the display mode of the shadows

the arrows scroll the tiles, mouse on the screen's border too.

Home return you to the left border

Enter has the same effect as the OK button

Page Up & Page Down scroll the tiles by 1 page, vertically
Ctrl + Page Up & Ctrl + Page Down scroll the tiles by 1 page, horizontaly

The "special" tiles in the 'walls' buttons are important, do not delete them
from the ds1, they are the entry points of the map, corpse location, Tp
location, corner of an area where some layers become invisible when the
player go inside...

The names of the DT1 where the FIRST Tiles of each line are coming from are on the
left side of the 1st Tile column, just scroll to left a little, you'll end to see them.





CTRL + SHIFT + Right-Click = Expert Tile Editing
================================================
When you're on a tile, do this combination, and a special window will appear.
In this one you can edit the datas that are in the ds1, even if they're not
editable in the editor in another way. You'll have to edit them at a bit level.
It's as if you were hexediting a ds1.

Each line is 1 layer in the map:

F1, F2         = Floor 1, Floor 2 layers
SH             = Shadow layer
W1, W2, W3, W4 = Wall 1, Wall 2, Wall 3, Wall 4 layers
SUB            = Substitution layer (only present in very few ds1)

Some bits / group of bits are identified, so you have some text to help you, but
some other bits are still unknown.

The 2 important columns are "Hidden" and "Unwalkable", but you can edit the other
datas too.

Vis0 to Vis7 (warps) that don't have gfx must be hidden, or else the game will display
the infamous Green Tile when you'll play the map. Set the bit to 1 for the layer that
has the Vis, and the Green Tile will disapear.

The "Unwalkable" bit make the entire Tile unwalkable, without editing any DT1. You can
use the Unwalkable infos to help you (Space Key before entering this window).

==> there are 2 possible window, depending if there's a tile selection or not. The White
numbers indicates the bit value is the same for all tiles of the selection. The Blue
numbers indicates the bit value is different, depending of the tiles inside the selection.
A Green minus ('-') indicate that this bit is not editbale, because no tiles of the
selection is in this layer.

==> there's no undo popssible, so take care.





In the 'Object' editing mode
============================

Objects are in the 'Obj.txt' file, open it with Excel or D2Excel if you're
planing to edit it, there's Tabulations that musn't go away.

Objects are displayed as 2 parts : a Label with infos, and the Feet
(the object itself). The label don't exists in the game, it's just internal to
this editor, so when you'll reload a previously edited ds1, the labels will be
in their standard place. By moving label you can see behind, and select the
objects that may be behind. Selecting an Object require you to target the feet,
and if you move it, the label folow (the reverse is not true).

The infos of the label are :
   * 1st line   : description
   * 2nd line...
      - #nn     : object number
      - Type-Id : Type & Id of the object
      - Pnn     : number of paths for this object

Right click over 1 object / label : edit its type and Id (change object)
   while in this mode : moving the mouse at the top or the bottom of the
   'Object List' Window scroll it (speed of this scroll is in ds1edit.ini,
   at the 'obj_edit_scroll = ' line.

   Type 1 objects are NPC & Monsters, Type 2 are usually objects, but
   sometimes NPC (Hratli in act 3 for instance).

   Type 1 objects can be in all acts, but not the Type 2, hence the
   desactivation of the other acts in the list when it's a Type 2 object.

   You can press the Enter key instead of clicking the OK button (either the
   one on the Keypad or the regular one)

   You can also press the Up & Down keys, to scroll the object's list.
   Note that the line "winobj_can_scroll_keyb" in the ds1edit.ini must be set
   to "NO" to enable this scrolling.

   While this window is display, the options defined in the ds1edit.ini takes
   effects : you will be able to scroll the *main Tile* window or not. Keyboard
   and mouse scrolling are 2 distinct options in the .ini


Insert             : insert (add) a new object

Left Click         : select an object / label, or deselect all if not over an
                     object / label

Shift + Left click : add an object / label to the selection, but you can't
                     select objects AND labels, that's one or the other.

Ctrl + left click  : remove from the selection


Posible actions to the selection are ...

Alt (or AltGr) + Left Click on the selection : start moving the selection,
         you can release the Alt just after. Left click to drop it.

Ctrl + X : delete the selection (only if objects)

Ctrl + C : copy the selection (only if objects)

Note : right after an unwanted modification, you can Undo it : Ctrl + U. It
   works after a move / delete / copy / insert, but not a selection modification.
   there's just 1 step of undo for Objects, multiple undo don't works for them (yet).

F3  : still toggle the animation layer state

I   : like in Tile Editing Mode, you can select all identical Label / Objects
      in the map with just 1 click.





Quick loading
=============
The 1st time the Editor is launched, it extracts the palettes from the Mpq,
and make the appropriate color maps at run-time, then it save them all. Next
time you want to edit a map, since the palettes and the colormaps are on your
disk already, the editor just load them, which is a lot quicker. Theses files
are in the directory 'data' of the Editor, and they all have the .bin extension.
If the Editor don't find them at startup, it re-creates them at run-time.





Multiple ds1 loading
====================
you have to make a special text file. Look at the 'Demo.ini', it's an example.
This one open Tristram, then a little special map (for trees), then Duriel. The
first 2 numbers are Type Id & Def Id, in that order, then there's the file. Now
launch '_Multiple ds1 demo.bat' : it give this ini file to the editor, and redirect
the standard output to a file.

When working with multiple ds1 at the same time, as said before, copy / paste is
to use carefully. In our exemple, Tristram and the Trees have the exact same Type &
Def Id, so they have the exact same list of dt1, so you can safely copy / paste
between this 2 files. Now, Duriel have a complete different sets of dt1, so if
you try to copy a House from Tristram and paste it into Duriel, you'll get nothing
interesting : the editor (like the game will) remap the tiles of tristram with the
dt1 list of Duriel, hence theses weird tiles. Same problem when copying tiles from
Duriel to Tristram of course.

You can open up to 100 (one thousand !) ds1 at a time, so 100 lines max in the special
test file (you can use the name you want btw, like a full path).





Working with Isilweo's ds1 editor
=================================
Isilweo's ds1 editor don't support all .ds1 in the game as there are many version
of them, and the editor was done when we didn't know how it was working. If you try
to open an old ds1, it won't work in Isilweo's editor.

An easy way to solve this is to use my editor, as it can read all kind of versions
of ds1, and save the maps right after, as it always save them in the last version (18),
which is the one the Isilweo's ds1 editor understand too.

Also, my editor always make the map with 2 Floor layers and 4 Wall layers, even if
they just have a few originaly. So it's usefull to copy/paste tiles from different .ds1
that didn't have the same number of layers at first. This feature can be disable by a
line in the ds1edit.ini.





Resizing a map, without Isilweo's ds1 editor
============================================
It's possible, it's an option in the command line. For instance the line

   win_ds1edit tri_town4.ds1 11 300 -resize 250 100 > debug_tri_town4.txt

will resize the ds1 of Tristram to 250 width * 100 height tiles. Increasing a map is
simple, the editor just adding some empty tiles, but decreasing a map delete the
objects that are now out of the bound of the map, take care of this ! The paths of a
NPC can be out of bound too. If this happen, since you (still) can't edit them, you
have to delete the original NPC, that way his paths will be deleted as well. Now you
can insert a new object to replace the one you deleted.

Note : in Levels.txt / LvlPrest.txt, the size that you must enter are minus by 1. So
if you were trying to use that new Tristram, you'd enter the size 249 and 99 in the .txt





Command Line Syntax, and Options
================================

win_ds1edit <file.ini>

   Use a .ini to open several .ds1 at once, check the "Multiple ds1 loading" section.
   You can NOT use any other options with that syntaxe.



win_ds1edit <file.ds1> <LvlType.txt ID> <LvlPrest.txt DEF> [options]

   options are :
      -resize <width> <height>
      -force_pal <act>
      -no_check_act

   You can combine all options, and their order is not important.



win_ds1edit <file.ds1> [options]

   options are :
      -resize <width> <height>
      -force_pal <act>
      -no_check_act
      -force_dt1 <file1.dt1> <file2.dt1> ... <file32.dt1>

   If you're using the option "-force_dt1", then the editor don't expect to have
   LvlTypes.txt ID nor lvlPrest.txt DEF on the command line, right after the ds1 name.

   "-force_dt1" must be folowed by at least 1 dt1 file name, on your disk (not in a mpq),
   you can specify up to 32 files this way.

   You can combine all options, and their order is not important.



Exemples :
----------

win_ds1edit gravey.ds1 2 108 > log.txt

   will simply open the Graveyard



win_ds1edit gravey.ds1 2 108 -resize 50 100 > log.txt

   will open the Graveyard, and resize it to be 50 * 100 tiles, instead of
   its original 25 * 33 dimensions.

   (again : use 49 * 99 in levels.txt and lvlprest.txt in this case)



win_ds1edit gravey.ds1 2 108 -force_pal 4 > log.txt

   will open the Graveyard, and use the palette of act 4, which can be usefull to
   see how the graveyard ds1 will look in act 4, without playing the game.
   (note : the result is ugly)



win_ds1edit lavaE.ds1 35 1055 -no_check_act -force_pal 4 > log.txt

   this ds1 is from d2exp.mpq : data\global\tiles\ACT4\Expansion\lavaE.ds1

   It will open a ds1 of the Act *5* that use the Act *4* lava tiles. By default the
   editor don't open it since it detect a difference between what lvlTypes.txt is
   expecting, and the actual ds1 act.

   To solve this problem, you use the -no_check_act option, making the editor skip that
   safety check. In order to solve a palette problem, you specify the correct one by
   using -force_pal, which is the one from Act 4 here.



win_ds1edit tri_town4.ds1 -force_dt1 town.dt1 trees.dt1 > log.txt

   open Tristram, but using only the files :

      * Data/Global/Tiles/Act1/Tristram/town.dt1
      * Data/Global/Tiles/Act1/Town/trees.dt1

   you need to extract the dt1 files mentionned, and put them into the editor directory,
   before beeing able to use them with the "-force_dt1" option.   

   You'll see lot of unknown tiles, but even if the trees are not from the dt1 originally
   used by Tristram, they'll still be displayd, showing that Act1/Town/trees.dt1 and
   Act1/Outdoors/TreeGroups.dt1 are more or less compatible.


   This option is also usefull for trying to apply different DT1 to a ds1 which you don't
   find its LvlTypes.txt ID nor LvlPrest.txt DEF.



win_ds1edit lavaE.ds1 -no_check_act -force_pal 4 -resize 10 20 -force_dt1 floor.dt1 > log.txt


   if
      * floor.dt1 is Data\Global\Tiles\Act4\Lava\Floor.dt1
      * lavaE.dt1 is Data\Global\tiles\ACT4\Expansion\lavaE.ds1

   then you'll open the act 5 lava ds1 using palette of act 4, resizing it
   to 10 * 20 tiles, and using only the floor.dt1, not all originals dt1.



Paths editing
=============
The paths editing is still very basic, but with patience it's posible to do what you want.

    you select 1 and only 1 object in the Object Editing mode. It can be either a
    Type 1 or a Type 2 object, but only Type 1 (Monster / NPC) objects can use paths in the game.
    You then click on the ALL NEW button to delete the current paths and create new ones, by
    clicking on the map. To change the (hardcoded) "Action" (animation and / or sound) that the
    NPC will do at a particular place, click on the (Enter) SET ACTION button and type a number
    in the keypad, then press the Enter key and the next paths that you'll place will use that
    new Action value. When you have finished, click on the OK button, and the NPC have now whole
    new Paths. CANCEL will restore the original NPC paths (if any).
