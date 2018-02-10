
Debian
====================
This directory contains files used to package infinexd/infinex-qt
for Debian-based Linux systems. If you compile infinexd/infinex-qt yourself, there are some useful files here.

## infinex: URI support ##


infinex-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install infinex-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your infinex-qt binary to `/usr/bin`
and the `../../share/pixmaps/infinex128.png` to `/usr/share/pixmaps`

infinex-qt.protocol (KDE)

