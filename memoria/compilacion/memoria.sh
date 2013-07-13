gcc -I/usr/include -I/usr/include/i386-linux-gnu -I/usr/lib/gcc/i686-linux-gnu/4.7/include -I/usr/lib/gcc/i686-linux-gnu/4.7/include-fixed -I/usr/local/include -I"../../so-commons-library/src" -I"../../so-nivel-gui-library" -I"../../memoria" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"memoria.d" -MT"memoria.d" -o "memoria.o" "../memoria.c"

gcc -shared -o "libmemoria.so"  ./memoria.o   