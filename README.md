# ufufs

### Descrição

Sistema de Arquivos construído para a disciplina de Sistemas Operacionais do Curso em Ciência da Computação pela Universidade Federal de Uberlândia.


#### Construindo a Biblioteca Compartilhada
gcc -c -Wall -Werror -fpic ufuFS.c bloco.c bitmap.c && gcc -shared -o libufuFS.so ufuFS.o bloco.o bitmap.o -lm

#### Compilando e Executando o programa de formatação ufufs_format
gcc -L$PWD -Wl,-rpath=$PWD -Wall -o format ufuFS_format.c -lufuFS -lm


&nbsp;
sudo ./format

#### Compilando e Executando o program de mini shell ufufs_shell
gcc -L$PWD -Wl,-rpath=$PWD -Wall -o shell ufuFS_shell.c -lufuFS -lm


&nbsp;
sudo ./shell

