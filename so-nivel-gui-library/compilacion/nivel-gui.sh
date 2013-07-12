mkdir nivel-gui

gcc -I/usr/include -I/usr/include/i386-linux-gnu -I/usr/lib/gcc/i686-linux-gnu/4.7/include -I/usr/lib/gcc/i686-linux-gnu/4.7/include-fixed -I/usr/local/include -I"/home/utnso/git/tp-20131c-juanito-y-los-clonosaurios/so-commons-library/src" -I"/home/utnso/git/tp-20131c-juanito-y-los-clonosaurios/so-nivel-gui-library/nivel-gui" -I"/home/utnso/git/tp-20131c-juanito-y-los-clonosaurios/tad_items" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"nivel-gui/nivel.d" -MT"nivel-gui/nivel.d" -o "nivel-gui/nivel.o" "../nivel-gui/nivel.c"

g++ -shared -o "libso-nivel-gui-library.so"  ./nivel-gui/nivel.o   
