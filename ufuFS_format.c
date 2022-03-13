#ifndef UFUFS_FORMAT_H
#define UFUFS_FORMAT_H

/*
Módulo de formatação

*/


// abrir o device file e obter o tamanho do disco.
// calcular a quantidade de blocos
// baseando-se na quantidade de blocos, determinar os ranges para o bitmap de dados, bitmap de inodes. Tudo isso considerando que 10% do disco é dedicado à inodes.
// criar e preenchar uma estrutura de superbloco.

// escrever o superbloco no disco e zerar os blocos referentes ao bitmap de dados, bitmap de inodes e a tabela de inodes.
// criar o inode e fazer a alteração nos bitmaps para o diretório raiz. (todo diretório tem uma entrada . , incluindo o raiz)
	// n daria pra usar a operações de criaçao de arquivo pois é o diretório raiz e o nome dele n existe.
// podemos colocar a opção de um hard format ou format rápido. No caso do hard format talvez seja bom criar uma matriz do tamanho de um bloco com um caractere 
 	// inválido e então preencher todo o disco com esse bloco.













#endif
