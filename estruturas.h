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

// estruturas em disco //

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

typedef struct inode
{
	short int tipo; // 0 - invalido, 1 - arquivo, 2 - diretorio
	int inode_num;
	struct dataTime criacao;
	struct dataTime acesso; 
	unsigned int tamanho; // tamanho, em bytes, se for um arquivo. tamanho, em quantidade de entradas, se for um diretório.
	
	int bloco_inicial; // bloco de dados inicial
	int bloco_final; // bloco de dados final
} inode;

typedef struct dir_entry
{
	int numero_inode; // -1 faz com que seja inválido
	char nome[MAXIMUM_NAME_LENGTH + 1];
} dir_entry;

// estruturas em memória apenas

typedef struct file_descriptor
{
	inode inode_data;
	char nome[MAXIMUM_NAME_LENGTH + 1];
	unsigned int offset;// em bytes
	unsigned int tamanho;
	short int escrita;
}file_descriptor;

long int abrir_dispositivo(const char *pathname, int *fd);
int ler_bloco(int fd, unsigned int num_bloco, void *bloco);
int escrever_bloco(int fd, unsigned int num_bloco, void *bloco);
int read_inode(int fd, unsigned int file_table_begin, unsigned int num_inode, void *inode);
int write_inode(int fd,  unsigned int file_table_begin, unsigned int num_inode, void *inode);
int obter_inode_livre(int fd, superblock sb);
int localizar_bit(unsigned char valor, char bit, int sentido);
int alterar_bitmap(int fd, int number, superblock sb, int tipo);
unsigned char inverter_bit(unsigned char valor, int pos);
int alterar_faixa_bitmap(int fd, int inicio, int fim, superblock sb);
int get_bitmap_pos_status(int fd, int number, superblock sb, int tipo);
int get_block_sequence(int fd, int comeco, superblock sb, int qtd);
#endif
