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

#define APPEND 1 // append no final do arquivo
#define OVERWRITTEN 2 // reescreve desde o começo
#define APPEND_AT 3 // append a partir do offset

extern superblock sb;
extern div_fd = -1;
extern file_descriptor *fd_table[MAXIMUM_OPEN_FILES];

int ufufs_mount(char *dispositivo)
{
	int num_blocks;
	int magic_number;
	void *primeiro_bloco = calloc(1,BLOCK_SIZE);
	
	num_blocks = abrir_dispositivo(dispositivo,&div_fd);
	if(num_blocks < 0)
	{
		return -1;
	}
	
	if(ler_bloco(div_fd,0,primeiro_bloco) == 0)
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


int ufufs_open(char *pathname, int flags)
{
	char token[MAXIMUM_NAME_LENGTH + 1];
	int i,h,qtd,offset;
	inode atual;
	dir_entry entrada;
	void *buffer = calloc(1,BLOCK_SIZE);
	
	int retorno = read_inode(div_fd,sb.file_table_begin,0,&atual);// carrega o inode do diretório raiz
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
			ler_bloco(div_fd, sb.data_bitmap_begin + i, buffer);
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
					read_inode(div_fd,sb.file_table_begin,entrada.numero_inode,&atual);
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
		fd_table->escrita = flags;
		
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
		if(ler_bloco(div_fd,i,buffer) == 0)
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
int ufufs_write(int fd, void *buffer, unsigned int qtd)
{
	if(fd < 0 || fd >= MAXIMUM_OPEN_FILES || fd_table[fd] == NULL)
		return -1;
		
	inode this = fd_table[fd]->inode_data;
	int escrita = fd_table[fd]->escrita;
	int offset = fd_table[fd]->offset;
	int tamanho = fd_table[fd]->tamanho;
	int num_blocos = qtd / BLOCK_SIZE;//número de blocos completos necessários
	int resto_ser_escrito = qtd % BLOCK_SIZE; // número de bytes a serem escritos que n formam um bloco
	int resto_pode_ser_escrito; // quantidade de bytes que podem ser escritos no último bloco de dados do arquivo
	
	int i, h, j, ponteiro,aux, retorno;
	int ultimo, local;
	void *to_write = calloc(1,BLOCK_SIZE);	
	
	if(escrita == OVERWRITTEN)
	{
		// se a reescrita contiver menos bytes
		if(qtd <= tamanho)
		{
			i = sb.data_table_begin + this.bloco_inicial;
			aux = sb.data_table_begin + this.bloco_inicial + num_blocos;
			
			alterar_faixa_bitmap(div_fd, this.bloco_inicial,this.bloco_final,sb); // zera o bitmap
			
			this.bloco_final = aux - 1; // altera o inode
			this.tamanho = qtd;
			
			alterar_faixa_bitmap(div_fd, this.bloco_inicial,this.bloco_final,sb); // marca as posições
			
			ponteiro = 0;
			for(; i < aux; i++)
			{
				if(qtd < BLOCK_SIZE)
				{
					memcpy(to_write, buffer + ponteiro, qtd);
					ponteiro += qtd;
					qtd = 0;
				}
				else
				{
					memcpy(to_write, buffer + ponteiro, BLOCK_SIZE);
					ponteiro += BLOCK_SIZE;
					qtd -= BLOCK_SIZE:
				}
						
				escrever_bloco(div_fd,i, to_write);
						
				if(qtd == 0)
					break;
			}
		}
		else // se contiver mais bytes
		{
			ultimo = this.bloco_final;
			int blocos_extras_necessarios = (qtd - tamanho) / BLOCK_SIZE; // blocos completos extras necessários
			resto_ser_escrito = (qtd - tamanho) % BLOCK_SIZE; // bytes a serem escritos que n formam um bloco
			resto_pode_ser_escrito =  BLOCK_SIZE - (tamanho % BLOCKSIZE); // bytes que podem ser escritos no ultimo bloco
			resto_ser_escrito -= resto_pode_ser_escrito;
			
			if(resto_ser_escrito > 0)
				blocos_extras_necessarios++;// vai precisar de um bloco a mais, que n vai ser escrito completamente
			
			for(i = ultimo + 1; i < blocos_extras_necessarios i++)
			{
				retorno = get_bitmap_pos_status(fd,i,sb,1);
			
				if(retorno == -1 || retorno == 1) // fora do data_table ou existe blocos ocupados, logo n pode expandir suficientemente
					// procura por uma sequencia de blocos suficiente e copia os blocos anteriores
					num_blocos += (ultimo - primeiro + 1);
					retorno = get_block_sequence(div_fd, 0,sb, num_blocos);	 //retorno contém o bloco que inicia a sequencia
					if(retorno == -1)
						return -1;
						
					alterar_faixa_bitmap(div_fd, this.bloco_inicial,this.bloco_final,sb); // zera o bitmap
					
					this.bloco_inicial = retorno;
					this.bloco_final = retorno + num_blocos - 1; // altera o inode
					this.tamanho = tamanho + qtd;
					
					alterar_faixa_bitmap(div_fd,this.bloco_inicial,this.bloco_final,sb); // marca as posições

					for(h = this.bloco_inicial, j = primeiro ; j <= ultimo; h++, j++)
					{
						ler_bloco(div_fd, j, to_write);
						escrever_bloco(div_fd, h, to_write);
					}
					
					memcpy(to_write + offset, buffer, BLOCK_SIZE - offset);
					qtd -= BLOCK_SIZE - offset;
					
					ponteiro = 0;
					for(; i < aux; i++)
					{
						if(qtd < BLOCK_SIZE)
						{
							memcpy(to_write, buffer + ponteiro, qtd);
							ponteiro += qtd;
							qtd = 0;
						}
						else
						{
							memcpy(to_write, buffer + ponteiro, BLOCK_SIZE);
							ponteiro += BLOCK_SIZE;
							qtd -= BLOCK_SIZE:
						}
								
						escrever_bloco(div_fd,i, to_write);
								
						if(qtd == 0)
							break;
					}
					
					break;
			}
			
			if(retorno == 0)
			{
				// o arquivo pode ser expandido
				alterar_faixa_bitmap(div_fd, this.bloco_inicial,this.bloco_final,sb); // zera o bitmap
			
				this.bloco_final += blocos_extras_necessarios;
				this.tamanho = qtd;
				
				alterar_faixa_bitmap(div_fd, this.bloco_inicial,this.bloco_final,sb); // remarca o bitmap
				
				ponteiro = 0;
				for(; i < aux; i++)
				{
					if(qtd < BLOCK_SIZE)
					{
						memcpy(to_write, buffer + ponteiro, qtd);
						ponteiro += qtd;
						qtd = 0;
					}
					else
					{
						memcpy(to_write, buffer + ponteiro, BLOCK_SIZE);
						ponteiro += BLOCK_SIZE;
						qtd -= BLOCK_SIZE:
					}
							
					escrever_bloco(div_fd,i, to_write);
							
					if(qtd == 0)
						break;
				}
			}
		
		}
	
	}
		
	
	//altera acesso
	write_inode(div_fd, sb.file_table_begin, this.inode_num, &this); // atualiza o inode
	fd_table[fd]->tamanho = this.tamanho;
	fd_table[fd]->offset += ponteiro;

	return ponteiro;
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

/*
	if(escrita == APPEND_AT)
	{
		local = offset / BLOCK_SIZE; // determina o bloco onde o offset está
		ultimo = local;
		
	
		if( tamanho >= offset + qtd)
		{
			// não precisa alocar mais blocos, talvez diminuir
		
			num_blocos -= (qtd - offset) / BLOCK_SIZE;
			resto_pode_ser_escrito = BLOCK_SIZE - (offset % BLOCKSIZE);
			resto_ser_escrito -= resto_pode_ser_escrito;
		
			if(resto_ser_escrito > 0)
				num_blocos++;// vai precisar de um bloco a mais, que n vai ser escrito completamente
			
		
		
		}
		else
		{
			// precisa alocar mais blocos

			num_blocos -= (qtd - offset) / BLOCK_SIZE;
			resto_ser_escrito = (qtd - offset) % BLOCK_SIZE;
	
			resto_pode_ser_escrito = BLOCK_SIZE - (offset % BLOCKSIZE);
			resto_ser_escrito -= resto_pode_ser_escrito;
		
			if(resto_ser_escrito > 0)
				num_blocos++;// vai precisar de um bloco a mais, que n vai ser escrito completamente
			
			for(i = ultimo + 1; i < num_blocos i++)
			{
				retorno = get_bitmap_pos_status(fd,i,sb,1);
			
				if(retorno == -1 || retorno == 1) // fora do data_table ou existe blocos ocupados, logo n pode expandir suficientemente
					// procura por uma sequencia de blocos suficiente e copia os blocos anteriores
				
			}
			// o arquivo pode ser expandido
		}
	}	
	*/
	
	/*
	if(escrita == APPEND)
	{
		ultimo = this.bloco_final;
		primeiro = this.bloco_inicial;
		resto_pode_ser_escrito = BLOCK_SIZE - (tamanho % BLOCKSIZE);
		resto_ser_escrito -= resto_pode_ser_escrito;
		
		if(resto_ser_escrito > 0)
			num_blocos++;// vai precisar de um bloco a mais, que n vai ser escrito completamente
			
		for(i = ultimo + 1; i < num_blocos_ i++)
		{
			retorno = get_bitmap_pos_status(fd,i,sb,1);
			
			if(retorno == -1 || retorno == 1) // fora do data_table ou existe blocos ocupados, logo n pode expandir suficientemente
			{
				// procura por uma sequencia de blocos suficiente e copia blocos anteriores
				num_blocos += (ultimo - primeiro + 1);
				retorno = get_block_sequence(div_fd, 0,sb, num_blocos);	 //retorno contém o bloco que inicia a sequencia
				if(retorno == -1)
					return -1;
					
				alterar_faixa_bitmap(div_fd, this.bloco_inicial,this.bloco_final,sb); // zera o bitmap
				
				this.bloco_inicial = retorno;
				this.bloco_final = retorno + num_blocos - 1; // altera o inode
				this.tamanho = tamanho + qtd;
				alterar_faixa_bitmap(div_fd,this.bloco_inicial,this.bloco_final,sb); // marca as posições

				write_inode(div_fd, sb.file_table_begin, this.inode_num, &this); // atualiza o inode
				for(h = this.bloco_inicial, j = primeiro ; j <= ultimo; h++, j++)
				{
					ler_bloco(div_fd, j, to_write);
					escrever_bloco(div_fd, h, to_write);
				}
				
				break;
			}
		}
		
		if(retorno == 0)
		{
			// o arquivo pode ser expandido
		}
	}
	*/

#endif
