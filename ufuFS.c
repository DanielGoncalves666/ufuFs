/*
Este módulo contém funções relacionadas com a operação do sistema de arquivos em si. Como abertura do device driver e manipulação das estruturas de metadados.

*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<sys/types.h>
#include<unistd.h>
#include<time.h>
#include"ufuFS.h"


superblock sb;
int div_fd = -1;
file_descriptor *fd_table[MAXIMUM_OPEN_FILES];
extern int inode_number;
extern int inodes_in_a_block;

/*
ufufs_mount
------------
Entrada: string contendo o nome do dispositivo
Descrição: monta o dispositivo
Saída: -1, em falha, 0 em sucesso
*/
int ufufs_mount(char *dispositivo)
{
	int num_blocks;
	void *primeiro_bloco = calloc(1,BLOCK_SIZE);
	
	num_blocks = abrir_dispositivo(dispositivo,&div_fd);
	if(num_blocks < 0)
	{
		return -1;
	}

	if(ler_bloco(div_fd,0,primeiro_bloco) == 0)
	{
		return -1;
	}

	memcpy(&sb,primeiro_bloco,sizeof(superblock));

	if(sb.magic_number != UFUFS_MAGIC_NUMBER)
	{
		return -1;
	}
	
	inode_number = sb.inode_table_length * (BLOCK_SIZE / sizeof(inode));
	inodes_in_a_block = BLOCK_SIZE / sizeof(inode);
	
	for(int i = 0; i < MAXIMUM_OPEN_FILES; i++)
	{
		fd_table[i] = NULL;
	}
	
	free(primeiro_bloco);
	return 0;
}

/*
ufufs_unmount
------------
Entrada: nada
Descrição: desmonta o dispositivo
Saída:nada
*/
void ufufs_unmount()
{
	for(int i = 0; i < MAXIMUM_OPEN_FILES; i++)
	{	
		free(fd_table[i]);
	}
	
	if(div_fd != -1)
		close(div_fd);
}

// cria o file descriptor da raiz com sucesso
int ufufs_open(char *pathname, int flags)
{
	char *token;
	int i,h,qtd,offset;
	inode atual;
	dir_entry entrada;
	void *buffer = calloc(1,BLOCK_SIZE);
	
	int retorno = read_inode(div_fd,sb.file_table_begin,ROOT_DIRECTORY_INODE,&atual);// carrega o inode do diretório raiz
	strcpy(entrada.nome,"/");
	entrada.numero_inode = 0;

	if(!retorno)
		return -1;

	token = strtok(pathname, "/");
	do
	{	
		// se token for uma string vazia então temos em dir o inode do arquivo que queremos
		if(token == NULL)
			break;
		

		h = 0;
		qtd = atual.tamanho / sizeof(dir_entry);
		for(i = atual.bloco_inicial; i <= atual.bloco_final; i++)
		{
			ler_bloco(div_fd, sb.data_table_begin + i, buffer);
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
			{
				free(buffer);
				return -1;
			}
			
			if(h == -1)
				break;	
		}
		
		token = strtok(NULL, "/");
	}while(1);

	free(buffer);
	for(i = 0; i < MAXIMUM_OPEN_FILES; i++)
	{
		if(fd_table[i] != NULL)
			continue;		

		fd_table[i] = (file_descriptor *) calloc(1,sizeof(file_descriptor));	
		mudar_horario(&(atual.acesso));
	    write_inode(div_fd, sb.file_table_begin, atual.inode_num, &atual); // atualiza o inode

		fd_table[i]->inode_data = atual;
		strcpy(fd_table[i]->nome,entrada.nome);
		fd_table[i]->offset = 0;
		fd_table[i]->tamanho = atual.tamanho;
		fd_table[i]->escrita = flags;
	
		return i;
	}

	fprintf(stderr,"\nQuantidade máxima de arquivos abertos atingida.\n");
	return -1;
}

/*
ufufs_read
----------
Entrada: inteiro indicando o file descriptor do arquivo, escritos para o posição da memória onde os dados lidos devem ser armazenados, inteiro indicando a quantidade de bytes a serem lidos
Descrição: tenta ler qtd bytes do arquivo indicado pelo file descriptor fd. Os bytes lidos são colocados em destino. Se caso qtd for maior que o tamanho do arquivo, apenas tamanho bytes são lidos.
Saída: -1 em falha, quantidade de bytes lidos em sucesso
*/
int ufufs_read(int fd, void *destino, int qtd)
{

	if(fd < 0 || fd >= MAXIMUM_OPEN_FILES || fd_table[fd] == NULL || fd_table[fd]->escrita == WRITE_ONLY)
		return -1;
		
	if(fd_table[fd]->offset > fd_table[fd]->tamanho) // se o offset estiver posicionado além do tamanho do arquivo 
		return 0; // neste caso o offset ainda estará além do tamanho
		
	if(qtd > fd_table[fd]->tamanho - fd_table[fd]->offset) // se requisitar mais bytes do que o tamanho depois do offset
		qtd = fd_table[fd]->tamanho - fd_table[fd]->offset;
	
	void *buffer = calloc(1,BLOCK_SIZE);	
		
	int inicio = sb.data_table_begin + fd_table[fd]->inode_data.bloco_inicial;
	int fim = sb.data_table_begin + fd_table[fd]->inode_data.bloco_final;
	int bloco_offset = fd_table[fd]->offset / BLOCK_SIZE; // bloco onde o próximo byte a ser lido está localizado
	int inside_offset = fd_table[fd]->offset % BLOCK_SIZE; // próximo byte dentro do bloco a ser lido
	int restante = BLOCK_SIZE - inside_offset;
	int lidos = 0;
	int i;

	for(i = inicio + bloco_offset; i<= fim; i++)
	{
		if(ler_bloco(div_fd,i,buffer) == 0)
			return -1;

		if(i == fim)
		{					
			// último bloco do arquivo ( a qtd sempre será menor que BLOCK_SIZE)
				// se começar desde o começo do bloco, inside_offset será zero
				// talvez seja possível eliminar essa posição
			memcpy(destino + lidos,buffer + inside_offset, qtd);
			lidos += qtd;
			
			break;
		}		
			
		if(restante > qtd)
		{
			// menos bytes que o restante em um bloco serão lidos ( só pode ocorrer com o primeiro bloco)
			memcpy(destino,buffer + inside_offset, qtd);
			lidos = qtd;
			break;		
		}
		else if(restante < 0)
		{
			if(qtd < BLOCK_SIZE)
			{
				// lê um bloco desde o início, mas não inteiro
				memcpy(destino + lidos,buffer, qtd);
				lidos += qtd;
				break;
			}
			else
			{
				// lê blocos inteiros
				memcpy(destino + lidos,buffer, BLOCK_SIZE);
				lidos += BLOCK_SIZE;
				qtd -= BLOCK_SIZE;	
			}
		}
		else
		{
			// le o resto do bloco ( só pode ocorrer com o primeiro bloco )
			memcpy(destino,buffer + inside_offset, restante);
			qtd -= restante;
			lidos += restante;	
			restante = -1;	
		}
	}
	
	mudar_horario(&(fd_table[fd]->inode_data.acesso));
	fd_table[fd]->offset += lidos;
	write_inode(div_fd, sb.file_table_begin, fd_table[fd]->inode_data.inode_num, &(fd_table[fd]->inode_data)); // atualiza o inode
	
	free(buffer);
	return lidos;
}

/*
ufufs_write
----------
Entrada: inteiro indicando o file descriptor do arquivo, escritos para o posição da memória onde os dados a serem escritos estão armazenados, inteiro indicando a quantidade de bytes a serem escritos
Descrição: escreve no arquivo indicando pelo file descriptor fd. Os dados a serem escritos estão em buffer e a quantidade é indicada por qtd. A escrita começa no offset do arquivo e calcular dado depois dele é perdido. Caso seja necessário mais espaço, é alocado.
Saída: -1 em falha, quantidade de bytes escritos em sucesso
*/
int ufufs_write(int fd, void *buffer, unsigned int qtd)
{
	// não existe verificação se o offset está além do tamanho do arquivo

	if(fd < 0 || fd >= MAXIMUM_OPEN_FILES || fd_table[fd] == NULL || fd_table[fd]->escrita == READ_ONLY)
		return -1;
		
		// escrita a partir do offset
		
	inode this = fd_table[fd]->inode_data;
	int num_blocos = qtd / BLOCK_SIZE;//número de blocos completos necessários
	int resto_ser_escrito = qtd % BLOCK_SIZE; // número de bytes a serem escritos que n formam um bloco (considerando escrita desde o começo de um bloco)
	int resto_pode_ser_escrito = BLOCK_SIZE - (fd_table[fd]->tamanho % BLOCK_SIZE); // quantidade de bytes que podem ser escritos no último bloco de dados do arquivo
	
	int i, h, j, aux, retorno;
	int inside_bloco, inside_offset, blocos_extras_necessarios;
	
	int escritos = 0, escrever_meio_bloco = -1;
	int primeiro = this.bloco_inicial;	
	int ultimo = this.bloco_final;
	void *to_write = calloc(1,BLOCK_SIZE);	

	if( fd_table[fd]->offset == 0 && qtd <= fd_table[fd]->tamanho + resto_pode_ser_escrito) // quantidade a ser escrita tem que ser menor que o tamanho mais o resto do ultimo bloco ( para não precisar alocar novos blocos ) - arquivo reescrito desde o começo
	{	
		if(resto_ser_escrito > 0)
			num_blocos++;

		i = sb.data_table_begin + this.bloco_inicial; // começo da escrita
		aux = sb.data_table_begin + this.bloco_inicial + num_blocos - 1; 

		alterar_faixa_bitmap(div_fd, this.bloco_inicial, this.bloco_final,sb); // zera o bitmap			
		alterar_faixa_bitmap(div_fd, this.bloco_inicial, this.bloco_inicial + num_blocos - 1,sb); // marca as posições

		this.bloco_final = this.bloco_inicial + num_blocos - 1; // altera o valor do bloco final no inode local
		this.tamanho = qtd;
		fd_table[fd]->offset = qtd;
	}
	else if(fd_table[fd]->offset == 0 && qtd > fd_table[fd]->tamanho + resto_pode_ser_escrito)
	{	
		// precisa de alocação
		if(resto_ser_escrito > 0)
			num_blocos++;// vai precisar de um bloco a mais, que n vai ser escrito completamente
			
		for(i = ultimo + 1; i < (primeiro + num_blocos); i++)
		{
			//verifica se a quantidade necessária de bloco seguintes está livre
			retorno = get_bitmap_pos_status(div_fd, i, sb, 1);
		
			if(retorno == -1 || retorno == 1) // fora do data_table ou existe blocos ocupados, logo n pode expandir suficientemente
			{		// procura por uma sequencia de blocos suficiente e copia os blocos anteriores
				retorno = get_block_sequence(div_fd, 0,sb, num_blocos);	 //retorno contém o bloco que inicia a sequencia
				
				if(retorno == -1) // impossivel realocar
				{
					free(to_write);		
					return -1;
				}
					
				// como a escrita começa no byte 0, não é necessário copiar os blocos
					
				alterar_faixa_bitmap(div_fd, this.bloco_inicial,this.bloco_final,sb); // zera o bitmap					
				alterar_faixa_bitmap(div_fd, retorno, retorno + num_blocos - 1,sb); // marca as posições		

				this.bloco_inicial = retorno;
				this.bloco_final = retorno + num_blocos - 1;	
				fd_table[fd]->offset = fd_table[fd]->offset + qtd;	
				
				i = sb.data_table_begin + this.bloco_inicial;
				aux = sb.data_table_begin + this.bloco_final;
					
				break;
			}
		}
		
		if( retorno == 0 )
		{
			alterar_faixa_bitmap(div_fd, this.bloco_inicial,this.bloco_final,sb); // zera o bitmap
			alterar_faixa_bitmap(div_fd, this.bloco_inicial,this.bloco_inicial + num_blocos - 1,sb); // marca as posições
			
			this.bloco_final += num_blocos - 1;
			fd_table[fd]->offset = qtd;
			
			i = sb.data_table_begin + this.bloco_inicial;
			aux = sb.data_table_begin + this.bloco_final;
		}
		
		this.tamanho = qtd;
	}
	else if(fd_table[fd]->offset > 0 && qtd <= fd_table[fd]->tamanho + resto_pode_ser_escrito - fd_table[fd]->offset) // quantidade a ser escrita tem que ser menor que a quantidade de bytes disponíveis já alocados
	{	
		if(resto_ser_escrito > resto_pode_ser_escrito)
			num_blocos++;
			
		inside_bloco = fd_table[fd]->offset / BLOCK_SIZE; // blocos completos preenchidos
		inside_offset = fd_table[fd]->offset % BLOCK_SIZE;
	
		i = sb.data_table_begin + this.bloco_inicial + inside_bloco;
		aux = sb.data_table_begin + this.bloco_inicial + inside_bloco + num_blocos;
		
		alterar_faixa_bitmap(div_fd, this.bloco_inicial, this.bloco_final,sb); // zera o bitmap			
		alterar_faixa_bitmap(div_fd, this.bloco_inicial, this.bloco_inicial + inside_bloco + num_blocos,sb); // marca as posições
	
		this.bloco_final = this.bloco_inicial + inside_bloco + num_blocos;
		this.tamanho = fd_table[fd]->offset + qtd;
		fd_table[fd]->offset += qtd;	

		escrever_meio_bloco = 1;		
	}
	else
	{
		// precisa de alocação e fazer cópia dos bytes antes do offset
			//qtd - resto_pode_ser_escrito não é zerado pois esse caso entra no if anterior
		blocos_extras_necessarios = (qtd - resto_pode_ser_escrito) / BLOCK_SIZE; // blocos completos extras necessários
		resto_ser_escrito = (qtd - resto_pode_ser_escrito) % BLOCK_SIZE; // bytes a serem escritos que n formam um bloco (considerando começo da escrita no meio de um bloco), esses bytes serão escritos no novo último bloco
			
		if(resto_ser_escrito > 0)
			blocos_extras_necessarios++;// vai precisar de um bloco a mais, que n vai ser escrito completamente
			
		for(i = ultimo + 1; i <= ultimo + blocos_extras_necessarios; i++)
		{
			//verifica se a quantidade necessária de blocos seguintes está livre
		
			retorno = get_bitmap_pos_status(div_fd,i,sb,1);
			if(retorno == -1 || retorno == 1) // fora do data_table ou existe blocos ocupados, logo n pode expandir suficientemente
			{		// procura por uma sequencia de blocos suficiente e copia os blocos anteriores
				num_blocos = (ultimo - primeiro + 1) + blocos_extras_necessarios; // quantidade de blocos contíguos que deve ser procurada
				retorno = get_block_sequence(div_fd, 0,sb, num_blocos);	 //retorno contém o bloco que inicia a sequencia
				
				if(retorno == -1) // impossivel realocar
				{
					free(to_write);		
					return -1;
				}
					
				for(h = primeiro + sb.data_table_begin, j = retorno + sb.data_table_begin; h <= ultimo + sb.data_table_begin; h++, j++)
				{
					// realiza a cópia dos dados nos antigos blocos para os novos (copia também os bytes que serão sobreescritos)
					ler_bloco(div_fd,h,to_write);
					escrever_bloco(div_fd,j,to_write);
				}	
					
				alterar_faixa_bitmap(div_fd, this.bloco_inicial,this.bloco_final,sb); // zera o bitmap					
				alterar_faixa_bitmap(div_fd, retorno, retorno + num_blocos - 1,sb); // marca as posições		
				this.bloco_inicial = retorno;
				this.bloco_final = retorno + num_blocos - 1;	

				i = sb.data_table_begin + this.bloco_final;
				aux = sb.data_table_begin + this.bloco_final;
					
				break;
			}
		}											
			
		// existem blocos suficientes para o arquivo ser expandido ou ele foi realocado e agora existem blocos suficientes

		if( retorno == 0 )
		{
			// o arquivo foi expandido 
			alterar_faixa_bitmap(div_fd, this.bloco_inicial, this.bloco_final,sb); // zera o bitmap					
			alterar_faixa_bitmap(div_fd, this.bloco_inicial, this.bloco_final + blocos_extras_necessarios,sb); // marca as posições
			this.bloco_final += blocos_extras_necessarios;
		}

		inside_bloco = fd_table[fd]->offset / BLOCK_SIZE; // blocos completos preenchidos
		inside_offset = fd_table[fd]->offset % BLOCK_SIZE; // byte à começar a escrita 
					
		i = sb.data_table_begin + this.bloco_inicial + inside_bloco;
		aux = sb.data_table_begin + this.bloco_inicial + inside_bloco + blocos_extras_necessarios;
		
		this.tamanho = fd_table[fd]->offset + qtd;
		fd_table[fd]->offset = fd_table[fd]->offset + qtd;
	
		escrever_meio_bloco = 1;
	}
	
	for(; i <= aux; i++)
	{
		if(escrever_meio_bloco == 1)
		{
			// escrever no meio de um bloco
			ler_bloco(div_fd, i, to_write);
			
			if(qtd < resto_pode_ser_escrito)
			{
				memcpy(to_write + inside_offset, buffer + escritos, qtd);
				escritos += qtd;
				qtd = 0;
			}
			else
			{
				memcpy(to_write + inside_offset, buffer + escritos, resto_pode_ser_escrito);
				escritos += resto_pode_ser_escrito;
				qtd -= resto_pode_ser_escrito;
			}

			escrever_meio_bloco = -1;
		}
		else if(qtd < BLOCK_SIZE)
		{
			memset(to_write, 0, BLOCK_SIZE); // zera todo o bloco, com isso os dados não sobreescritos no disco no resto do bloco acabam sendo zerados, diferentemente de outros sistemas de arquivos
			memcpy(to_write, buffer + escritos, qtd);
			escritos += qtd;
			qtd = 0;
		}
		else
		{
			memcpy(to_write, buffer + escritos, BLOCK_SIZE);
			escritos += BLOCK_SIZE;
			qtd -= BLOCK_SIZE;
		}
							
		if( escrever_bloco(div_fd,i, to_write) == 0 )
			return -1;
							
		if(qtd == 0)
			break;
	}
	
	mudar_horario(&(this.acesso));
	
	write_inode(div_fd, sb.file_table_begin, this.inode_num, &this); // atualiza o inode
	fd_table[fd]->tamanho = this.tamanho;
	fd_table[fd]->inode_data = this;
	
	free(to_write);

	return escritos;
}

/*
	   SEEK_SET 1
              The file offset is set to offset bytes.

       SEEK_CUR 2
              The file offset is set to its current location plus offset bytes.

       SEEK_END 3
              The file offset is set to the size of the file plus offset bytes.
*/
/*
ufufs_seek
-----------
Entrada: inteiro, indicando o file descriptor
		 inteiro sem sinal, indicando a quantidade de bytes que o marcador deve ser movido
		 inteiro, indicando como mover
Descrição: Move o offset no file descriptor do arquivo
Saída: -1, em falha, 0, em sucesso

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
		
	if(flags == SEEK_SET_UFU)
	{
		fd_table[fd]->offset = offset;
	}
	
	if(flags == SEEK_CUR_UFU)
	{
		fd_table[fd]->offset += offset;
	}
	
	if(flags == SEEK_END_UFU)
	{
		fd_table[fd]->offset = fd_table[fd]->tamanho + offset;
	}
	
	mudar_horario(&(fd_table[fd]->inode_data.acesso));
	write_inode(div_fd, sb.file_table_begin, fd_table[fd]->inode_data.inode_num, &(fd_table[fd]->inode_data)); // atualiza o inode
	
	return 0;
}

/*
ufufs_close
------------
Entrada:inteiro,indicando of file descriptor do arquivo
Descrição: desaloca o file descriptor do arquivo
Saída: -1, em falha, 0, em sucesso
*/
int ufufs_close(int fd)
{
	if(fd < 0 || fd >= MAXIMUM_OPEN_FILES || fd_table[fd] == NULL)
		return -1;
		
	free(fd_table[fd]);
	fd_table[fd] = NULL;
	
	return 0;
}

/*
ufufs_size
-----------
Entrada: inteiro,indicando of file descriptor do arquivo
Descrição: calcula o tamanho do arquivo
Saída: inteiro, indicando o tamanho do arquivo, em sucesso, -1, em falha
*/
int ufufs_size(int fd)
{
	if(fd < 0 || fd >= MAXIMUM_OPEN_FILES || fd_table[fd] == NULL)
		return -1;
		
	return fd_table[fd]->tamanho;
}

/*
ufufs_tipo
-----------
Entrada: inteiro,indicando of file descriptor do arquivo
Descrição: consulta o tipo
Saída: inteiro, indicando o tipo, em sucesso, -1 em falha
*/
short int ufufs_tipo(int fd)
{
	if(fd < 0 || fd >= MAXIMUM_OPEN_FILES || fd_table[fd] == NULL)
		return -1;
		
	return fd_table[fd]->inode_data.tipo;
}


/*
ufufs_offset
-----------
Entrada: inteiro,indicando of file descriptor do arquivo
Descrição: consulta o offset
Saída: inteiro, indicando o offset, em sucesso, -1 em falha
*/
int ufufs_offset(int fd)
{	
	if(fd < 0 || fd >= MAXIMUM_OPEN_FILES || fd_table[fd] == NULL)
		return -1;
		
	return fd_table[fd]->offset;
}


/*
mudar_horario
--------------
Entrada: endereço de um struct dataTime a ser atualizada ou preenchida
Descrição: preenche a struct dataTime com o horário atual
Saída: nada
*/
void mudar_horario(struct dataTime *mudar)
{
	struct tm *agora;
	time_t raw = time(NULL);
	agora = localtime(&raw);
	
	mudar->dia = agora->tm_mday;
	mudar->mes = agora->tm_mon + 1; 
	mudar->ano = agora->tm_year + 1900;
	mudar->hora = agora->tm_hour;
	mudar->minuto = agora->tm_min; 
	mudar->segundo = agora->tm_sec;
}
