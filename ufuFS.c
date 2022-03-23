#ifndef UFUFS_H
#define UFUFS_H

/*
Este módulo contém funções relacionadas com a operação do sistema de arquivos em si. Como abertura do device driver e manipulação das estruturas de metadados.

*/

#include<math.h>

#define MAXIMUM_OPEN_FILES

extern superblock sb;
extern fd = -1;
extern file_descriptor fd_table[MAXIMUM_OPEN_FILES];

int ufufs_mount(char *dispositivo)
{
	int num_blocks;
	int magic_number;
	void *primeiro_bloco = calloc(1,BLOCK_SIZE);
	
	num_blocks = abrir_dispositivo(dispositivo,&fd);
	if(num_blocks < 0)
	{
		return -1;
	}
	
	if(ler_bloco(fd,0,primeiro_bloco) == 0)
	{
		return -1
	}
	
	memcpy(&magic_number,primeiro_bloco,sizeof(int));
	if(magic_number != UFUFS_MAGIC_NUMBER)
	{
		fprintf(stderr,"\nufuFS não identificado\n");
		return -1;
	}
	
	memcpy(&sb,primeiro_bloco,sizeof(superblock)); // carrega o superblock em memória
	
	for(int i = 0; i < MAXIMUM_OPEN_FILES; i++)
	{
		fd_table[i] = -1;
	}
	
	free(primeiro_bloco);
	return 0;
}


int ufufs_open(char *pathname)
{
	char token[MAXIMUM_NAME_LENGTH + 1];
	int i,h,qtd,offset;
	inode atual;
	dir_entry entrada;
	void *buffer = calloc(1,BLOCK_SIZE);
	
	int retorno = read_inode(fd,sb.file_table_begin,0,&atual);// carrega o inode do diretório raiz
	if(!retorno)
		return -1;
		
	while(1)
	{
		token = strtok(pathname, "/");
		// se token for uma string vazia então temos em dir o inode do arquivo que queremos
		if(token[0] == '\0')
			break;
		
		h = 0;
		qtd = atual.tamanho / sizeof(dir_entry);
		for(i = atual.bloco_inicial; i <= atual.bloco_final; i++)
		{
			ler_bloco(fd, sb.data_bitmap_begin + i, buffer);
			offset = 0;
			while(h < qtd)
			{
				memcpy(&entrada,buffer + offset,sizeof(dir_entry));
				offset += sizeof(dir_entry);
				
				if(entrada.numero_inode == -1)
				{
					// inválida
					continue;
				}	
				
				if( strcmp(token,entrada.nome) == 0)
				{
					// verifica se a entrada é a mesma que o nome
					read_inode(fd,sb.file_table_begin,entrada.numero_inode,&atual);
					h = -1;
					break;
				}
				
				h++;
			}
			
			if(h == qtd)
				return -1;
			
			if(h == -1)
				break;	
		}
	}

	free(buffer);
	for(i = 0; i < MAXIMUM_OPEN_FILES; i++)
	{
		if(fd_table[i] == -1)
			continue;		
		
		fd_table[i].inode_data = atual;
		strcpy(fd_table.nome,entrada.nome);
		fd_table.offset = 0;
		
		return i;
	}

	fprintf(stderr,"\nQuantidade máxima de arquivos abertos atingida.\n");
	return -1;
}

/*
ufufs_read
----------
Entrada: inteiro indicando o file descriptor do arquivo, ponteiro para o posição da memória onde os dados lidos devem ser armazenados, inteiro indicando a quantidade de bytes a serem lidos
Descrição: 
Saída: -1 em falha, quantidade de bytes lidos em sucesso
*/
int ufufs_read(int fd, void *buffer, int qtd)
{
	// teremos que usar a estrutura em memória do inode, essa estrutura vai ter um valor chamado de CURSOR que indica o proximo byte a ser lido
	// em falta dessa estrutura, referencia a esse valor será dada por CURSOR
	
	int b = CURSOR / BLOCK_SIZE; //determina o bloco de dados onde está o próximo byte a ser lido
	int pode_lido = BLOCK_SIZE - (CURSOR % BLOCK_SIZE);//quantidade de bytes que podem ser lidos do primeiro bloco 

	//  void *memcpy(void *dest, const void *src, size_t n);

	/*
		recebe o file descriptor obtido por meio do open
		realizamos então uma busca pelos ponteiros de dados pelo bloco que contém os bytes requeridos
	*/
}

/*
ufufs_write
----------
Entrada: inteiro indicando o file descriptor do arquivo, ponteiro para o posição da memória onde os dados a serem escritos estão armazenados, inteiro indicando a quantidade de bytes a serem escritos
Descrição: 
Saída: -1 em falha, quantidade de bytes escritos em sucesso
*/
int ufufs_write(int fd, void *buffer, int qtd)
{
	// verifica se CURSOR (consultar explicação em ufufs_read) é 0, nesse caso todos os data_blocks devem ser desconsiderados e liberados se não forem ocupados
	// se o CURSOR for maior que zero é pra colocar os novos bytes à partir dessa posição alocando novos data_blocks sempre que necessário


	/*
		recebe o file descriptor obtido por meio do open
		teremos que verificar se é pra fazer append ou se é pra reescrever o arquivo
		sempre que precisar de um novo bloco precisamos alocar mais para o arquivo, alterando assim o bitmap de dados e os dados no inode	
	*/
}

int ufufs_seek(int fd, int offset, int flags)
{
	/*
		recebe o file descriptr obtido por meio do open
		altera o "cursor" do arquivo. Esse cursor é armazenado no file descriptor do arquivo em memória.
		dada uma quantidade de bytes, procuramos pelo bloco específico.
	*/
}

int ufufs_close(int fd)
{
	/*
		recebe o file descriptr obtido por meio do open
		realiza o salvamento de qualquer dados não salvos em disco e então desaloca as estruturas criadas por meio do open
	*/
}

#endif
