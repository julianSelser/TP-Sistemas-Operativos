mkdir nivel-gui

gcc -I/usr/include -I/usr/include/i386-linux-gnu -I/usr/lib/gcc/i686-linux-gnu/4.7/include -I/usr/lib/gcc/i686-linux-gnu/4.7/include-fixed -I/usr/local/include -I"../../so-commons-library/src" -I"../../so-nivel-gui-library/nivel-gui" -I"../../tad_items" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"nivel-gui/nivel.d" -MT"nivel-gui/nivel.d" -o "nivel-gui/nivel.o" "../nivel-gui/nivel.c"

g++ -shared -o "libso-nivel-gui-library.so"  ./nivel-gui/nivel.o   
