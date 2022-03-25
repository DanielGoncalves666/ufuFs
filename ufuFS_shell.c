#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"estruturas.h"
#include"ufuFS.h"
#include"bloco.h"
#include"bitmap.h"

extern superblock sb;
extern int div_fd;

void ufuFS_shell();
void ufuFS_help();
void ufuFS_list();

int main()
{
	ufuFS_shell();


	return 0;
}

void ufuFS_help()
{
    printf("Lista de Comandos:\n");
    printf("\tcreate <arquivo>\n");
    printf("\tdelete <arquivo>\n");
    printf("\tlist\n");
    printf("\tcopy_ufufs_to_real <ufu_arquivo> <real_arquivo>\n");
    printf("\tcopy_real_to_ufufs <real_arquivo> <ufu_arquivo>\n");
    printf("\texit\n");
}

void ufuFS_shell()
{
	char nome_dispositivo[15];

    int end = 0;
    char rqt[256], *fn;
    int check;

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
	  	}
	}	
  	
    printf("\n");
    ufuFS_help();
    printf("\n");

	//ufuFS_list();
    
}


void ufuFS_list()
{
	inode dir_atual, entrada;
	int qtd_entradas;
	int i, h, retorno;
	dir_entry *buffer = (dir_entry *) calloc(1,sizeof(dir_entry));

	read_inode(div_fd,sb.file_table_begin,0,&dir_atual); // carrega o inode do diretório raiz;
	qtd_entradas = dir_atual.tamanho / sizeof(dir_entry); 
	
	retorno = ufufs_open("/", OVERWRITTEN);
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
	
	if((caminho_fd = ufufs_open(aqui, OVERWRITTEN)) == -1)
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

	if((aqui_fd = ufufs_open(aqui, OVERWRITTEN)) == -1)
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
	if( /*(aqui_fd = ufufs_open(aqui, OVERWRITTEN)) == -1*/)
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



