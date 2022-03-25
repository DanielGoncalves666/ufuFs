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
    printf("\tcopy_to_usb <caminho>\n");
    printf("\tcopy_to_so <arquivo>\n");
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

	ufuFS_list();
    
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

// {
   // int i;
   // inode i_node;

   // lseek(fd, sb.ind_bloco * sb.tamanho, SEEK_SET);

   // for(i = 0; i < sb.qtd_inode; i++)
   // {
       // read(fd, &i_node, sizeof(inode));

       // if(!i_node.nome[0]) continue;                    //pegar o nome

       	// printf("%-15s  %10d bytes Criacao: %s ",
			// i_node.nome, i_node.tamanho,                    //pegar o nome
			// ctime((time_t *)&i_node.criacao)
           // );

           // printf("Ultimo acesso: %s",ctime((time_t *)&i_node.acesso));
   // }
// }




/*
	Funcionalidades necessárias:
		criar arquivos e diretórios
		excluir arquivos e diretórios
		listar os arquivos armazenados (lista em ordem alfabetica, usar uma arvore AVL)
		copiar arquivo do ufuFS para o sistema base e vice-versa



	Pode ser útil saber como pegar o path em que o processo está trabalhando
*/

/*
	criar arquivos
	
	vai até o diretório onde será criado
	aloca um inode e o preenche
	coloca a entrada no diretório do arquivo
	marca o bitmap de inodes
*/

/*
	excluir arquivos e diretórios
	
	localiza o arquivo a ser excluido
	desmarca o bitmap de dados
	desmarca o bitmap do inode
	zera o inode
	remove a entrada do diretório
*/



/*
	copiar arquivo do ufuFS para o sistema base
	
	obter caminho para onde o arquivo será copiado
	obter caminho do arquivo a ser copiado
	criar arquivo no destino
	abrir arquivo a ser copiado
	ler bloco por bloco do arquivo a ser copiado e escrever bloco por bloco no arquivo de destino
	fechar ambos arquivos

*/


/*
	copiar arquivo do sistema base para ufuFS
	
	obter caminho do arquivo a ser copiado
	obter caminho para onde o arquivo será copiado
	criar arquivo no destino
	abrir arquivo a ser copiado
	ler uma quantidade de BLOCK_SIZE e escrever no arquivo destino até que tudo tenha sido copiado
	fechar ambos arquivos.	
	

*/
