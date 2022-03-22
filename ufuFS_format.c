#ifndef UFUFS_FORMAT_H
#define UFUFS_FORMAT_H

/*
Módulo de formatação

*/

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<math.h>
#include<time.h>
#include"estruturas.h"
#include <unistd.h>


// abrir o device file e obter o tamanho do disco.
// calcular a quantidade de blocos
// baseando-se na quantidade de blocos, determinar os ranges para o bitmap de dados, bitmap de inodes. Tudo isso considerando que 10% do disco é dedicado à inodes.
// criar e preenchar uma estrutura de superbloco.

// escrever o superbloco no disco e zerar os blocos referentes ao bitmap de dados, bitmap de inodes e a tabela de inodes.
// criar o inode e fazer a alteração nos bitmaps para o diretório raiz. (todo diretório tem uma entrada . , incluindo o raiz)
	// n daria pra usar a operações de criaçao de arquivo pois é o diretório raiz e o nome dele n existe.
// podemos colocar a opção de um hard format ou format rápido. No caso do hard format talvez seja bom criar uma matriz do tamanho de um bloco com um caractere 
 	// inválido e então preencher todo o disco com esse bloco.

inode criar_inode_raiz();

int main()
{
	char pathname[25];
	int *buffer;
	unsigned char *bc;
	int size, resul;  
	static int div_fd;

	// COLOCAR PRINTFS
	scanf("%s",pathname);

	size = abrir_dispositivo(pathname, &div_fd); // size tem a quantidade de blocos no dispositivo
	if(size == -1)
		return 0; // falha
	
	bc = (unsigned char *) calloc(1,BLOCK_SIZE);
	resul = ler_bloco(div_fd,0,bc); // carrega o bloco onde o superblock deveria estar
	if(resul == 0)
		return 0; // falha

	memcpy(buffer, bc, 4);
	if(*buffer == UFUFS_MAGIC_NUMBER)
	{
		// já existe sistema de arquivos
			// perguntar se quer sobreescrever
				// se sim, continua normalmente
				// se não, finaliza programa
	}

	// pergunta se o usuário quer um hard ou soft format
		// hard format preenche com blocos vazios todas as posições
		// soft deixa os blocos de dados do jeito que estão e atua apenas nas estruturas de matadados

	inodes_in_a_block =  BLOCK_SIZE / sizeof(inode);	 
	inode_number = (size * 0.05) * inodes_in_a_block;

	superblock sb;
	  
	sb.magic_number = UFUFS_MAGIC_NUMBER;
	sb.block_number = size;
	sb.block_size = BLOCK_SIZE;
	sb.inode_table_length = size * 0.05; // 10% de todos os blocos
	sb.inode_bitmap_begin = 1;
    sb.data_bitmap_begin = sb.inode_bitmap_begin + (int) ceil(inode_number / (BLOCK_SIZE * 8));  
    
    int aux = size - (sb.data_bitmap_begin + sb.inode_table_length);
    int qtd_data_bitmap = ceil(aux/(BLOCK_SIZE * 8));
    
	sb.file_table_begin = sb.data_bitmap_begin + qtd_data_bitmap; 
	sb.data_table_begin = sb.file_table_begin + sb.inode_table_length;

	void *empty_block = calloc(1,BLOCK_SIZE);
	
	for(int i = 0; i < sb.data_table_begin; i++)
	{
		// limpa o superbloco, bitmaps e a file_table
		escrever_bloco(div_fd,i,empty_block);
	}
	
	if(/*se o usuario tiver escolhido hard format*/ 0 )
	{
		for(int i = sb.data_table_begin; i < size; i++)
		{
			// limpa o data_table
			if(escrever_bloco(div_fd,i,empty_block) == 0)
			{
				fprintf(stderr,"\nFalha na escrita\n");
				return 0;
			}
		}
	}

	printf("%d\n",sb.magic_number);
	printf("%u\n",sb.block_number);
	printf("%u\n",sb.block_size);
	printf("%u\n",sb.inode_table_length);	
	printf("%u\n",sb.inode_bitmap_begin);
	printf("%u\n",sb.data_bitmap_begin);
	printf("%u\n",sb.file_table_begin);
	printf("%u\n\n",sb.data_table_begin);

	int offset = 0;
	memcpy(empty_block + offset,&sb.magic_number,sizeof(int));
	offset += sizeof(int);
	memcpy(empty_block + offset,&sb.block_number,sizeof(unsigned int));
	offset += sizeof(unsigned int);
	memcpy(empty_block + offset,&sb.block_size,sizeof(unsigned int));
	offset += sizeof(unsigned int);
	memcpy(empty_block + offset,&sb.inode_table_length,sizeof(unsigned int));
	offset += sizeof(unsigned int);
	memcpy(empty_block + offset,&sb.inode_bitmap_begin,sizeof(unsigned int));
	offset += sizeof(unsigned int);
	memcpy(empty_block + offset,&sb.data_bitmap_begin,sizeof(unsigned int));
	offset += sizeof(unsigned int);
	memcpy(empty_block + offset,&sb.file_table_begin,sizeof(unsigned int));
	offset += sizeof(unsigned int);
	memcpy(empty_block + offset,&sb.data_table_begin,sizeof(unsigned int));

	// escrever superblock
	//memcpy(empty_block,&sb,sizeof(superblock));
	if(escrever_bloco(div_fd,0,empty_block) == 0)
	{
		printf("falha na escrita do superblock\n");
	}
	
	// escrever inode raiz

	// verificar sucesso
	inode raiz = criar_inode_raiz();// cria inode raiz
	alterar_bitmap(div_fd,0,sb,2);// marca a posição inode 0 como ocupada
	write_inode(div_fd,sb.file_table_begin,0, &raiz); // escreve o inode em disco
	
	dir_entry itself = {0,"."}; // cria a entrada que aponta pro próprio diretório
	void *data_block = calloc(1,BLOCK_SIZE); // cria um bloco vazio
	memcpy(data_block,&itself,sizeof(dir_entry)); // copia o dir_entry pro bloco vazio
	escrever_bloco(div_fd, sb.data_table_begin, data_block); // escreve o bloco que contém a entrada itself
	
	alterar_bitmap(div_fd,0,sb,1); // marca a posição dado 0 como ocupada

	close(div_fd);
	
	printf("\n--- Formatado ---\n");
	return 0;
}

inode criar_inode_raiz()
{
	inode raiz;
	struct tm *agora;
	time_t raw = time(NULL);
	agora = localtime(&raw);
	
	raiz.tipo = 2;
	raiz.criacao.dia = agora->tm_mday;
	raiz.criacao.mes = agora->tm_mon + 1; 
	raiz.criacao.ano = agora->tm_year + 1900;
	raiz.criacao.hora = agora->tm_hour;
	raiz.criacao.minuto = agora->tm_min; 
	raiz.criacao.segundo = agora->tm_sec;
	
	raiz.acesso = raiz.criacao;
	raiz.tamanho = 1;
	
	raiz.blocos[0] = 0; // primeiro bloco da data table
	for(int i = 1; i < 10; i++)
	{
		raiz.blocos[i] = -1;
	}
	
	raiz.ind_bloco = -1;
	raiz.dup_ind_bloco = -1;

	return raiz;
}



#endif
