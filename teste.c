#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include"estruturas.h"

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
	memcpy(&sb.magic_number,empty_block + offset,sizeof(int));
	offset += sizeof(int);
	memcpy(&sb.block_number,empty_block + offset,sizeof(unsigned int));
	offset += sizeof(unsigned int);
	memcpy(&sb.block_size,empty_block + offset,sizeof(unsigned int));
	offset += sizeof(unsigned int);
	memcpy(&sb.inode_table_length,empty_block + offset,sizeof(unsigned int));
	offset += sizeof(unsigned int);
	memcpy(&sb.inode_bitmap_begin,empty_block + offset,sizeof(unsigned int));
	offset += sizeof(unsigned int);
	memcpy(&sb.data_bitmap_begin,empty_block + offset,sizeof(unsigned int));
	offset += sizeof(unsigned int);
	memcpy(&sb.file_table_begin,empty_block + offset,sizeof(unsigned int));
	offset += sizeof(unsigned int);
	memcpy(&sb.data_table_begin,empty_block + offset,sizeof(unsigned int));
	
	printf("%d\n",sb.magic_number);
	printf("%u\n",sb.block_number);
	printf("%u\n",sb.block_size);
	printf("%u\n",sb.inode_table_length);	
	printf("%u\n",sb.inode_bitmap_begin);
	printf("%u\n",sb.data_bitmap_begin);
	printf("%u\n",sb.file_table_begin);
	printf("%u\n",sb.data_table_begin);
		


	return 0;

}

