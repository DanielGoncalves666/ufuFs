#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<fcntl.h>
#include<unistd.h>
#include"ufuFS.h"

/*
os camandos seguem um padrão de separação entre argumentos com espaço. Qualquer espaço é identificado como separador, logo esse shell não suporta nomes com espaço

*/

extern superblock sb;
extern int div_fd;
extern file_descriptor *fd_table[MAXIMUM_OPEN_FILES];

void ufufs_help();
void ufufs_shell();
void ufufs_list(char *caminho);
int ufufs_create_arquivo(char *caminho, char *nome);
void ufufs_create_directory(char *caminho, char *nome);
void ufufs_delete(char *caminho, char *nome);
void copy_real_to_ufufs(char *real, char *caminho, char *nome);
void copy_ufufs_to_real(char *aqui, char *real);
void ufufs_read_shell(char *caminho_nome);
void ufufs_write_shell(char *caminho_nome);

int main()
{
	ufufs_shell();

	return 0;
}

/*
ufufs_help
------------
Entrada: nada
Descrição: mostra os comandos disponíveis
Saída: nada
*/
void ufufs_help()
{
    printf("Lista de Comandos:\n");
    printf("Obs: Espaços separam o comando de seus argumentos\n");
    printf("\tcreate_arq <caminho> <nome>\n"); // caminho precisa terminar em um diretório
    printf("\tcreate_dir <caminho> <nome>\n"); // caminho precisa terminar em um diretório
    printf("\tdelete <caminho> <nome>\n"); // se for um diretório precisa estar vazio
    printf("\tlist <caminho>\n"); // se for um arquivo lista somente as informações dele 
    printf("\tcopy_ufufs_to_real <ufu_arquivo> <real_arquivo>\n"); 
    printf("\tcopy_real_to_ufufs <real_arquivo> <caminho> <nome>\n");// caminho precisa terminar em um diretório
    printf("\twrite <caminho_nome>\n"); // caminho_nome precisa ser um arquivo
    printf("\tread <caminho_nome>\n"); // caminho_nome precisa ser um arquivo
    printf("\tclear\n"); // limpa a tela
    printf("\thelp\n"); // mostra esta lista
    printf("\texit\n"); // fecha o programa
}


/*
ufufs_shell
------------
Entrada: nada
Descrição: realiza a montagem do pendrive e a dinâmica de recebimento e processamento de comandos
Saída: nada
*/
void ufufs_shell()
{
	// argumentos extras são ignorados

	char nome_dispositivo[15], linha_comando[256];
	int check;
	int device_file_montado = -1;
	char *comando, *arg1, *arg2, *arg3;

	printf("\e[1;1H\e[2J");
    printf("Bem vindo ao Shell de Navegacao ufuFS\n\n");
	
	while(1)
	{
	  	if(device_file_montado <= 0)
	  	{
		  	printf("\nInsira o nome (ex: /dev/sdXX) do dispositivo que contenha um ufuFS para montá-lo.\n");
		  	scanf("%s",nome_dispositivo);
		  	if(device_file_montado == -1 && ufufs_mount(nome_dispositivo) == 0)
		  	{
			  	printf("Dispositivo montado com sucesso!\n");
			  	device_file_montado = 1;
		  	}
		  	else
		  	{
		  		if(device_file_montado == 0)
		  		{
		  		 	if(access(nome_dispositivo, F_OK) == 0)
		  		 	{
		  		 		device_file_montado = 1;
		  				continue;
		  			}
		  		}
		  	
		  		printf("Falha! Device file não encontrado ou ufuFS não identificado no dispositivo.\n");
		  		printf("Deseja tentar novamente? 1 - sim, outro - não\n");
		  		scanf("%d",&check);
		  		if(check != 1)
		  		{
		  			printf("\nFechando...\n");
		  			return;
		  		}
		  	}
		}

  		if(access(nome_dispositivo, F_OK)!= 0) // verifica se o dispositivo ainda está conectado
  		{
  			device_file_montado = 0;
  			continue;
  		}
  	
  		printf("\nufu_shell: ");

  		check = scanf(" %[^\n]", linha_comando);
  		comando = strtok (linha_comando, " ");
  	
  		if(strcmp(comando,"create_arq") == 0)
  		{
  			arg1 = strtok (NULL, " ");
  			arg2 = strtok (NULL, " ");
  			if(arg1 == NULL || arg2 == NULL)
  			{
  				printf("\nArgumentos insuficientes.\n");
  				continue;
  			}
  			
  			ufufs_create_arquivo(arg1,arg2);
  		}
  		else if(strcmp(comando,"create_dir") == 0)
  		{
  			arg1 = strtok (NULL, " ");
  			arg2 = strtok (NULL, " ");
  			if(arg1 == NULL || arg2 == NULL)
  			{
  				printf("\nArgumentos insuficientes.\n");
  				continue;
  			}
  			
  			ufufs_create_directory(arg1,arg2);
  		}
  		else if(strcmp(comando,"delete") == 0)
  		{
  			arg1 = strtok (NULL, " ");
  			arg2 = strtok (NULL, " ");
  			if(arg1 == NULL || arg2 == NULL)
  			{
  				printf("\nArgumentos insuficientes.\n");
  				continue;
  			}
  			
  			ufufs_delete(arg1,arg2);
  		}
  		else if(strcmp(comando,"list") == 0)
  		{
  			arg1 = strtok (NULL, " ");
  			if(arg1 == NULL)
  			{
  				printf("\nArgumentos insuficientes.\n");
  				continue;
  			}
  			
  			ufufs_list(arg1);
  		}
  		else if(strcmp(comando,"copy_ufufs_to_real") == 0)
  		{
  			arg1 = strtok (NULL, " ");
  			arg2 = strtok (NULL, " ");
  			
  				printf("%s %s",arg1,arg2);
  			if(arg1 == NULL || arg2 == NULL)
  			{
  				printf("\nArgumentos insuficientes.\n");
  				continue;
  			}
  			
  			copy_ufufs_to_real(arg1,arg2);
  		}
  		else if(strcmp(comando,"copy_real_to_ufufs") == 0)
  		{
  			arg1 = strtok (NULL, " ");
  			arg2 = strtok (NULL, " ");
  			arg3 = strtok (NULL, " ");
  			if(arg1 == NULL || arg2 == NULL || arg3 == NULL)
  			{
  				printf("\nArgumentos insuficientes.\n");
  				continue;
  			}
  			
  			copy_real_to_ufufs(arg1,arg2,arg3);
  		}
  		else if(strcmp(comando,"write") == 0)
  		{
  			arg1 = strtok (NULL, " ");
  			if(arg1 == NULL)
  			{
  				printf("\nArgumentos insuficientes.\n");
  				continue;
  			}
  			ufufs_write_shell(arg1);
  		}
  		else if(strcmp(comando,"read") == 0)
  		{
  			arg1 = strtok (NULL, " ");
  			if(arg1 == NULL)
  			{
  				printf("\nArgumentos insuficientes.\n");
  				continue;
  			}
  			ufufs_read_shell(arg1);
  		}
  		else if(strcmp(comando,"clear") == 0)
  		{
  			printf("\e[1;1H\e[2J");
  		}
  		else if(strcmp(comando,"help") == 0)
  		{
  			ufufs_help();
  		}
  		else if(strcmp(comando,"exit") == 0)
  		{
  			ufufs_unmount();
  			return; // retorna para a main
  		}
  		else
  		{
  			printf("\nComando não identificado.");
  		}
  	}
}

/*
ufufs_list
------------
Entrada: string indicando o diretório a ser mostrado
Descrição: printa na tela todos os arquivos e diretórios, e suas informações, que estão contidos no diretório passado
Saída: nada
*/
void ufufs_list(char *caminho)
{
	inode entrada;
	int qtd_entradas;
	int h, atual_fd;
	dir_entry *buffer = (dir_entry *) calloc(1,sizeof(dir_entry));

	atual_fd = ufufs_open(caminho, READ_ONLY);
	if(atual_fd == -1)
	{
		printf("\nErro na abertura.");
		return;
	}

	qtd_entradas = fd_table[atual_fd]->tamanho / sizeof(dir_entry); // calcula a quantidade de entradas válidas no diretório
	for(h = 0; h < qtd_entradas;h++)
	{
		if(ufufs_read(atual_fd,buffer, sizeof(dir_entry)) == -1)
			break;
				
		if(buffer->numero_inode == -1)
			continue;// entrada de um arquivo que foi excluído
			
		printf("%10s\t", buffer->nome);
	
		read_inode(div_fd,sb.file_table_begin,buffer->numero_inode,&entrada); // carrega o inode de cada entrada do diretório
		printf("%c\t",entrada.tipo == 1? 'A' : 'D');
		printf("Criação: %02d:%02d:%02d %02d/%02d/%d\t", entrada.criacao.hora, entrada.criacao.minuto, entrada.criacao.segundo, entrada.criacao.dia, entrada.criacao.mes, entrada.criacao.ano);
		printf("Acesso: %02d:%02d:%02d %02d/%02d/%d\t", entrada.acesso.hora, entrada.acesso.minuto, entrada.acesso.segundo, entrada.acesso.dia, entrada.acesso.mes, entrada.acesso.ano);
		printf("Tamanho: %5u bytes\n",entrada.tamanho);
	}
	
	free(buffer);
	ufufs_close(atual_fd);
}

/*
ufufs_create_arquivo
------------
Entrada: string indicando o diretório onde o arquivo deve ser criado, string indicando o nome do arquivo
Descrição: cria um arquivo vazio no diretório indicado por caminho
Saída: nada
*/
int ufufs_create_arquivo(char *caminho, char *nome)
{
	int caminho_fd;// file descriptor do diretório onde se quer criar o arquivo
	int retorno;
	
	if( strcmp(nome, "/") == 0)
	{
		printf("Nome inválido.\n");
		return 0;
	}
	
	if((caminho_fd = ufufs_open(caminho, WRITE_ONLY)) == -1)
	{
		printf("Caminho inexistente.\n");
		return 0;
	}
	
	if(ufufs_tipo(caminho_fd) != DIRETORIO)
	{
		printf("Caminhos terminando em arquivo são inválidos.\n");
		return 0;
	}

	// cria e preenche o inode
	inode novo;	
	
	novo.tipo = ARQUIVO;
	novo.inode_num = obter_inode_livre(div_fd,sb);
	mudar_horario(&(novo.criacao));
	novo.acesso = novo.criacao;
	novo.tamanho = 0;
	novo.bloco_inicial = novo.bloco_final = get_block_sequence(div_fd, 0, sb,1);
	
	if(novo.bloco_inicial == -1)
	{
		printf("Espaço insuficiente em disco para a criação do arquivo.\n");
		return 0;
	}
	
	retorno = write_inode(div_fd, sb.file_table_begin, novo.inode_num, &novo); // escreve o inode no disco
	if(retorno == 0)
	{
		printf("Quantidade máxima de arquivos em disco foi atingida.\n");
		return 0;
	}
	
	alterar_bitmap(div_fd, novo.inode_num,sb, 2); // altera bitmap de inodes
    alterar_faixa_bitmap(div_fd,novo.bloco_inicial, novo.bloco_final,sb); // marca o bitmap de dados
	
	// cria e preenche o dir_entry à ser armazenado nos blocos de dados do diretório
	dir_entry nova_entrada;
	nova_entrada.numero_inode = novo.inode_num;
	strcpy(nova_entrada.nome,nome);
	
	// move o offset pro final dos dados do diretório
		// desse jeito, exclusões deixarão buracos nos dados do diretório
	ufufs_seek(caminho_fd,0,SEEK_END_UFU);	
	retorno = ufufs_write(caminho_fd,&nova_entrada,sizeof(dir_entry)); // escreve a entrada nos dados do diretório
	if( retorno == -1 )
	{
		printf("Falha na linkagem do arquivo no diretório.\n");
		alterar_bitmap(div_fd, novo.inode_num,sb, 2);
		alterar_faixa_bitmap(div_fd,novo.bloco_inicial, novo.bloco_final,sb);
		//reverte as alterações no bitmap
		
		return 0;
	}
	
	ufufs_close(caminho_fd);
	
	printf("\nArquivo criado com sucesso!\n");
	return 1;
}

/*
ufufs_create_diretorio
------------
Entrada: string indicando o diretório onde o diretório deve ser criado, string indicando o nome do diretório
Descrição: cria um diretório no diretório indicando por caminho
Saída: nada
*/
void ufufs_create_directory(char *caminho, char *nome)
{
	int caminho_fd;// file descriptor do diretório onde se quer criar o diretório
	int atual_fd;
	int retorno;
	
	if( strcmp(nome, "/") == 0)
	{
		printf("Nome inválido.\n");
		return;
	}
	
	if((caminho_fd = ufufs_open(caminho, WRITE_ONLY)) == -1)
	{
		printf("Caminho inexistente.\n");
		return;
	}
	
	if(ufufs_tipo(caminho_fd) != DIRETORIO)
	{
		printf("Caminhos terminando em arquivo são inválidos.\n");
		return;
	}
	
	inode novo;	
	
	novo.tipo = DIRETORIO;
	novo.inode_num = obter_inode_livre(div_fd,sb);
	mudar_horario(&(novo.criacao));
	novo.acesso = novo.criacao;
	novo.tamanho = 0;
	novo.bloco_inicial = novo.bloco_final = get_block_sequence(div_fd, 0, sb,1);
	
	if(novo.bloco_inicial == -1)
	{
		printf("Espaço insuficiente em disco para a criação do arquivo.\n");
		return;
	}
	
	retorno = write_inode(div_fd, sb.file_table_begin, novo.inode_num, &novo); // escreve o inode no disco
	if(retorno == 0)
	{
		printf("Quantidade máxima de arquivos em disco foi atingida.\n");
		return;
	}
	
	alterar_bitmap(div_fd, novo.inode_num,sb, 2); // altera bitmap de inodes
    alterar_faixa_bitmap(div_fd,novo.bloco_inicial, novo.bloco_final,sb); // marca o bitmap de dados
	
	// cria e preenche o dir_entry à ser armazenado nos blocos de dados do diretório
	dir_entry nova_entrada;
	nova_entrada.numero_inode = novo.inode_num;
	strcpy(nova_entrada.nome,nome);
	
	// move o offset pro final dos dados do diretório
		// desse jeito, exclusões deixarão buracos nos dados do diretório
	ufufs_seek(caminho_fd,0,SEEK_END_UFU);	
	retorno = ufufs_write(caminho_fd,&nova_entrada,sizeof(dir_entry)); // escreve a entrada nos dados do diretório
	if( retorno == -1 )
	{
		printf("Falha na linkagem do arquivo no diretório.\n");
		alterar_bitmap(div_fd, novo.inode_num,sb, 2);
		alterar_faixa_bitmap(div_fd,novo.bloco_inicial, novo.bloco_final,sb);
		//reverte as alterações no bitmap
		
		return;
	}
	
	// cria, preenche e escreve no dados do novo diretório a entrada que indica o diretório anterior, além de armazenar o dir_entry do próprio diretório e do anterior
	dir_entry anterior;
	anterior.numero_inode = fd_table[caminho_fd]->inode_data.inode_num;
	strcpy(anterior.nome, "..");
		
	char novo_caminho[250];
	strcpy(novo_caminho,caminho);	
	if(novo_caminho[strlen(novo_caminho) -1] != '/')
		strcat(novo_caminho,"/");
		
	strcat(novo_caminho,nome);
	if( (atual_fd = ufufs_open(novo_caminho,WRITE_ONLY)) == -1)
	{
		printf("Falha na abertura do arquivo criado.\n");
		
		alterar_bitmap(div_fd, novo.inode_num,sb, 2);
		alterar_faixa_bitmap(div_fd,novo.bloco_inicial, novo.bloco_final,sb);
		//reverte as alterações no bitmap
		
		return;
	}
	
	strcpy(nova_entrada.nome,".");
	
	void *buffer = calloc(1,2 * sizeof(dir_entry));
	memcpy(buffer,&nova_entrada, sizeof(dir_entry));
	memcpy(buffer + sizeof(dir_entry), &anterior, sizeof(dir_entry));
	
	ufufs_write(atual_fd, buffer, sizeof(dir_entry) * 2);
	
	ufufs_close(caminho_fd);
	ufufs_close(atual_fd);
	free(buffer);
	
	printf("\nDiretório criado com sucesso!\n");

}

/*
ufufs_delete
------------
Entrada: string indicando o diretório onde o o arquivo ou diretório a ser excluída está localizado, string indicando o nome do arquivo ou diretório a ser excluído
Descrição: exclui o arquivo ou diretório (se estiver vazio) localizado no diretório indicado por caminho.
Saída: nada
*/
void ufufs_delete(char *caminho, char *nome)
{
	int excluir_fd;
	int diretorio_fd;
	int qtd_entradas, validas;
	char nome_aux[250];
	int i, h, j;
	dir_entry *buffer = (dir_entry *) calloc(1,sizeof(dir_entry));
	
	strcpy(nome_aux, caminho);
	if(nome_aux[strlen(nome_aux) -1] != '/')
		strcat(nome_aux,"/");
	strcat(nome_aux, nome);
	
	if(strcmp(nome_aux, "/.") == 0 || strcmp(nome_aux, "//") == 0)
	{
		printf("Impossível excluir o diretório raiz.\n");
		return;
	}

	if((excluir_fd = ufufs_open(nome_aux, READ_WRITE)) == -1)
	{
		printf("Arquivo ou diretório inexistente.\n");
		return;
	}
	
	if(ufufs_tipo(excluir_fd) == DIRETORIO)
	{
		// verifica se o diretório está vazio
		qtd_entradas =  ufufs_size(excluir_fd) / sizeof(dir_entry);
		validas = 0;
		for(i = 0; i < qtd_entradas; i++)
		{
			if(ufufs_read(excluir_fd,buffer, sizeof(dir_entry)) == -1)
				break;
				
			if(buffer->numero_inode == -1)
				continue;// entrada de um arquivo que foi excluído
			else
				validas++;
		}
		
		if(validas != 2)
		{
			printf("Apenas diretórios vazios podem ser excluídos\n");
			return;	
		}
	}	

	if((diretorio_fd = ufufs_open(caminho, READ_WRITE)) == -1)
	{
		printf("Falha na abertura do diretório que o arquivo está contido.\n");
		return;
	}
	
	inode *in = (inode *) calloc(1,sizeof(inode));	
	write_inode(div_fd, sb.file_table_begin, fd_table[excluir_fd]->inode_data.inode_num, in);
	free(in);
	
	alterar_bitmap(div_fd,fd_table[excluir_fd]->inode_data.inode_num,sb,2); // marca no bitmap o inode correspondente como desocupado
	alterar_faixa_bitmap(div_fd, fd_table[excluir_fd]->inode_data.bloco_inicial, fd_table[excluir_fd]->inode_data.bloco_final, sb); // marca no bitmap a desocupação dos blocos de dados
	
	void *bc = (unsigned char *) calloc(1,BLOCK_SIZE);
	
	qtd_entradas = ufufs_size(diretorio_fd) / sizeof(dir_entry);

	i = 0;
	for(h = fd_table[diretorio_fd]->inode_data.bloco_inicial; h <= fd_table[diretorio_fd]->inode_data.bloco_final; h++)
	{
		if(ler_bloco(div_fd,sb.data_table_begin + h,bc) == 0)
			return;
			
		for(j = 0; j < BLOCK_SIZE / sizeof(dir_entry); j++)
		{
			i++;
			
			if(i > qtd_entradas)
			{
				break;
			}
		
			memcpy(buffer, bc + j * sizeof(dir_entry), sizeof(dir_entry));
			if(buffer->numero_inode == fd_table[excluir_fd]->inode_data.inode_num)
			{
				buffer->numero_inode = -1; // marca como inválido
				memcpy(bc + j * sizeof(dir_entry), buffer, sizeof(dir_entry));
				
				escrever_bloco(div_fd, sb.data_table_begin + h, bc);
				i = -1;
				
				break;
			}
		}	
		
		if(i == -1)
			break;
	}
	
	free(buffer);
	free(bc);
	
	ufufs_close(diretorio_fd);
	ufufs_close(excluir_fd);
	
	// não ocorre atualização de horário no diretório

	printf("Arquivo ou diretório excluído com sucesso.\n");
}

/*
copy_ufufs_to_real
------------
Entrada: string indicando o caminho e nome do arquivo local, string indicando caminho e nome do arquivo no sistema linux
Descrição: copia o conteúdo do arquivo indicando por aqui para um arquivo no sistema linux indicado por real.
Saída: nada
*/
void copy_ufufs_to_real(char *aqui, char *real)
{
	// como o arquivo criado no sistema real herda as permissões do processo que o cria é necessário permissão de administrador pra ler seu conteúdo

	int real_fd;
	int aqui_fd;
	int tamanho;
	void *buffer = calloc(1,BLOCK_SIZE);

	if((aqui_fd = ufufs_open(aqui, READ_ONLY)) == -1)
	{
		printf("\nArquivo %s não encontrado no ufufs.\n",aqui);
		return;
	}
	
	if( (real_fd = open(real, O_WRONLY | O_CREAT)) == -1 )
	{
		printf("\nArquivo %s não pode ser criado.\n",real);
		return;
	}
	
	int ler;
	tamanho = ufufs_size(aqui_fd);
	while(tamanho > 0)
	{
		if(tamanho < BLOCK_SIZE)
			ler = tamanho;
		else
			ler = BLOCK_SIZE;
	
		if( ufufs_read(aqui_fd, buffer, ler) == -1)
		{
			printf("\nFalha durante a operação de cópia (leitura).\n");
			return;
		}
		
		if( write(real_fd, buffer, ler) == -1)
		{
			printf("\nFalha durante a operação de cópia (escrita).\n");
			return;
		}
		
		tamanho -= ler;
	}

	printf("\nCópia concluída.\n");

	ufufs_close(aqui_fd);
	close(real_fd);
	free(buffer);
}

/*
copy_real_to_ufufs
------------
Entrada: string indicando o caminho e nome do arquivo no sistema linux, string indicando o caminho no ufufs, string indicando o nome no ufufs
Descrição: copia o conteúdo do arquivo indicando por real para um arquivo no sistema linux indicado por caminho/nome
Saída: nada
*/
void copy_real_to_ufufs(char *real, char *caminho, char *nome)
{
	int real_fd;
	int aqui_fd;
	int tamanho;
	char nome_aux[250];
	void *buffer = calloc(1,BLOCK_SIZE);
	
	strcpy(nome_aux, caminho);
	if(nome_aux[strlen(nome_aux) -1] != '/')
		strcat(nome_aux,"/");
	strcat(nome_aux, nome);
	
	if( (real_fd = open(real,O_RDONLY)) == -1 )
	{
		printf("\nArquivo %s não encontrado.\n",real);
		return;
	}
	
	if( ufufs_create_arquivo(caminho,nome) == 0)
		return;
     
    
	if( (aqui_fd = ufufs_open(nome_aux, WRITE_ONLY)) == -1)
	{
		printf("\nArquivo %s/%s não encontrado.\n",caminho,nome);
		return;
	}
	
	int ler;
	tamanho = lseek(real_fd, 0, SEEK_END);
	lseek(real_fd, 0, SEEK_SET);
	while(tamanho > 0)
	{
		if(tamanho < BLOCK_SIZE)
			ler = tamanho;
		else
			ler = BLOCK_SIZE;
	
		if(read(real_fd, buffer, ler) == -1)
		{
			printf("\nFalha durante a operação de cópia (leitura).\n");
			return;
		}
		
		if( ufufs_write(aqui_fd, buffer, ler) == -1)
		{
			printf("\nFalha durante a operação de cópia (escrita).\n");
			return;
		}
		
		tamanho -= BLOCK_SIZE;
	}
	
	printf("Cópia concluída.\n");
	
	ufufs_close(aqui_fd);
	close(real_fd);
	free(buffer);	
}

/*
ufufs_read_shell
------------
Entrada: string indicando o arquivo a ser lido
Descrição: lê do arquivo passado
Saída: nada
*/
void ufufs_read_shell(char *caminho_nome)
{
	int arquivo_fd;
	int i;
	unsigned int posicao, qtd;
	unsigned char *leitura;
	
	if((arquivo_fd = ufufs_open(caminho_nome, READ_ONLY)) == -1)
	{
		printf("Arquivo inexistente.\n");
		return;
	}
	
	if( ufufs_tipo(arquivo_fd) == DIRETORIO)
	{
		printf("Não é possível ler de um diretório.\n");
		return;
	}
	
	printf("Entre com a posição onde a leitura deve iniciar:\n");
	scanf("%u",&posicao);
	
	if( ufufs_seek(arquivo_fd,posicao,SEEK_SET_UFU) == -1)
	{
		printf("Falha durante o reposicionamento.\n");
		ufufs_close(arquivo_fd);
		return;
	}
	
	printf("Entre com a quantidade de bytes que deseja ler ( 0 para o restante do arquivo ):\n");
	scanf("%u",&qtd);
			
	
	if( qtd == 0)
		qtd = ufufs_size(arquivo_fd) - ufufs_offset(arquivo_fd);
				
	leitura = (unsigned char *) malloc(sizeof(unsigned char) * qtd);
				
	if( (qtd = ufufs_read(arquivo_fd,leitura,qtd)) == -1)
	{
		printf("Falha durante a leitura.\n");
		ufufs_close(arquivo_fd);
		return;	
	}
				
	if(qtd == 0)
	{
		printf("Nada lido\n");
		ufufs_close(arquivo_fd);
		return;
	}
				
	for(i = 0; i < qtd; i++)
	{
		if( i % 100 == 0)
		{
			printf("\n%d ", i / 100);
		}
	
		printf("%c", (char) leitura[i]);
	}
	printf("\n\n");
	
	free(leitura);			
	ufufs_close(arquivo_fd);			
}

/*
ufufs_write_shell
------------
Entrada: string indicando o arquivo a ser escrita
Descrição: escreve no arquivo passado
Saída: nada
*/
void ufufs_write_shell(char *caminho_nome)
{
	int arquivo_fd;
	int i;
	unsigned int posicao, qtd;
	unsigned char *escrita;
	
	if((arquivo_fd = ufufs_open(caminho_nome, WRITE_ONLY)) == -1)
	{
		printf("Arquivo inexistente.\n");
		return;
	}
	
	if( ufufs_tipo(arquivo_fd) == DIRETORIO)
	{
		printf("Não é possível escrever em um diretório.\n");
		return;
	}
	
	printf("Entre com a posição onde a escrita deve iniciar:\n");
	scanf("%u",&posicao);
	
	if( ufufs_seek(arquivo_fd,posicao,SEEK_SET_UFU) == -1)
	{
		printf("Falha durante o reposicionamento.\n");
		ufufs_close(arquivo_fd);
		return;
	}
	
	printf("Entre com a quantidade de bytes que deseja escrever:\n");
	scanf("%u",&qtd);
			
	if(qtd <= 0)
	{
		printf("Quantidade inválida.\n");
		ufufs_close(arquivo_fd);
		return;
	}
				
	escrita = (unsigned char *) malloc(sizeof(unsigned char) * qtd);
	
	printf("Entre com o conteúdo a ser escrito: \n");
	for(i = 0; i < qtd; i++)
	{
		scanf(" %c", escrita + i);
	}
	
	if( (qtd = ufufs_write(arquivo_fd,escrita,qtd)) == -1)
	{
		printf("Falha durante a escrita\n");
		ufufs_close(arquivo_fd);
		return;
	}
						
	if(qtd == 0)
	{
		printf("Nada escrito\n");
		ufufs_close(arquivo_fd);
		return;
	}
				
	printf("\nEscrita concluída com sucesso.\n");
	
	free(escrita);			
	ufufs_close(arquivo_fd);
}
