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
	
	int retorno = read_inode(div_fd,sb.file_table_begin,0,&atual);// carrega o inode do diretório raiz
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
Entrada: inteiro indicando o file descriptor do arquivo, ponteiro para o posição da memória onde os dados lidos devem ser armazenados, inteiro indicando a quantidade de bytes a serem lidos
Descrição: tenta ler qtd bytes do arquivo indicado pelo file descriptor fd. Os bytes lidos são colocados em destino. Se caso qtd for maior que o tamanho do arquivo, apenas tamanho bytes são lidos.
Saída: -1 em falha, quantidade de bytes lidos em sucesso
*/
int ufufs_read(int fd, void *destino, int qtd)
{

	if(fd < 0 || fd >= MAXIMUM_OPEN_FILES || fd_table[fd] == NULL || fd_table[fd]->escrita == WRITE_ONLY)
		return -1;
		
	if(qtd > fd_table[fd]->tamanho) // se requisitar mais bytes do que o tamanho, lê apenas tamanho bytes
		qtd = fd_table[fd]->tamanho;
		
	void *buffer = calloc(1,BLOCK_SIZE);	
		
	int inicio = sb.data_table_begin + fd_table[fd]->inode_data.bloco_inicial;
	int fim = sb.data_table_begin + fd_table[fd]->inode_data.bloco_final;
	int bloco_offset = fd_table[fd]->offset / BLOCK_SIZE; // bloco onde o próximo byte a ser lido está localizado
	int inside_offset = fd_table[fd]->offset % BLOCK_SIZE; // próximo byte dentro do bloco a ser lido
	int restante = BLOCK_SIZE - inside_offset + 1;
	int lidos = 0, ultimo_byte;
	
	for(int i = inicio + bloco_offset; i<= fim; i++)
	{
		if(ler_bloco(div_fd,i,buffer) == 0)
			return -1;
			
		if(i == fim)
		{
			ultimo_byte = fd_table[fd]->tamanho % BLOCK_SIZE;
			
			if( qtd <= ultimo_byte)
			{
				memcpy(destino + lidos,buffer, qtd);
				lidos += qtd;
			}
			else
			{
				memcpy(destino + lidos,buffer, ultimo_byte);
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
	
	inode this = fd_table[fd]->inode_data;
	mudar_horario(&(this.acesso));
	fd_table[fd]->inode_data = this;
	fd_table[fd]->offset += lidos;
	write_inode(div_fd, sb.file_table_begin, this.inode_num, &this); // atualiza o inode
	
	free(buffer);
	return lidos;
}

/*
ufufs_write
----------
Entrada: inteiro indicando o file descriptor do arquivo, ponteiro para o posição da memória onde os dados a serem escritos estão armazenados, inteiro indicando a quantidade de bytes a serem escritos
Descrição: escreve no arquivo indicando pelo file descriptor fd. Os dados a serem escritos estão em buffer e a quantidade é indicada por qtd. A escrita começa no offset do arquivo e calcular dado depois dele é perdido. Caso seja necessário mais espaço, é alocado.
Saída: -1 em falha, quantidade de bytes escritos em sucesso
*/
int ufufs_write(int fd, void *buffer, unsigned int qtd)
{
	if(fd < 0 || fd >= MAXIMUM_OPEN_FILES || fd_table[fd] == NULL || fd_table[fd]->escrita == READ_ONLY)
		return -1;
		
		// escrita a partir do offset
		
	inode this = fd_table[fd]->inode_data;
	int offset = fd_table[fd]->offset;
	int tamanho = fd_table[fd]->tamanho;
	int num_blocos = qtd / BLOCK_SIZE;//número de blocos completos necessários
	int resto_ser_escrito = qtd % BLOCK_SIZE; // número de bytes a serem escritos que n formam um bloco
	int resto_pode_ser_escrito = BLOCK_SIZE - (offset % BLOCK_SIZE); // quantidade de bytes que podem ser escritos no último bloco de dados do arquivo
	
	int i, ponteiro,aux, retorno;
	int ultimo, primeiro, novo_inicio = -1;
	int inside_bloco,inside_offset, blocos_extras_necessarios;
	
	void *to_write = calloc(1,BLOCK_SIZE);	
	
	
	if( offset == 0 && qtd <= tamanho + resto_pode_ser_escrito) // quantidade a ser escrita tem que ser menor que o tamanho mais o resto do ultimo bloco
	{
		// reescreve todo o arquivo
		
		if(resto_ser_escrito > 0) // ultimo bloco será incompleto
			num_blocos++;
		
		i = sb.data_table_begin + this.bloco_inicial; // começo da escrita
		aux = sb.data_table_begin + this.bloco_inicial + num_blocos; 
		
		alterar_faixa_bitmap(div_fd, this.bloco_inicial,this.bloco_final,sb); // zera o bitmap
		this.bloco_final = aux - 1; // altera o valor do bloco final no inode local
		this.tamanho = qtd;
		fd_table[fd]->offset = qtd;
		
		alterar_faixa_bitmap(div_fd, this.bloco_inicial,this.bloco_final,sb); // marca as posições
		
		ponteiro = 0;
	}
	else if(offset == 0 && qtd > tamanho + resto_pode_ser_escrito)
	{
		ultimo = this.bloco_final;
		primeiro = this.bloco_inicial;
		
		// precisa de alocação
		blocos_extras_necessarios = (qtd - tamanho) / BLOCK_SIZE; // blocos completos extras necessários
		resto_ser_escrito = (qtd - tamanho) % BLOCK_SIZE; // bytes a serem escritos que n formam um bloco
		resto_pode_ser_escrito =  BLOCK_SIZE - (tamanho % BLOCK_SIZE); // bytes que podem ser escritos no ultimo bloco
		resto_ser_escrito -= resto_pode_ser_escrito; // bytes a serem escritos que n formam um bloco
			
		if(resto_ser_escrito > 0)
			blocos_extras_necessarios++;// vai precisar de um bloco a mais, que n vai ser escrito completamente
			
		for(i = ultimo + 1; i < blocos_extras_necessarios; i++)
		{
			//verifica se a quantidade necessária de bloco seguintes está livre
		
			retorno = get_bitmap_pos_status(fd,i,sb,1);
			
			if(retorno == -1 || retorno == 1) // fora do data_table ou existe blocos ocupados, logo n pode expandir suficientemente
			{		// procura por uma sequencia de blocos suficiente e copia os blocos anteriores
				num_blocos += (ultimo - primeiro + 1) + blocos_extras_necessarios;
				retorno = get_block_sequence(div_fd, 0,sb, num_blocos);	 //retorno contém o bloco que inicia a sequencia
				
				if(retorno == -1) // impossivel realocar
					return -1;
					
				break;
			}
		}
			
		if(novo_inicio != -1)
		{
			alterar_faixa_bitmap(div_fd, this.bloco_inicial,this.bloco_final,sb); // zera o bitmap
					
			this.bloco_inicial = retorno;
			this.bloco_final = retorno + num_blocos - 1;
			this.tamanho = offset + qtd;
			fd_table[fd]->offset = offset + qtd;
					
			alterar_faixa_bitmap(div_fd,this.bloco_inicial,this.bloco_final,sb); // marca as posições

			// daqui pra frente é escrever qtd == tamanho bytes desde o começo

			i = sb.data_table_begin + this.bloco_inicial;
			aux = sb.data_table_begin + this.bloco_final + 1;
			
			ponteiro = 0;
		}	
			
		// existem blocos suficientes para o arquivo ser expandido
		if(retorno == 0)
		{
			i = sb.data_table_begin + this.bloco_final + 1;
			aux = sb.data_table_begin + this.bloco_final + blocos_extras_necessarios + 1;
		
			alterar_faixa_bitmap(div_fd, this.bloco_inicial,this.bloco_final,sb); // zera o bitmap
			this.tamanho = offset + qtd;
			fd_table[fd]->offset = offset + qtd;
			this.bloco_final += blocos_extras_necessarios;			
			alterar_faixa_bitmap(div_fd, this.bloco_inicial,this.bloco_final,sb); // remarca o bitmap
				
			qtd -= resto_pode_ser_escrito;	
			ponteiro = resto_pode_ser_escrito;
		}
	
	}
	else if(offset > 0 && qtd <= tamanho + resto_pode_ser_escrito - offset) // quantidade a ser escrita tem que ser menor que a quantidade de bytes disponíveis já alocados
	{
		if(resto_ser_escrito > resto_pode_ser_escrito)
			num_blocos++;
			
		inside_bloco = offset / BLOCK_SIZE; // blocos completos
		inside_offset = offset % BLOCK_SIZE; 
	
		ler_bloco(div_fd, sb.data_table_begin + inside_bloco + 1, to_write);
		memcpy(to_write + inside_offset,buffer,resto_pode_ser_escrito); // preenche o bloco
		qtd -= resto_pode_ser_escrito;
		
		escrever_bloco(div_fd,sb.data_table_begin + inside_bloco + 1, to_write);
	
		i = sb.data_table_begin + inside_bloco + 2;
		aux = sb.data_table_begin + inside_bloco + 2 + num_blocos;
			
		alterar_faixa_bitmap(div_fd, this.bloco_inicial,this.bloco_final,sb); // zera o bitmap
			
		this.bloco_final = aux - 1; // altera o inode
		this.tamanho = offset + qtd;
		fd_table[fd]->offset = offset + qtd;
			
		alterar_faixa_bitmap(div_fd, this.bloco_inicial,this.bloco_final,sb); // marca as posições
	
		ponteiro = resto_pode_ser_escrito;		
	}
	else
	{
		// precisa de alocação
		ultimo = this.bloco_final;
		primeiro = this.bloco_inicial;


		blocos_extras_necessarios = (qtd - tamanho) / BLOCK_SIZE; // blocos completos extras necessários
		resto_ser_escrito = (qtd - tamanho) % BLOCK_SIZE; // bytes a serem escritos que n formam um bloco
		resto_pode_ser_escrito =  BLOCK_SIZE - (tamanho % BLOCK_SIZE); // bytes que podem ser escritos no ultimo bloco
		resto_ser_escrito -= resto_pode_ser_escrito; // bytes a serem escritos que n formam um bloco
			
		if(resto_ser_escrito > 0)
			blocos_extras_necessarios++;// vai precisar de um bloco a mais, que n vai ser escrito completamente
			
		for(i = ultimo + 1; i < blocos_extras_necessarios; i++)
		{
			//verifica se a quantidade necessária de bloco seguintes está livre
		
			retorno = get_bitmap_pos_status(fd,i,sb,1);
			
			if(retorno == -1 || retorno == 1) // fora do data_table ou existe blocos ocupados, logo n pode expandir suficientemente
			{		// procura por uma sequencia de blocos suficiente e copia os blocos anteriores
				num_blocos += (ultimo - primeiro + 1) + blocos_extras_necessarios;
				retorno = get_block_sequence(div_fd, 0,sb, num_blocos);	 //retorno contém o bloco que inicia a sequencia
				
				if(retorno == -1) // impossivel realocar
					return -1;
					
				break;
			}
		}
			
		if(novo_inicio != -1)
		{
			alterar_faixa_bitmap(div_fd, this.bloco_inicial,this.bloco_final,sb); // zera o bitmap
					
			this.bloco_inicial = retorno;
			this.bloco_final = retorno + num_blocos - 1;
			this.tamanho = offset + qtd;
			fd_table[fd]->offset = offset + qtd;
					
			alterar_faixa_bitmap(div_fd,this.bloco_inicial,this.bloco_final,sb); // marca as posições

			// daqui pra frente é escrever qtd == tamanho bytes desde o começo

			i = sb.data_table_begin + this.bloco_inicial;
			aux = sb.data_table_begin + this.bloco_final + 1;
			
			ponteiro = 0;
		}	
			
		// existem blocos suficientes para o arquivo ser expandido
		if(retorno == 0)
		{
	
			inside_bloco = offset / BLOCK_SIZE; // blocos completos
			inside_offset = offset % BLOCK_SIZE; 
	
			ler_bloco(div_fd, sb.data_table_begin + inside_bloco + 1, to_write);
			memcpy(to_write + inside_offset,buffer,resto_pode_ser_escrito); // preenche o bloco
			
			escrever_bloco(div_fd,sb.data_table_begin + inside_bloco + 1, to_write);
					
			i = sb.data_table_begin + this.bloco_final + 1;
			aux = sb.data_table_begin + this.bloco_final + blocos_extras_necessarios + 1;
		
			alterar_faixa_bitmap(div_fd, this.bloco_inicial,this.bloco_final,sb); // zera o bitmap
			this.tamanho = offset + qtd;
			fd_table[fd]->offset = offset + qtd;
			this.bloco_final += blocos_extras_necessarios;			
			alterar_faixa_bitmap(div_fd, this.bloco_inicial,this.bloco_final,sb); // remarca o bitmap
				
			qtd -= resto_pode_ser_escrito;	
			ponteiro = resto_pode_ser_escrito;
		}
		
	}
	
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
			qtd -= BLOCK_SIZE;
		}
							
		escrever_bloco(div_fd,i, to_write);
							
		if(qtd == 0)
			break;
	}
	
	mudar_horario(&(this.acesso));
	
	write_inode(div_fd, sb.file_table_begin, this.inode_num, &this); // atualiza o inode
	fd_table[fd]->tamanho = this.tamanho;
	fd_table[fd]->inode_data = this;

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
/*
ufufs_seek
-----------
Entrada: inteiro, indicando o file descriptor
		 inteiro sem sinal, indicando para onde mover o offset
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
		fd_table[fd]->offset = offset - 1;
	}
	
	if(flags == SEEK_CUR_UFU)
	{
		fd_table[fd]->offset += offset;
	}
	
	if(flags == SEEK_END_UFU)
	{
		fd_table[fd]->offset = fd_table[fd]->tamanho + offset - 1;
	}

	inode atual;	
	mudar_horario(&(atual.acesso));
	write_inode(div_fd, sb.file_table_begin, atual.inode_num, &atual); // atualiza o inode
	fd_table[fd]->inode_data = atual;
	
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
