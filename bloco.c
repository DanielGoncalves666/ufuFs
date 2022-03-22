#ifndef BLOCO_H
#define BLOCO_H

/*
Este módulo contém funções responsáveis pela manipulação de blocos.
Todo bloco é indicado por um inteiro que vai de 0 até a quantidade de blocos no sistema de arquivos.
Todas as system calls nesse módulo estarão operando em cima do file descriptor referente ao volume montado no sistema.
As funções com parâmetros tipo void necessitam deles pois os dados não tem tipo definido em disco.
*/

#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include"estruturas.h"
#include<sys/types.h>
#include<unistd.h>
#include<fcntl.h>

int block_number; // quantidade de blocos no dispositivo

/*
abrir_dispositivo
------------------
Entrada: String indicando o caminho até o device file do dispositivo de armazenamento que será acessado.
		 Ponteiro para inteiro onde será armazenado o file descriptor do device file
Descrição: abre o device file responsável pelo dispositivo que queremos acessar/formatar
Saída: -1, em fracasso, o tamanho do disco em blocos
*/
long int abrir_dispositivo(const char *pathname, int *fd)
{	
	*fd = open(pathname, O_RDWR);
	
	if(*fd < 0)
	{
		fprintf(stderr,"\nFalha no acesso ao disco %s.\n",pathname);
		return -1;
	}
	
	// uso do fstat não funciona com device files
	
	block_number = lseek(*fd,0,SEEK_END) / BLOCK_SIZE;
	return block_number;
}


/*
ler_bloco
----------
Entrada: inteiro contendo o file descriptor do device file
         inteiro indicando o número do bloco a ser "lido"
	     ponteiro para tipo void que apontará para o bloco "lido"
Descrição: Se o num_bloco indicar um bloco válido realiza a "leitura" dele.
Saída: 0, em erro, 1, em sucesso 
*/
int ler_bloco(int fd, unsigned int num_bloco, void *bloco)
{
	if(fd < 0)
		return 0; // file descriptor inválido
	
	if(num_bloco >= block_number)
		return 0; // verificação de validade do bloco
	
	// system call lseek reposiciona o "cursor" do arquivo.
	// SEEK_SET estabelece que é na posição (num_bloco * BLOCK_SIZE)	
	if( lseek(fd, num_bloco * BLOCK_SIZE, SEEK_SET) < 0)
		return 0;
		
	read(fd, bloco, BLOCK_SIZE);
	return 1;
}

/*
escrever_bloco
----------
Entrada: inteiro contendo o file descriptor do device file
		 inteiro indicando o número do bloco a ser escrito 
         ponteiro para tipo void que aponta para o bloco a ser escrito
Descrição: Se o num_bloco indicar um bloco válido realiza a escrita dele.
Saída: 0, em erro, 1, em sucesso 
*/
int escrever_bloco(int fd, unsigned int num_bloco, void *bloco)
{
	if(fd < 0)
		return 0; // file descriptor inválido

	if(num_bloco >= block_number)
		return 0; // verificação de validade do bloco
	
	if( lseek(fd, num_bloco * BLOCK_SIZE, SEEK_SET) < 0)
		return 0;
		
	write(fd, bloco, BLOCK_SIZE);
	fsync(fd); // sincroniza com o disco
	
	return 1;
}

/*
read_inode
-----------
Entrada: inteiro contendo o file descriptor do device file
		 inteiro indicando o bloco que os inodes começam a ser armazenados
         inteiro indicando o número do inode a ser lido;
         ponteiro para tipo void para onde o inode deve ser copiado
Descrição: realiza leitura de um inode
Saída: 0, em falha, 1, em sucesso
*/
int read_inode(int fd, unsigned int file_table_begin, unsigned int num_inode, void *inode)
{
	if(fd < 0)
		return 0; // file descriptor inválido

	if(num_inode >= inode_number)
		return 0;

	int bloco = file_table_begin + num_inode / inodes_in_a_block; // calcula o bloco onde o inode se localiza
	int offset = num_inode % inodes_in_a_block; // calcula o offset do inode dentro do bloco

	if( lseek(fd, (bloco * BLOCK_SIZE) + offset * sizeof(inode), SEEK_SET) < 0)
		return 0;	
	
	read(fd,inode, sizeof(inode));
	return 1;
}

/*
write_inode
-----------
Entrada: inteiro contendo o file descriptor do device file
		 inteiro indicando o bloco que os inodes começam a ser armazenados
		inteiro indicando o número do inode a ser escrito
		ponteiro para tipo void para onde o inode a ser escrito está
Descrição: realiza leitura de um inode
Saída: 0, em falha, 1, em sucesso
*/
int write_inode(int fd,  unsigned int file_table_begin, unsigned int num_inode, void *inode)
{
	if(fd < 0)
		return 0; // file descriptor inválido

	if(num_inode >= inode_number)
		return 0;

	int bloco = file_table_begin + num_inode / inodes_in_a_block;
	int offset = num_inode % inodes_in_a_block;

	if( lseek(fd, (bloco * BLOCK_SIZE) + offset * sizeof(inode), SEEK_SET) < 0)
		return 0;	
	
	write(fd,inode, sizeof(inode));
	return 1;
}


/*
	Funções necessárias relacionadas a blocos, mas que talvez nao precisem estar aqui.
	obter_bloco_livre --> vai depender da estratégia sendo usada.
*/

/*
obter_inode_livre
------------------
Entrada: inteiro contendo o file descriptor do device file
		 estrutura de superbloco para se obter informações de início do bitmap
Descrição: procura pelo primeiro inode não ocupado
Saída: -1, em falha, inteiro não negativo indicando o inode livre encontrado
*/
int obter_inode_livre(int fd, superblock sb)
{
	unsigned char *bitmap = (char *) malloc(sizeof(unsigned char) * BLOCK_SIZE);
	int num_inode = 0;

	if(fd < 0)
		return 0; // file descriptor inválido

	for(int i = sb.inode_bitmap_begin; i < sb.data_bitmap_begin; i++)
	{
		ler_bloco(fd, i, bitmap); // lê bloco a bloco do inode_bitmap
		
		for(int h = 0; h < BLOCK_SIZE; h++)
		{
			if(bitmap[h] != 255) // procura pelo primeiro byte que não tem apenas 1's
			{
				num_inode += h * 8;
				return num_inode + localizar_bit(bitmap[h]);
			}
		}
		num_inode += BLOCK_SIZE * 8;
	}
	
	return -1;
}

/*
localizar_bit
--------------
Entrada: unsigned char, contendo o byte
Descrição: procura pelo primeiro zero da esquerda pra direta
Saída: inteiro indicando o primeiro zero, da esquerda pra direita; -1 se não existir nenhum
*/
int localizar_bit(unsigned char valor)
{
	char vetor[8] = {'0','0','0','0','0','0','0','0'};
	int i = 7, resto, aux;
	
	
	aux = valor;
	while(aux != 0)
	{
		resto = aux % 2;
		aux /= 2;
		
		vetor[i] = resto == 1 ? '1' : '0';
		i--;
	}	
	// preenche o vetor da direita pra esquerda de modo a deixar nele o número na ordem correta
	
	for(i = 0; i < 7; i++)
	{
		if(vetor[i] == '0')
			return i;
	}

	return -1;
}

/*
alterar_bitmap
---------------
Entrada: inteiro contendo o file descriptor do device file
		 inteiro, indicando a posição (em bits) a ser alterada
		 estrutura de superbloco para se obter informações de início do bitmap; 
		 inteiro, 1 para DATA, 2 para INODE
Descrição: realiza a alteração da posição indicada
Saída: 0, em fracasso, 1, em sucesso
*/
int alterar_bitmap(int fd, int number, superblock sb, int tipo)
{
	unsigned char *bitmap = (char *) malloc(sizeof(unsigned char) * BLOCK_SIZE);

	if(fd < 0)
		return 0; // file descriptor inválido

	// verificar se sb é válido
	int b = tipo == 1? sb.data_bitmap_begin : sb.inode_bitmap_begin;

	b = b + number / (BLOCK_SIZE * 8); // número do bloco onde a posição fica
	int offset = (number % (BLOCK_SIZE * 8)) / 8; // byte onde está o bit referente ao bloco ou inode
	int final_offset = (number % (BLOCK_SIZE * 8)) % 8; // offset do bit dentro do byte
	
	// VERIFICA SE OS VALORES OBTIDOS FAZEM SENTIDO
	
	if( ler_bloco(fd,b,bitmap) == 0)
		return 0;
	
	bitmap[offset] = inverter_bit(bitmap[offset],final_offset);
	
	if(escrever_bloco(fd,b,bitmap) == 0)
		return 0;
	
	return 1; 
}

unsigned char inverter_bit(unsigned char valor, int pos)
{
	int i = 7,h;
	int resto, aux;
	char vetor[8] = {'0','0','0','0','0','0','0','0'};
	
	aux = valor;
	while(aux != 0)
	{
		resto = aux % 2;
		aux /= 2;
		
		vetor[i] = resto == 1 ? '1' : '0';
		i--;
	}
	
	vetor[pos] = vetor[pos] == '1'? '0' : '1'; // inverte o bit da posição requerida
	
	aux = 0;
	for(i = 7, h = 0; i >= 0; i--, h++)
	{
		aux += (vetor[i] == '1'? 1 : 0) * pow(2,h);
	}
	
	return aux;	
}


#endif
