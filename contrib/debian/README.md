
Debian
====================
This directory contains files used to package mazacoind/mazacoin-qt
for Debian-based Linux systems. If you compile mazacoind/mazacoin-qt yourself, there are some useful files here.

## mazacoin: URI support ##


mazacoin-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install mazacoin-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your mazacoin-qt binary to `/usr/bin`
and the `../../share/pixmaps/mazacoin128.png` to `/usr/share/pixmaps`

mazacoin-qt.protocol (KDE)

