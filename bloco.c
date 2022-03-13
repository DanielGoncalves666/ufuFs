#ifndef BLOCO_H
#define BLOCO_H

/*
Este módulo contém funções responsáveis pela manipulação de blocos.
Todo bloco é indicado por um inteiro que vai de 0 até a quantidade de blocos no sistema de arquivos.
Todas as system calls nesse módulo estarão operando em cima do file descriptor referente ao volume montado no sistema.
As funções com parâmetros tipo void necessitam deles pois os dados não tem tipo definido em disco.
*/

#include"estruturas.h"
#include<sys/types.h>
#include<unistd.h>

/*
ler_bloco
----------
Entrada: inteiro indicando o número do bloco a ser "lido"; ponteiro para tipo void que apontará para o bloco "lido"
Descrição: Se o num_bloco indicar um bloco válido realiza a "leitura" dele.
Saída: 0, em erro, 1, em sucesso 
*/
int ler_bloco(unsigned int num_bloco, void *bloco)
{
	// verifica se o num_bloco é válido (menor que o número máximo de blocos) 
	// usa lseek pra reposicionar o "cursor" para o bloco requerido
	// usa read para ler o bloco (é necessário informar o tamanho de um bloco)

}

/*
escrever_bloco
----------
Entrada: inteiro indicando o número do bloco a ser escrito; ponteiro para tipo void que aponta para o bloco a ser escrito
Descrição: Se o num_bloco indicar um bloco válido realiza a escrita dele.
Saída: 0, em erro, 1, em sucesso 
*/
int escrever_bloco(unsigned int num_bloco, void *bloco)
{
	// verifica se o num_bloco é válido
	// usa lseek para reposicionar o "cursor" para o bloco requerido
	// usa write para escrever o bloco (é necessário informar o tamanho de um bloco)
	// como ocorreu uma escrita de um bloco e queremos que isso seja passado imediatamente para o disco, usamos fsync para forçar isso.
}


/* 
para ler e escrever em um inode específico teremos que ter o valor de INODES_POR_BLOCO definido.
Fazemos INODE_NUM / INODES_POR_BLOCO. O quociente vai ser o bloco a ser obtemos por meio da função ler_bloc.
O resto será o offset dentro do bloco. Com isso acessaremos o inode especificado.
*/

/*
	Funções necessárias relacionadas a blocos, mas que talvez nao precisem estar aqui.
	obter_bloco_livre --> vai depender da estratégia sendo usada.
	obter_inode_livre --> retorna o primeiro inode que não estiver sendo ocupado.
*/

#endif
