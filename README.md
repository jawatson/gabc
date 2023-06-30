# gabc

Gnome ABC, a small abc editor for Gnome.  I doubt if the world needs another 
abc editor but it's really just a means of helping me learn more 
about GTK4, Gnome Builder and FlatPak.

Gabc allows users to open/edit/save abc files and render them either
as engraved music notation or as playable midi files.

## Preferences

A few preferences, related to abcm2ps and abc2midi are supported;

### abcm2ps
- Show errors.  If enabled will indicate problems with the input file with
a red circle in the engraved output.
- FMT File Path. This specifies the path to a .fmt file that should be searched 
for notation symbols.  e.g. Users wishing to employ Grey Larsen's 
formatting for flute or whistle ornaments may use this parameter to specify the 
location of of a local copy of the file 
[larsen.fmt](https://github.com/jawatson/abc-larsen/blob/master/larsen.fmt).
- Page numbering specifies the use and placement of page numbers in the rendered output. 

### abc2midi
- 'Barfly Stress Model. abc2midi supports a couple of 'stress models' for stressing  notes 
when playing the file.  Detail of how the stress model works may be found [here](https://abcmidi.sourceforge.io/#stressmodel).
Note: In order for the stress model to work, the 'R:' (rhythm) parameter must be specified.

## Embedded MIDI commands
Conversion to midi is performed by abc2midi which supports a number of embedded commands.  
Details of these commands may be found in the [Online documentation](https://abcmidi.sourceforge.io/#top).

The program parameter may be used to control the voice of the rendered midi.  Voices of interest to the traditional musician include;

    %%MIDI program 21 # Accordion
    %%MIDI program 40 # Violin
    %%MIDI program 73 # Flute

The above commands should be inserted between the header and music.  For the full list of 128 
available voices, refer to the [online documentation](https://abcmidi.sourceforge.io/#channels)



## Credits

### abcm2ps 
File engraving is performed by [abcm2ps](https://github.com/lewdlime/abcm2ps) 

### abc2midi
Conversion from abc to midi is performed using the 
[abc2midi](https://github.com/sshlien/abcmidi) application.

### Syntax highlighting
The abc notation syntax highlighting is based upon the .lang file created by
B. Petersen, available [here](https://github.com/r10s/gtksourceview-abc/blob/master/abc.lang)



In keeping with Flatpak design practices, these have been 
packaged as modules.  Copyright of these applications is neither claimed nor 
should be inferred.
 



