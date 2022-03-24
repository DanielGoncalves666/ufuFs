#ifndef UFUFS_H
#define UFUFS_H

/*
Este módulo contém funções relacionadas com a operação do sistema de arquivos em si. Como abertura do device driver e manipulação das estruturas de metadados.

*/

#include<math.h>

#define MAXIMUM_OPEN_FILES 50
#define SEEK_SET 1
#define SEEK_CUR 2
#define SEEK_END 3

extern superblock sb;
extern fd = -1;
extern file_descriptor *fd_table[MAXIMUM_OPEN_FILES];

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
		fd_table[i] = NULL;
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
		if(fd_table[i] == NULL)
			continue;		
			
		fd_table[i] = (file_descriptor *) calloc(1,sizeof(file_descriptor));	
		
		fd_table[i]->inode_data = atual;
		strcpy(fd_table->nome,entrada.nome);
		fd_table->offset = 0;
		fd_table->tamanho = atual.tamanho;
		
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
int ufufs_read(int fd, void *destino, int qtd)
{
	if(fd < 0 || fd >= MAXIMUM_OPEN_FILES || fd_table[fd] == NULL)
		return -1;
		
	void *buffer = calloc(1,BLOCK_SIZE);	
		
	int inicio = sb.data_table_begin + fd_table[fd]->inode_data.bloco_inicial;
	int fim = sb.data_table_begin + fd_table[fd]->inode_data.bloco_final;
	int bloco_offset = fd_table[fd]->offset / BLOCK_SIZE; // bloco onde o próximo byte a ser lido está localizado
	int inside_offset = fd_table[fd]->offset % BLOCK_SIZE; // próximo byte dentro do bloco a ser lido
	int restante = BLOCK_SIZE - inside_offset + 1;
	int lidos = 0, ler, ultimo_byte;
	
	for(int i = inicio + bloco_offset; i<= fim; i++)
	{
		if(ler_bloco(fd,i,buffer) == 0)
			return -1;
			
		if(i == fim)
		{
			ultimo_byte = fd_table[fd].tamanho % BLOCK_SIZE;
			
			if( qtd <= ultimo_byte)
			{
				memcpy(destino + lidos,buffer, qtd);
				lidos += qtd;
			}
			else
			{
				memcpy(destino + lidos,buffer, ultimo_byte)
				lidos += ultimo_byte;
			}
			break;
		}		
			
		if(restante > qtd)
		{
			// apesar do bloco n ter sido todo lido, será lido menos doq o restante
			memcpy(destino,buffer + inside_offset, qtd);
			lidos = qtd;
			break;		
		}
		else if(restante < 0)
		{
			// blocos desde o inicio
			if(qtd < BLOCK_SIZE)
			{
				memcpy(destino + lidos,buffer, qtd);
				lidos += qtd;
				break;
			}
			else
			{
				memcpy(destino + lidos,buffer, BLOCK_SIZE);
				lidos += BLOCK_SIZE;
				qtd -= BLOCK_SIZE;	
			}
		}
		else
		{
			// le o resto do bloco
			memcpy(destino,buffer + inside_offset, restante);
			qtd -= restante;
			lidos += restante;	
			restante = -1;	
		}
	}
	
	fd_table[fd]->offset += lidos;
	free(buffer);
	return lidos;
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


/*
	   SEEK_SET 1
              The file offset is set to offset bytes.

       SEEK_CUR 2
              The file offset is set to its current location plus offset bytes.

       SEEK_END 3
              The file offset is set to the size of the file plus offset bytes.
*/
int ufufs_seek(int fd, unsigned int offset, int flags)
{
	if(fd < 0 || fd >= MAXIMUM_OPEN_FILES || fd_table[fd] == NULL)
		return -1;

	if(flags < 1 || flags > 3)
		return -1;
		
	if(flags == SEEK_SET)
	{
		fd_table[fd]->offset = offset - 1;
	}
	
	if(flags == SEEK_CUR)
	{
		fd_table[fd]->offset += offset;
	}
	
	if(flags == SEEK_END)
	{
		fd_table[fd]->offset = fd_table[fd]->tamanho + offset - 1;
	}
	
	return 0;
}

int ufufs_close(int fd)
{
	if(fd < 0 || fd >= MAXIMUM_OPEN_FILES || fd_table[fd] == NULL)
		return -1;
		
	free(fd_table[fd]);
	fd_table[fd] = NULL;
	
	return 0;
}

#endif
