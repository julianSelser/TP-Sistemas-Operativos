
gcc -I"/home/utnso/git/tp-20131c-juanito-y-los-clonosaurios/so-commons-library" -I/usr/include -I"/home/utnso/git/tp-20131c-juanito-y-los-clonosaurios/so-serial-library" -I/usr/include/i386-linux-gnu -I/usr/lib/gcc/i686-linux-gnu/4.7/include -I/usr/lib/gcc/i686-linux-gnu/4.7/include-fixed -I/usr/local/include -I"/home/utnso/git/tp-20131c-juanito-y-los-clonosaurios/so-commons-library/src" -I"/home/utnso/git/tp-20131c-juanito-y-los-clonosaurios/so-nivel-gui-library/nivel-gui" -I"/home/utnso/git/tp-20131c-juanito-y-los-clonosaurios/tad_items" -I"/home/utnso/git/tp-20131c-juanito-y-los-clonosaurios/memoria" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"personaje.d" -MT"personaje.d" -o "personaje.o" "../personaje.c"

gcc -L"/home/utnso/git/tp-20131c-juanito-y-los-clonosaurios/so-serial-library/Debug" -L"/home/utnso/git/tp-20131c-juanito-y-los-clonosaurios/so-commons-library/Debug" -o "Personaje"  ./personaje.o   -lso-serial-library -lso-commons-library