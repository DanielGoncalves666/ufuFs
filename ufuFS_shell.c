#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<fcntl.h>
#include<unistd.h>
#include"estruturas.h"
#include"ufuFS.h"
#include"bloco.h"
#include"bitmap.h"

extern superblock sb;
extern int div_fd;

void ufuFS_shell();
void ufuFS_help();
void ufuFS_list();
void copy_real_to_ufufs(char *real, char *aqui);
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

void ufuFS_shell()
{
	char nome_dispositivo[15], linha_comando[256];
	int check;
	char *comando, *arg1, *arg2, *arg3;

	printf("\e[1;1H\e[2J");
    printf("Bem vindo ao Shell de Navegacao ufuFS\n\n");
	
	while(1)
	{
	  	printf("Insira o nome (ex: /dev/sdXX) do dispositivo que contenha um ufuFS para montá-lo.\n");
	  	scanf("%s",nome_dispositivo);
	  	
	  	if( ufufs_mount(nome_dispositivo) == 0)
	  	{
	  		printf("Dispositivo montado com sucesso!\n");
	  		break;
	  	}
	  	else
	  	{
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
  	
  	while(1)
  	{
  		printf("\n Shell> ");
  		check = scanf("%[^\n]", linha_comando);
  		
  		comando = strtok (linha_comando, " ");
  		
  	
  		if(strcmp(comando,"create_arq") == 0)
  		{
  		
  		}
  		else if(strcmp(comando,"create_dir") == 0)
  		{
  		
  		}
  		else if(strcmp(comando,"delete") == 0)
  		{
  		
  		}
  		else if(strcmp(comando,"list") == 0)
  		{
  		
  		}
  		else if(strcmp(comando,"copy_ufufs_to_real") == 0)
  		{
  		
  		}
  		else if(strcmp(comando,"copy_real_to_ufufs") == 0)
  		{
  		
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


void ufuFS_list()
{
	inode dir_atual, entrada;
	int qtd_entradas;
	int i, h, retorno;
	dir_entry *buffer = (dir_entry *) calloc(1,sizeof(dir_entry));

	read_inode(div_fd,sb.file_table_begin,0,&dir_atual); // carrega o inode do diretório raiz;
	qtd_entradas = dir_atual.tamanho / sizeof(dir_entry); 
	
	retorno = ufufs_open("/", READ_ONLY);
	if(retorno == -1)
		printf("erro na abertura");
		//break;
		
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
	
	ufufs_close(retorno);
}


// create_file <caminho> <nome>       cria um arquivo vazio, caminho precisa terminar em um diretório
void ufufs_create_arquivo(char *caminho, char *nome)
{
	int caminho_fd;
	
	if((caminho_fd = ufufs_open(caminho, WRITE_ONLY)) == -1)
	{
		printf("Caminho inexistente.\n");
		
	}

	/*
	criar arquivos
	
	vai até o diretório onde será criado
	aloca um inode e o preenche
	coloca a entrada no diretório do arquivo
	marca o bitmap de inodes
*/

}

void ufufs_create_directory()
{


}

void ufufs_delete_arquivo()
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

void ufufs_delete_directory()
{


}

void copy_ufufs_to_real(char *aqui, char *real)
{
	int real_fd;
	int aqui_fd;
	int tamanho;
	void *buffer = calloc(1,BLOCK_SIZE);

	if((aqui_fd = ufufs_open(aqui, READ_ONLY)) == -1)
	{
		printf("\nArquivo %s não encontrado.\n",aqui);
		return;
	}
	
	if( (real_fd = open(real,O_WRONLY)) == -1 )
	{
		printf("\nArquivo %s não encontrado.\n",real);
		return;
	}
	
	tamanho = ufufs_size(aqui_fd);
	while(tamanho > 0)
	{
		if( ufufs_read(aqui_fd, buffer, BLOCK_SIZE) == -1)
		{
			printf("\nFalha durante a operação de cópia (leitura).\n");
			return;
		}
		
		if( write(real_fd, buffer, BLOCK_SIZE) == -1)
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


void copy_real_to_ufufs(char *real, char*aqui)
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
	if( (aqui_fd = ufufs_open(aqui, WRITE_ONLY)) == -1)
	{
		printf("\nArquivo %s não encontrado.\n",aqui);
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



