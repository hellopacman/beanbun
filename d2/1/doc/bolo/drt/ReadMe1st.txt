Diablo Resource Tester (Dr. Tester) - v0.20f (Nov 24,2003)


This utility will let you extract any file from the Diablo I & II MPQ files.
It also has built-in viewers for many types, cof,dcc,dc6, ds1,dt1, pcx.

It was started as a test bed for my dcc decoder class and grew into a 
control center for mod building.

---------------------
Getting started...
---------------------

To start, extract the files to C:\DrTest\ or some other place you like.
Make a folder C:\MyMod\ or such, for a place to build you mod.

Make a short cut on the desktop with:

Target = "C:\DrTest\DrTest.exe"
Start in = "C:\MyMod\"

Get the lastest "listfile" from Phrozen Keep, and copy it to C:\MyMod\.
Rename it to (listfile).0 . I'll explain that later :)

Ok ready to go.

---------------------
File Menu ...
---------------------

The idea dehind Dr. Tester is getting the files you want to mod from the mpq's
and putting them in the Mod folder to modify. The file menu let's you switch
back and forth between the mpq's to see where the files you want are located.

The 'local path' selection views your local mod folder as an mpq file, so you
can see the files you're working with.

'Other mpq' allows you to open an mpq in another place besides the Diablo II 
install location. It's great if you want to add Diablo I or War/Starcraft
files into your mod.

'Save to local' - extracts the file from the mpq into your mod folder using
the same path as in the mpq.

'Save As' - Lets you pick a place to extract the file and the format to save
it. It always starts the Save As dialog in the mod folder path. Choosing a
format that doesn't support frames/animation will create a folder with the
frames numbered like 'basename###'.

---------------------
Option Menu ...
---------------------

'Use Local Files' - This tells Dr Tester to show any local files of the same
name when browsing an mpq, instead of the one that's really in the mpq. This
is nice when you want to see how you mod effects other files. (if you change
a cof file you can see if the layers in it are correct).

'Rebuild Local List' - When browsing the 'local path' Dr tester uses (listfile)
to choose the names. This option update that file with all the names in your 
local path. By editing the (listfile) you can limit which files you want to
view in the local path.

'Auto Rebuild' - This forces the local file list to be updated whenever you save
to local.

'Update Frames' - this toggles the frame control box off and on during animations.
On slower machines, updating the frame number every time causes flicker.

'Background Color' - choose background color of image display. The background
color is used when exporting to other formats also. See 'Save as'


'Enable Halftone' - Enables 'smoothing' when zooming an image. Doesn't work on
all OS/video cards.


'Existing Files Only' - show only the filenames that are really in the MPQ.

'Filter extentions' - allows you to choose the type of file to view.

'Use external list for' - Lets you use a list file when viewing each mpq. Put
the file names you want in the (listfile).x you want. That why we copied a list
file to (listfile).0 because patch_d2.mpq doesn't have an internal (listfile).
This lets Dr Tester know what files there.

--------------------
Run Menu ...
--------------------

'Diablo II' - Runs Diablo II in you mod folder with the -w -direct -txt switches.
Use this to test your mod.

'Explorer' - explores the path of the selected file.

'External program' - Run an external program on the selected file. You can tell
Dr Tester what programs you want to use in the 'run.ini' file.

'Edit Run.ini' - runs notepad to edit the run.ini file

--------------------
Run.ini
--------------------

This ini lets you tell Dr Tester what programs to run for what types of files.
You put a line for the file extension (ie, 'txt') if not already there, and a
line for the command/program you want to run.

For example ...

[txt]

Note Pad = notepad.exe


That's it. '[txt]' is the extension, 'Note Pad' is the discription that shows up.
And notepad.exe is the program to run. You may have as many programs under the
extention as you need. Don't add the [ext] if it's already there, just add your 
command under it, like this ...


[txt]

Note Pad = notepad.exe
AFJ Sheet Editor = "C:\AFJ\AFJ Sheet Edit 0.04a\AFJ Sheet.exe"

[dc6]
Convert to PCX = dc6con.exe


There is a 'special' extention to allow a program for all types of files ...

[*]
Hex Edit = "C:\XVI32\XVI32.exe"


-----------------------------

I still have a lot of features to add/finish.

If you have comments or suggestions email me at...

webmail@stx.rr.com

with 'Dr Tester' in the subject.
