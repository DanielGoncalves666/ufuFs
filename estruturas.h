#ifndef ESTRUTURAS_H
#define ESTRUTURAS_H

#define MAXIMUM_NAME_LENGTH 10
#define BLOCK_SIZE 4096
#define ROOT_DIRECTORY_INODE 0

#define ARQUIVO 1
#define DIRETORIO 2


struct dataTime
{
	short int dia, mes, ano;
	short int hora, minuto, segundo;
};

typedef struct superblock
{
	int magic_number; // identificador do sistema de arquivos
	int block_number; // quantidade de blocos na partição
	int block_size; // tamanho dos blocos em bytes (múltiplo de 512)
	int inode_table_length; // quantidade de blocos que compõem a tabela de inodes
	int data_bitmap_begin; // numero de inicio do bitmap dos bloco de dados
	int inode_bitmap_begin; // numero de inicio do bitmap dos inodes
	
	int file_table_begin; // numero de inicio da tabela de inodes
	int data_table_begin; // numero de inicio da tabela de dados
} superblock;

typedef struct bloco_indireto
{
	int blocos[1024];
} bloco_indireto;

typedef struct inode
{
	short int tipo; // 0 - invalido, 1 - arquivo, 2 - diretorio
	int numero_inode; // numero do inode (talvez não seja necessário)
	struct dataTime criacao;
	struct dataTime acesso; 
	int tamanho; // tamanho, em bytes, do conteúdo do arquivo
	
	int blocos[10]; // armazena os numeros para os blocos de dados diretos
	int ind_bloco; // indica o bloco de dados onde está a estrutura bloco_indireto

} inode;
const int INODE_IN_A_BLOCK = BLOCK_SIZE / sizeof(inode);

typedef struct map
{
	unsigned char mat[BLOCK_SIZE];
}bitmap;


typedef struct dir_entry
{
	char nome[11];
	int numero_inode;
} dir_entry;

#endif
