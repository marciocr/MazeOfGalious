MAZE OF GALIOUS REMAKE v0.63
Santi Ontañón Villar

Note: this is a port of the original version to SDL3, for Linux x86_64.
Releases (Flatpak, AppImage and tar.gz) are published at
https://github.com/marciocr/MazeOfGalious/releases

To build it, install CMake, pkg-config, SDL3 and SDL3_mixer (3.x), then:

$ cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
$ cmake --build build
$ ./build/MazeOfGalious

The game must be started from this folder (it reads graphics/, rooms/,
shaders/ and sound/ from the current directory). Settings and saved games
go to ~/.local/share/mazeofgalious/ (older versions used ~/.mog/, which is
still read).

TITLE SCREEN: PLAY START, PASSWORD (to continue a saved game; L is a
shortcut) and OPTIONS (O is a shortcut). In the password screen, letters
and digits are typed, or changed with UP/DOWN; F5 to F9 load the passwords
saved at DEMETER's shrine.

OPTIONS:
- AUDIO: music and effects volume, sound set.
- VIDEO: graphic set, fullscreen, window size and upscaling filter
  (Scale2x, 2xSaI, hq2x, xBR, OmniScale, Lanczos).
- CONTROLS: gamepad 1-4 or keyboard only, and key redefinition.

GAMEPAD: D-pad or left stick - move, A - sword, B - secondary weapon,
X - jump, Y - item screen, START - pause, BACK - options menu.
In the menus: A/START - select, B/BACK - go back.

KEYS (default, can be redefined):

UP ARROW - UP/JUMP
DOWN ARROW - DOWN
LEFT ARROW - LEFT
RIGHT ARROW - RIGHT
SPACE - SWORD
M - FIRE SECONDARY WEAPON
F1 - ITEM SCREEN
F2 - PAUSE

ESC - Options menu, to quit to the title screen (NOT REDEFINABLE)
F10 - Changes the Graphic set (NOT REDEFINABLE)
F11 - Changes the Sound set (NOT REDEFINABLE)
F12 - Quits the game (NOT REDEFINABLE)
PGUP/PGDOWN - Zoom (NOT REDEFINABLE)
L - Pressed in the title screen, allows us to enter passwords. (NOT REDEFINABLE)
ALT + ENTER - Toggles fullscreen/windowed mode


Acknowledgements:

Thanks to (chronological order of collaboration):
- KOIOTE from Spain, for the SFX samples and beta testing.
- BOLTIAN from Spain, for his excellent graphic set.
- JASON EAMES from Spain, for hosting the web, helping me to promote it and for beta testing.
- CHRISTIAN JAVIER D'ORAZIO from Argentina, for his help with the password decoding.
- JOOST YERVANTE fron Netherland, for the help with SDL and the LINUX port.
- HINOX from Spain, for his great grahpic set and his motivation!
- HENK BOSMAN from Netherland, from beta testing.
- LARS THE 18TH from Netherland, for his error reports about the PASSWORD DECODING section,
  and his excellent PASSWORD generator.
- WISAM AL-RAWI from IRAQ, for beta testing and for the original SFX.
- MAX ATTAR FEINGOLD from US, for beta testing and for his help in fixing compatibility
  problems with Windows XP.
- SERGI GARCÍA BESORA from CATALONIA (SPAIN), for beta testing.
- JORRITH SCHAAP from NETHERLAND, for his excellent music files.
- BART ROYMANS from NETHERLAND, for his excellent music files.
- ZITA MIRANDA MARTINEZ from SPAIN, for beta testing.
- MENNO HAECK from NETHERLAND, for beta testing.
- JANNE VIERI from FINLAND, for beta testing.
- ROB POUWELSE from NETHERLAND, for beta testing and a new sound-track.
- J-LUNARTIC from GERMANY, for the translation of this file to german.
- WOLF from NETHERLAND, for his great sound files!
- TEMPLAU from SPAIN, for his neat trance version of the castle them!
- CHRISTOPHE HUET from FRANCE, for his effort on his own and great MoG remake!!!
- RANDY WINTJES from NETHERLAND, for the original MIDI files from where the alternate sound
  set was built.