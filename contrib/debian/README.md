
Debian
====================
This directory contains files used to package mazad/maza-qt
for Debian-based Linux systems. If you compile mazad/maza-qt yourself, there are some useful files here.

## maza: URI support ##


maza-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install maza-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your maza-qt binary to `/usr/bin`
and the `../../share/pixmaps/bitcoin128.png` to `/usr/share/pixmaps`

maza-qt.protocol (KDE)

