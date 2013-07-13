
gcc -I"../../so-commons-library" -I/usr/include -I"../../so-serial-library" -I/usr/include/i386-linux-gnu -I/usr/lib/gcc/i686-linux-gnu/4.7/include -I/usr/lib/gcc/i686-linux-gnu/4.7/include-fixed -I/usr/local/include -I"../../so-commons-library/src" -I"../../so-nivel-gui-library/nivel-gui" -I"../../tad_items" -I"../../memoria" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"personaje.d" -MT"personaje.d" -o "personaje.o" "../personaje.c"

gcc -L"../../so-serial-library/Debug" -L"../../so-commons-library/Debug" -o "Personaje"  ./personaje.o   -lso-serial-library -lso-commons-library