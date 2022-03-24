#ifndef BITMAP_H
#define BITMAP_H

int obter_inode_livre(int fd, superblock sb);
int localizar_bit(unsigned char valor, char bit, int sentido);
int alterar_bitmap(int fd, int number, superblock sb, int tipo);
unsigned char inverter_bit(unsigned char valor, int pos);
int alterar_faixa_bitmap(int fd, int inicio, int fim, superblock sb);
int get_bitmap_pos_status(int fd, int number, superblock sb, int tipo);
int get_block_sequence(int fd, int comeco, superblock sb, int qtd);

#endif
