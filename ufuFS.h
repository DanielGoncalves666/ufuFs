#ifndef UFUFS_H
#define UFUFS_H

#include"estruturas.h"
#include"bloco.h"
#include"bitmap.h"

#define MAXIMUM_OPEN_FILES 50
#define SEEK_SET_UFU 1
#define SEEK_CUR_UFU 2
#define SEEK_END_UFU 3

#define READ_ONLY 1
#define WRITE_ONLY 2
#define READ_WRITE 3

int ufufs_mount(char *dispositivo);
void ufufs_unmount();
int ufufs_open(char *pathname, int flags);
int ufufs_read(int fd, void *destino, int qtd);
int ufufs_write(int fd, void *buffer, unsigned int qtd);
int ufufs_seek(int fd, unsigned int offset, int flags);
int ufufs_close(int fd);
int ufufs_size(int fd);
short int ufufs_tipo(int fd);
int ufufs_offset(int fd);
void mudar_horario(struct dataTime *mudar);

#endif
