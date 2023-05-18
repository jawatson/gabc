# gabc

A small abc editor for Gnome.  I doubt if the world really needs another 
abc editor but I was looking for a small project that would help me learn more 
about GTK4 the Gnome Builder application and FlatPak.

Gabc (Gnome abc) allows users to open/edit/save abc files and render them either
as engraved music notation or as playable midi files.

File engraving and playing is performed by the [abcm2ps](https://github.com/lewdlime/abcm2ps) 
and [abc2midi](https://github.com/sshlien/abcmidi) applications
respectively.  In keeping with Flatpak design practices, these have been 
included as modules.  Copyright of these applications is neither claimed nor 
should be inferred.
 
The abc notation syntax highlighting is based upon the .lang file created by
B. Petersen, available at;

[https://github.com/r10s/gtksourceview-abc/blob/master/abc.lang](https://github.com/r10s/gtksourceview-abc/blob/master/abc.lang)




