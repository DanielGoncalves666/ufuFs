#ifndef ESTRUTURAS_H
#define ESTRUTURAS_H

#define MAXIMUM_NAME_LENGTH 10
#define BLOCK_SIZE 4096
#define MAPA_BLOCOS_INICIO 2			// necessária alteração
#define MAPA_INODES_INICIO 4			// necessária alteração, provavelmente será dinâmico.
#define TABELA_INODES_INICIO 100		// necessária alteração, provavelmente será dinâmico.
#define TABELA_BLOCOS_INICIO 150		// necessária alteração, provavelmente será dinâmico.

#define ARQUIVO 1
#define DIRETORIO 2



struct dataTime
{
	short int dia, mes, ano;
	short int hora, minuto, segundo;
};

typedef struct superblock
{
	int numero_magico; // identificador do sistema de arquivos
	int numero_blocos; // quantidade de blocos na partição
	int tamanho_bloco; // tamanho dos blocos em bytes (múltiplo de 512)
	// numero de inicio do bitmap dos bloco de dados
	// numero de inicio do bitmap dos inodes
	// numero de inicio da tabela de inodes
	// numero de inicio da tabela de dados
	
	
	int numero_blocos_inode; // quantidade de blocos dedicados à inodes
	int diretorio_raiz; // numero do inode do diretorio raiz
	
} superblock;

typedef struct bloco_indireto
{
	int blocos[1024];
} bloco_indireto;

typedef struct inode
{
	short int tipo; // 1 - arquivo, 2 - diretorio
	int numero_inode; // numero do inode (talvez não seja necessário)
	short int valido; // 1 para inode em uso, 0 para fora de uso
	struct dataTime criacao;
	struct dataTime acesso; 
	int tamanho; // tamanho, em bytes, do conteúdo do arquivo
	
	int blocos[10]; // armazena os numeros para os blocos de dados diretos
	int ind_bloco; // indica o bloco de dados onde está a estrutura bloco_indireto

} inode;

typedef struct dir_entry
{
	char nome[11];
	int numero_inode;
} dir_entry;

#endif
