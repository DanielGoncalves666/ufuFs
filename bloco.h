#ifndef BLOCO_H
#define BLOCO_H

long int abrir_dispositivo(const char *pathname, int *fd);
int ler_bloco(int fd, unsigned int num_bloco, void *bloco);
int escrever_bloco(int fd, unsigned int num_bloco, void *bloco);
int read_inode(int fd, unsigned int file_table_begin, unsigned int num_inode, void *buff);
int write_inode(int fd,  unsigned int file_table_begin, unsigned int num_inode, void *buff);

#endif
