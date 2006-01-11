For a more recent method, see the TiLP/build/mingw folder.
Original file: Build Environment 050215.txt / ReadMe.txt

==================[cut here]======================================

This is a new method, greatly simplified by the latest GTK+ package.

Win32
-----

1�) Below, you will find a methodology which is common to:
- libtifiles, libticables, libticalcs
- tilp
- tiemu

You should strictly follow it else you will encounter some (very) weird stuffs !

2�) Well, let get's us:

- get MinGW & MSys from <http://www.mingw.org> and install them. You can merge both
folders if you want but this is strongly discouraged by MinGW team. This was my 
way until I discover Tor Lillqvist' method (GiMP for Win32) with a separate 
/target folder.
I assume you installed MinGW as C:\MinGW and MSys as C:\MSys (without 1.0 path).

- get <http://prdownloads.sourceforge.net/gladewin32/gtk-win32-devel-2.4-rc26.exe> 
and install it. Don't forget to register Msys environnement when the installer ask you.

- create a MSys\target folder. Compiled packages will be installed into.

- do an 'export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/target/lib/pkgconfig'
	or make changes in your MSys\etc\profile for example.

3�) To compile TiLP framework, TiLP and/or TiEmu, do the following:
	
- ticables, tifiles, ticalcs:
	./configure --prefix=/target --disable-nls

- tilp:
	./configure --prefix=/target --disable-nls

- tiemu:
	./configure --prefix=/target --disable-nls

Tested with TiLP & TiEmu, 15/02/05.

Linux (cross-compilation)
-------------------------

Obsolete ! To rewrite...

---

Romain Li�vin (The TiLP Team leader).