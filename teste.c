#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include"estruturas.h"
#include"bitmap.h"
#include"bloco.h"

int f;


int main(int argc, char *argv[])
{
	int size,div_fd;
	char pathname[50];
	void *empty_block = calloc(1,BLOCK_SIZE);

	scanf("%s",pathname);

	size = abrir_dispositivo(pathname, &div_fd); // size tem a quantidade de blocos no dispositivo
	
	if(size == -1)
	{
		printf("size -1");
		return 0;
	}
	
	ler_bloco(div_fd,0,empty_block);
	
	superblock sb;
	
	int offset = 0;
	memcpy(&sb,empty_block,sizeof(superblock));
	
	
	printf("%d\n",sb.magic_number);
	printf("%u\n",sb.block_number);
	printf("%u\n",sb.block_size);
	printf("%u\n",sb.inode_table_length);	
	printf("%u\n",sb.inode_bitmap_begin);
	printf("%u\n",sb.data_bitmap_begin);
	printf("%u\n",sb.file_table_begin);
	printf("%u\n",sb.data_table_begin);
		
		
	void *in = calloc(1,BLOCK_SIZE);	
	dir_entry itself;	
		
	ler_bloco(div_fd,sb.data_table_begin,in);
		
	offset = 0;
	memcpy(&itself,in + offset,sizeof(dir_entry));
	
	
	printf("\n%d\n",itself.numero_inode);
	printf("%s\n",itself.nome);
	
	close(div_fd);
	
	return 0;

}

