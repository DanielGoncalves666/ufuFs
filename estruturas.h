#ifndef ESTRUTURAS_H
#define ESTRUTURAS_H

#define MAXIMUM_NAME_LENGTH 10
#define ROOT_DIRECTORY_INODE 0
#define BLOCO_INVALIDO 0
#define ARQUIVO 1
#define DIRETORIO 2
#define UFUFS_MAGIC_NUMBER 123456
#define BLOCK_SIZE 4096

int inode_number; // quantidade de inodes
int inodes_in_a_block;// quantidade de inodes em um bloco

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
	unsigned int *blocos;
} bloco_indireto;

typedef struct inode
{
	short int tipo; // 0 - invalido, 1 - arquivo, 2 - diretorio
	struct dataTime criacao;
	struct dataTime acesso; 
	int tamanho; // tamanho, em bytes, se for um arquivo. tamanho, em quantidade de entradas, se for um diretório.
	
	int blocos[10]; // armazena os numeros para os blocos de dados diretos (-1 é invalido) -- 10 * 4KB = 40KB
	int ind_bloco; // indica o bloco de dados onde está a estrutura bloco_indireto (-1 é invalido) -- 1024 * 4KB = 4MB
	int dup_ind_bloco; // indica um bloco que está preenchido com indicadores para blocos indiretos -- 1024 * 1024 * 4KB = 4GB
} inode;

typedef struct dir_entry
{
	int numero_inode;
	char nome[11];
} dir_entry;

long int abrir_dispositivo(const char *pathname, int *fd);
int ler_bloco(int fd, unsigned int num_bloco, void *bloco);
int escrever_bloco(int fd, unsigned int num_bloco, void *bloco);
int read_inode(int fd, unsigned int file_table_begin, unsigned int num_inode, void *inode);
int write_inode(int fd,  unsigned int file_table_begin, unsigned int num_inode, void *inode);
int obter_inode_livre(int fd, superblock sb);
int localizar_bit(unsigned char valor);
int alterar_bitmap(int fd, int number, superblock sb, int tipo);
unsigned char inverter_bit(unsigned char valor, int pos);

#endif
