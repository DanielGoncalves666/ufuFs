# ufufs

## Building shared library
gcc -c -Wall -Werror -fpic ufuFS.c bloco.c bitmap.c && gcc -shared -o libufuFS.so ufuFS.o bloco.o bitmap.o -lm

## Building and executing ufufs_format
gcc -L$PWD -Wl,-rpath=$PWD -Wall -o format ufuFS_format.c -lufuFS -lm
sudo ./format

## Building and executing ufufs_shell
gcc -L$PWD -Wl,-rpath=$PWD -Wall -o shell ufuFS_shell.c -lufuFS -lm
sudo ./shell

