#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include"ufuFS.h"

#define INODE_RATIO 0.05

extern int inodes_in_a_block;
extern int inode_number;
extern superblock sb;
int div_fd;

int main()
{
	if(ufufs_mount("/dev/sdb") == -1)
		printf("error\n");

	/*
	inodes_in_a_block =  BLOCK_SIZE / sizeof(inode);	 
	inode_number = (size * INODE_RATIO) * inodes_in_a_block;
	  
	sb.magic_number = UFUFS_MAGIC_NUMBER;
	sb.block_number = size;
	sb.block_size = BLOCK_SIZE;
	sb.inode_table_length = size * INODE_RATIO; // 5% de todos os blocos
	sb.inode_bitmap_begin = 1;
    sb.data_bitmap_begin = sb.inode_bitmap_begin + (int) ceil(inode_number / (BLOCK_SIZE * 8));  

	int aux = size - (sb.data_bitmap_begin + sb.inode_table_length);
    int qtd_data_bitmap = ceil(aux/(BLOCK_SIZE * 8));
    
	sb.file_table_begin = sb.data_bitmap_begin + qtd_data_bitmap; 
	sb.data_table_begin = sb.file_table_begin + sb.inode_table_length;
	*/

	// ------------------------------------------------------------------------ // 
	
	int fd = ufufs_open("/", READ_WRITE);
	if( fd == -1)
	{
		printf("erro na abertura\n");
		return 0;
	}
	
	void *b;
	unsigned char *c;
	unsigned char *vet;
	unsigned int op, valor;
	int i;
	
	alterar_faixa_bitmap(div_fd, 1,1,sb);
	printf("1 - %d\n",get_bitmap_pos_status(div_fd, 1, sb, 1));	
	while(1)
	{
		printf("\n Opções:\n 1 - Ler\n 2 - Mover offset (atual = %d)\n 3 - Escrever\n 4 - Sair\n",ufufs_offset(fd));
		scanf("%d",&op);
		
		switch(op)
		{
			case 1:
				printf("Entre com a quantidade de bytes a serem lidos:\n");
				scanf("%u",&valor);
				
				c = (unsigned char *) malloc(sizeof(unsigned char) * valor);
				
				if( (valor = ufufs_read(fd,c,valor)) == -1)
				{
					printf("Falha durante a leitura.\n");
					break;
				}
				
				if(valor == 0)
				{
					printf("Nada lido\n");
					break;
				}
				
				for(i = 0; i < valor; i++)
				{
					if( i % 128 == 0)
					{
						printf("\n%d ", i / 128);
					}
	
					printf("%c", (char) c[i]);
				}
				printf("\n");
				
				free(c);
			
				break;
			case 2:
				printf("Entre com a nova posição do offset:\n");
				scanf("%u",&valor);
				
				if( ufufs_seek(fd,valor,SEEK_SET_UFU) == -1)
				{
					printf("Falha durante o seek.\n");
				}
				
				break;
			case 3:
				printf("Entre com a quantidade de superbytes (1 superbyte = 512 bytes iguais):\n");
				scanf("%u",&valor);
				
				vet = (unsigned char *) malloc(sizeof(unsigned char) * valor);
				
				for(i = 0; i < valor; i++)
				{
					scanf(" %c", vet + i);
				}
				
				b = malloc(sizeof(unsigned char) * valor * 512);
				
				for(i = 0; i < valor; i++)
				{
					memset(b + (512 * i), (int) vet[i], 512);
				}
				
				if( ufufs_write(fd,b,512 * valor) == -1)
				{
					printf("Falha durante a escrita\n");
				}
				
				free(b);
				
				break;
			case 4:
				printf("Saindo\n");
				break;
			default:
				printf("Opção inválida.\n");
				break;
		}
		
		if(op == 4)
			break;
	}	
	
	ufufs_close(fd);
	close(div_fd);
	
	return 0;
}


