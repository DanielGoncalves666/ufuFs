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
#include<unistd.h>
#include"ufuFS.h"

#define INODE_RATIO 0.05

extern int inode_number;
extern int inodes_in_a_block;

inode criar_inode_raiz();

int main()
{
	char pathname[25];
	int div_fd;
	int size, resul, escolha;
	superblock sb;
	void *bc;

	printf("\nEntre com o caminho do device file para o dispositivo a ser formatado(Ex: /dev/sdXX).\n");
	scanf("%s",pathname);

	size = abrir_dispositivo(pathname, &div_fd); // size tem a quantidade de blocos no dispositivo
	if(size == -1)
	{
		printf("\nNão foi possível abrir o device file.\n");
		return 0; // falha
	}
	
	bc = calloc(1,BLOCK_SIZE);
	resul = ler_bloco(div_fd,0,bc); // carrega o bloco onde o superblock deveria estar
	if(resul == 0)
		return 0; // falha

	memcpy(&sb, bc, sizeof(superblock));
	free(bc);
	if(sb.magic_number == UFUFS_MAGIC_NUMBER)
	{
		printf("\n Sistema de Arquivos ufuFS detectado, deseja sobreescrever?\n 1 - Sim\n 2 - Não\n");
		scanf("%d",&escolha);
		
		if(escolha != 1)
		{
			close(div_fd);
			return 0;
		}
	}

	inodes_in_a_block =  BLOCK_SIZE / sizeof(inode);	 
	inode_number = (size * INODE_RATIO) * inodes_in_a_block;
	  
	sb.magic_number = UFUFS_MAGIC_NUMBER;
	sb.block_number = size;
	sb.block_size = BLOCK_SIZE;
	sb.inode_table_length = size * INODE_RATIO; // 5% de todos os blocos
	sb.inode_bitmap_begin = 1;
    sb.data_bitmap_begin = sb.inode_bitmap_begin + (int) ceil(inode_number / (BLOCK_SIZE * 8));  
    
    int aux = size - (sb.data_bitmap_begin + sb.inode_table_length);
    int qtd_data_bitmap = ceil(aux/(BLOCK_SIZE * 8));
    
	sb.file_table_begin = sb.data_bitmap_begin + qtd_data_bitmap; 
	sb.data_table_begin = sb.file_table_begin + sb.inode_table_length;

	bc = calloc(1,BLOCK_SIZE);
	
	printf("\n A formatação levará alguns minutos.\n");
	
	//limpa o superbloco, bitmaps e a file_table
	for(int i = 0; i < sb.data_table_begin; i++)
	{
		escrever_bloco(div_fd,i,bc);
	}
	
	memcpy(bc,&sb,sizeof(superblock));

	if(escrever_bloco(div_fd,0,bc) == 0)
	{
		printf("falha na escrita do superblock\n");
		return 0;
	}
	free(bc);

	
	inode raiz = criar_inode_raiz();// cria inode raiz
	alterar_bitmap(div_fd,0,sb,2);// marca a posição inode 0 como ocupada
	write_inode(div_fd,sb.file_table_begin,0, &raiz); // escreve o inode em disco
		
	dir_entry itself = {0,"."}; // cria a entrada que aponta pro próprio diretório
	bc = calloc(1,BLOCK_SIZE); // cria um bloco vazio
	memcpy(bc,&itself,sizeof(dir_entry)); // copia o dir_entry pro bloco vazio
	escrever_bloco(div_fd, sb.data_table_begin, bc); // escreve o bloco que contém a entrada itself
	
	alterar_bitmap(div_fd,0,sb,1); // marca a posição dado 0 como ocupada

	free(bc);
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
	
	raiz.tipo = DIRETORIO;
	raiz.inode_num = 0;
	raiz.criacao.dia = agora->tm_mday;
	raiz.criacao.mes = agora->tm_mon + 1; 
	raiz.criacao.ano = agora->tm_year + 1900;
	raiz.criacao.hora = agora->tm_hour;
	raiz.criacao.minuto = agora->tm_min; 
	raiz.criacao.segundo = agora->tm_sec;
	
	raiz.acesso = raiz.criacao;
	raiz.tamanho = sizeof(dir_entry);
	
	raiz.bloco_inicial = 0;
	raiz.bloco_final = 0;

	return raiz;
}



#endif
