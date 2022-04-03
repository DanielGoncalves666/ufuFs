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

void ufuFS_help();
void ufuFS_shell();
void ufuFS_list(char *caminho);
void ufufs_create_arquivo(char *caminho, char *nome);
void ufufs_create_directory(char *caminho, char *nome);
void ufufs_delete(char *caminho_nome);
void copy_real_to_ufufs(char *real, char *caminho, char *nome);
void copy_ufufs_to_real(char *aqui, char *real);

int main()
{
	ufuFS_shell();

	return 0;
}

void ufuFS_help()
{
    printf("Lista de Comandos:\n");
    printf("Obs: Espaços separam o comando de seus argumentos\n");
    printf("\tcreate_arq <caminho> <nome>\n"); // caminho precisa terminar em um diretório
    printf("\tcreate_dir <caminho> <nome>\n"); // caminho precisa terminar em um diretório
    printf("\tdelete <caminho/nome>\n"); // se for um diretório precisa estar vazio
    printf("\tlist <caminho>\n"); // se for um arquivo lista somente as informações dele 
    printf("\tcopy_ufufs_to_real <ufu_arquivo> <real_arquivo>\n"); 
    printf("\tcopy_real_to_ufufs <real_arquivo> <caminho> <nome>\n");// caminho precisa terminar em um diretório
    printf("\tclear\n"); // limpa a tela
    printf("\thelp\n"); // mostra esta lista
    printf("\texit\n"); // fecha o programa
}
/*
void limparBuffer()
{
	char tipo;
	while ((tipo = getchar()) != '\n' && tipo != EOF);				
}
*/

void ufuFS_shell()
{
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
	  		printf("\nDispositivo desconectado.\n");
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
  			if(arg1 == NULL)
  			{
  				printf("\nArgumentos insuficientes.\n");
  				continue;
  			}
  			
  			ufufs_delete(arg1);
  		}
  		else if(strcmp(comando,"list") == 0)
  		{
  			arg1 = strtok (NULL, " ");
  			if(arg1 == NULL)
  			{
  				printf("\nArgumentos insuficientes.\n");
  				continue;
  			}
  			
  			ufuFS_list(arg1);
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
  		else if(strcmp(comando,"clear") == 0)
  		{
  			printf("\e[1;1H\e[2J");
  		}
  		else if(strcmp(comando,"help") == 0)
  		{
  			ufuFS_help();
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


void ufuFS_list(char *caminho)
{
	inode dir_atual, entrada;
	int qtd_entradas;
	int h, retorno;
	dir_entry *buffer = (dir_entry *) calloc(1,sizeof(dir_entry));

	read_inode(div_fd,sb.file_table_begin,0,&dir_atual); // carrega o inode do diretório raiz;
	qtd_entradas = dir_atual.tamanho / sizeof(dir_entry); // calcula a quantidade de entradas válidas no diretório raiz
	
	retorno = ufufs_open("/", READ_ONLY);
	if(retorno == -1)
	{
		printf("\nErro na abertura.");
		return;
	}

	for(h = 0; h < qtd_entradas;)
	{
		if(ufufs_read(retorno,buffer, sizeof(dir_entry)) == -1)
			break;
				
		if(buffer->numero_inode == -1)
			continue;
			
		printf("%s\t", buffer->nome);
	
		read_inode(div_fd,sb.file_table_begin,buffer->numero_inode,&entrada); // carrega o inode de cada entrada do diretório
		printf("%c\t",entrada.tipo == 1? 'A' : 'D');
		printf("Criação: %02d:%02d:%02d %02d/%02d/%d\t", entrada.criacao.hora, entrada.criacao.minuto, entrada.criacao.segundo, entrada.criacao.dia, entrada.criacao.mes, entrada.criacao.ano);
		printf("Acesso: %02d:%02d:%02d %02d/%02d/%d\t", entrada.acesso.hora, entrada.acesso.minuto, entrada.acesso.segundo, entrada.acesso.dia, entrada.acesso.mes, entrada.acesso.ano);
		printf("Tamanho: %5u bytes\n",entrada.tamanho);
			
		h++;
	}
	
	free(buffer);
	ufufs_close(retorno);
}


// create_file <caminho> <nome>       cria um arquivo vazio, caminho precisa terminar em um diretório
void ufufs_create_arquivo(char *caminho, char *nome)
{
	int caminho_fd;
	inode novo;
	
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
	
	// cria novo inode
	novo.tipo = ARQUIVO;
	novo.inode_num = obter_inode_livre(div_fd,sb);
	mudar_horario(&(novo.criacao));
	mudar_horario(&(novo.acesso));
	novo.tamanho = 0;
	novo.bloco_inicial = novo.bloco_final = get_block_sequence(div_fd, 0, sb,1);
	
	// salva e marca bitmap de inodes
	write_inode(div_fd, sb.file_table_begin, novo.inode_num, &novo); // escreve o inode no disco
	alterar_bitmap(div_fd, novo.inode_num,sb, 2); // altera bitmap de inodes
	
    alterar_faixa_bitmap(div_fd,novo.bloco_inicial, novo.bloco_final,sb); // marca o bitmap de dados
	
	dir_entry de;
	de.numero_inode = novo.inode_num;
	strcpy(de.nome,nome);
	
	ufufs_seek(caminho_fd,0,SEEK_END);
	ufufs_write(caminho_fd,&de,sizeof(dir_entry)); // escreve a entrada de diretório
	
	ufufs_close(caminho_fd);
	
	printf("\nArquivo criado com sucesso!\n");
}

void ufufs_create_directory(char *caminho, char *nome)
{


}

void ufufs_delete(char *caminho_nome)
{
	/*
	excluir arquivos e diretórios
	
	localiza o arquivo a ser excluido
	desmarca o bitmap de dados
	desmarca o bitmap do inode
	zera o inode
	remove a entrada do diretório
*/

}

void copy_ufufs_to_real(char *aqui, char *real)
{
	int real_fd;
	int aqui_fd;
	int tamanho;
	void *buffer = calloc(1,BLOCK_SIZE);

	if((aqui_fd = ufufs_open("/", READ_ONLY)) == -1)
	{
		printf("\nArquivo %s não encontrado.\n",aqui);
		return;
	}
	
	if( (real_fd = open(real,O_WRONLY)) == -1 )
	{
		printf("\nArquivo %s não encontrado.\n",real);
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


void copy_real_to_ufufs(char *real, char *caminho, char *nome)
{
	int real_fd;
	int aqui_fd;
	int tamanho;
	void *buffer = calloc(1,BLOCK_SIZE);
	
	if( (real_fd = open(real,O_WRONLY)) == -1 )
	{
		printf("\nArquivo %s não encontrado.\n",real);
		return;
	}
	
	// cria o arquivo e então abre ele
	if( (aqui_fd = ufufs_open(caminho, WRITE_ONLY)) == -1)
	{
		printf("\nArquivo %s%s não encontrado.\n",caminho,nome);
		return;
	}
	
	tamanho = lseek(real_fd, 0, SEEK_END);
	while(tamanho > 0)
	{
		if(read(real_fd, buffer, BLOCK_SIZE) == -1)
		{
			printf("\nFalha durante a operação de cópia (leitura).\n");
			return;
		}
		
		if( ufufs_write(aqui_fd, buffer, BLOCK_SIZE) == -1)
		{
			printf("\nFalha durante a operação de cópia (escrita).\n");
			return;
		}
		
		tamanho -= BLOCK_SIZE;
	}
	
	ufufs_close(aqui_fd);
	close(real_fd);
	free(buffer);	
}



