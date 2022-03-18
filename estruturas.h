#ifndef ESTRUTURAS_H
#define ESTRUTURAS_H

#define MAXIMUM_NAME_LENGTH 10
#define BLOCK_SIZE 4096
#define ROOT_DIRECTORY_INODE 0

#define ARQUIVO 1
#define DIRETORIO 2
#define BLOCO_INVALIDO 0
#define UFUFS_MAGIC_NUMBER 11111


struct dataTime
{
	short int dia, mes, ano;
	short int hora, minuto, segundo;
};

typedef struct superblock
{
	int magic_number; // identificador do sistema de arquivos
	unsigned int block_number; // quantidade de blocos na partição
	unsigned int block_size; // tamanho dos blocos em bytes (múltiplo de 512)
	unsigned int inode_table_length; // quantidade de blocos que compõem a tabela de inodes
	
	unsigned int inode_bitmap_begin; // numero de inicio do bitmap dos inodes	
	unsigned int data_bitmap_begin; // numero de inicio do bitmap dos bloco de dados
	unsigned int file_table_begin; // numero de inicio da tabela de inodes
	unsigned int data_table_begin; // numero de inicio da tabela de dados
} superblock;

typedef struct bloco_indireto
{
	unsigned int blocos[BLOCK_SIZE / 4];
} bloco_indireto;

typedef struct inode
{
	short int tipo; // 0 - invalido, 1 - arquivo, 2 - diretorio
	unsigned int numero_inode; // numero do inode
	struct dataTime criacao;
	struct dataTime acesso; 
	int tamanho; // tamanho, em bytes, do conteúdo do arquivo
	
	unsigned int blocos[10]; // armazena os numeros para os blocos de dados diretos (0, que sempre é o superbloco, indica q é inválido) -- 10 * 4KB = 40KB
	unsigned int ind_bloco; // indica o bloco de dados onde está a estrutura bloco_indireto (0, que sempre é o superbloco, indica q é inválido) -- 1024 * 4KB = 4MB
	unsigned int dup_ind_bloco; // indica um bloco que está preenchido com indicadores para blocos indiretos -- 1024 * 1024 * 4KB = 4GB

} inode;
const int INODE_IN_A_BLOCK = BLOCK_SIZE / sizeof(inode);

struct bloco
{
	unsigned char mat[BLOCK_SIZE];
};

typedef struct bloco bloco;
typedef struct bloco bitmap;


typedef struct dir_entry
{
	char nome[11];
	int numero_inode;
} dir_entry;

#endif
