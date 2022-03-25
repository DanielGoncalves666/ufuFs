#ifndef UFUFS_H
#define UFUFS_H

#define MAXIMUM_OPEN_FILES 50
#define SEEK_SET_UFU 1
#define SEEK_CUR_UFU 2
#define SEEK_END_UFU 3

#define APPEND 1 // append no final do arquivo
#define OVERWRITTEN 2 // reescreve desde o começo
#define APPEND_AT 3 // append a partir do offset

int ufufs_mount(char *dispositivo);
int ufufs_open(char *pathname, int flags);
int ufufs_read(int fd, void *destino, int qtd);
int ufufs_write(int fd, void *buffer, unsigned int qtd);
int ufufs_seek(int fd, unsigned int offset, int flags);
int ufufs_close(int fd);


#endif
