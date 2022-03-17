#ifndef BLOCO_H
#define BLOCO_H

/*
Este módulo contém funções responsáveis pela manipulação de blocos.
Todo bloco é indicado por um inteiro que vai de 0 até a quantidade de blocos no sistema de arquivos.
Todas as system calls nesse módulo estarão operando em cima do file descriptor referente ao volume montado no sistema.
As funções com parâmetros tipo void necessitam deles pois os dados não tem tipo definido em disco.
*/

#include<stdio.h>
#include"estruturas.h"
#include<sys/types.h>
#include<unistd.h>

int fd; // armazena o file descriptor do device file
int block_number; // quantidade de blocos no dispositivo

/*
abrir_dispositivo
------------------
Entrada: String indicando o caminho até o device file do dispositivo de armazenamento que será acessado.
Descrição: abre o device file responsável pelo dispositivo que queremos acessar/formatar
Saída: -1, em fracasso, o tamanho em blocos do disco em sucesso
*/
int abrir_dispositivo(const char *pathname)
{
	struct stat info;
	
	fd = open(pathname, O_RDWR);
	
	if(fd < 0)
	{
		fprintf(stderr,"\nFalha no acesso ao disco %s.\n",pathname);
		return - 1;
	}
	
	if( fstat(fd,%info) < 0)
	{
		fprintf(stderr,"\nFalha na obtenção de informações sobre o disco.\n");
		return -1;
	}
	
	block_number = info.st_size / BLOCK_SIZE;
	
	return block_number;
}


/*
ler_bloco
----------
Entrada: inteiro indicando o número do bloco a ser "lido"; ponteiro para tipo void que apontará para o bloco "lido"
Descrição: Se o num_bloco indicar um bloco válido realiza a "leitura" dele.
Saída: 0, em erro, 1, em sucesso 
*/
int ler_bloco(unsigned int num_bloco, void *bloco)
{
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
Entrada: inteiro indicando o número do bloco a ser escrito; ponteiro para tipo void que aponta para o bloco a ser escrito
Descrição: Se o num_bloco indicar um bloco válido realiza a escrita dele.
Saída: 0, em erro, 1, em sucesso 
*/
int escrever_bloco(unsigned int num_bloco, void *bloco)
{
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
Entrada: inteiro indicando o número do inode a ser lido; ponteiro para tipo void para onde o inode deve ser copiado
Descrição: realiza leitura de um inode
Saída: 0, em falha, 1, em sucesso
*/
int read_inode(unsigned int num_inode, void *inode)
{
	// VERIFICAR SE ESTÁ DENTRO DOS LIMITES DE INODES

	int bloco = num_inode / INODE_IN_A_BLOCK;
	int offset = num_inode % INODE_IN_A_BLOCK;

	if( lseek(fd, (bloco * BLOCK_SIZE) + offset * sizeof(inode), SEEK_SET) < 0)
		return 0;	
	
	read(fd,inode, sizeof(inode));
	return 1;
}

/*
write_inode
-----------
Entrada: inteiro indicando o número do inode a ser escrito; ponteiro para tipo void para onde o inode a ser escrito está
Descrição: realiza leitura de um inode
Saída: 0, em falha, 1, em sucesso
*/
int write_inode(unsigned int num_inode, void *inode)
{
	// VERIFICAR SE ESTÁ DENTRO DOS LIMITES DE INODES

	int bloco = num_inode / INODE_IN_A_BLOCK;
	int offset = num_inode % INODE_IN_A_BLOCK;

	if( lseek(fd, (bloco * BLOCK_SIZE) + offset * sizeof(inode), SEEK_SET) < 0)
		return 0;	
	
	write(fd,inode, sizeof(inode));
	return 1;
}


/*
	Funções necessárias relacionadas a blocos, mas que talvez nao precisem estar aqui.
	obter_bloco_livre --> vai depender da estratégia sendo usada.
	obter_inode_livre --> retorna o primeiro inode que não estiver sendo ocupado.
*/

#endif
