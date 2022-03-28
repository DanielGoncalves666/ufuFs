#include<stdio.h>
#include<math.h>
#include"ufuFS.h"

#define INODE_RATIO 0.05

extern int inodes_in_a_block;
extern int inode_number;
int div_fd;

int main()
{
	int size;

	size = abrir_dispositivo("/dev/sdb", &div_fd); // size tem a quantidade de blocos no dispositivo
	if(size == -1)
		printf("error\n");


	superblock sb;

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

	// ------------------------------------------------------------------------ // 
	
	alterar_faixa_bitmap(div_fd,10,11,sb);
	
	for(int i = 0; i < 10; i++)
		printf("            %d\n",get_bitmap_pos_status(div_fd,i, sb, 1));
		
	printf("%d\n",get_block_sequence(div_fd, 0,sb,10));
}
