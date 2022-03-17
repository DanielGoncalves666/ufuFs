#ifndef UFUFS_H
#define UFUFS_H

/*
Este módulo contém funções relacionadas com a operação do sistema de arquivos em si. Como abertura do device driver e manipulação das estruturas de metadados.

*/

#include<math.h>


/*
alterar_bitmap
---------------
Entrada: inteiro, indicando a posição (em bits) a ser alterada, estrutura de superbloco para se obter informações de início do bitmap; inteiro, 1 para DATA, 2 para INODE
Descrição: realiza a alteração da posição indicada
Saída: 0, em fracasso, 1, em sucesso
*/
int alterar_bitmap(int number, superblock sb, int tipo)
{
	bitmap bm;

	// verificar se sb é válido
	int bloco = tipo == 1? sb.data_bitmap_begin : sb.inode_bitmap_begin;

	bloco = bloco + number / (BLOCK_SIZE * 8); // número do bloco onde a posição fica
	int offset = (number % (BLOCK_SIZE * 8)) / 8; // byte onde está o bit referente ao bloco ou inode
	int final_offset = (number % (BLOCK_SIZE * 8)) % 8; // offset do bit dentro do byte
	
	// VERIFICA SE OS VALORES OBTIDOS FAZEM SENTIDO
	
	if( ler_bloco(bloco,&(bm.mat)) == 0)
		return 0;
	
	bm.mat[offset] = inverter_bit(bm.mat[offset],final_offset);
	
	if(escrever_bloco(bloco,&(bm.mat)) == 0)
		return 0;
	
	return 1; 
}

unsigned char inverter_bit(unsigned char valor, int pos)
{
	int i = 7,h;
	int resto, aux;
	char vetor[8] = {'0','0','0','0','0','0','0','0'};
	
	aux = valor;
	while(aux != 0)
	{
		resto = aux % 2;
		aux /= 2;
		
		vetor[i] = resto == 1 ? '1' : '0';
		i--;
	}
	
	vetor[pos] = vetor[pos] == '1'? '0' : '1'; // inverte o bit da posição requerida
	
	aux = 0;
	for(i = 7, h = 0; i >= 0; i--, h++)
	{
		aux += (vetor[i] == '1'? 1 : 0) * pow(2,h);
	}
	
	return aux;	
}

/*
get_bitmap_pos_status
---------------
Entrada: inteiro, indicando a posição (em bits) a ser alterada, estrutura de superbloco para se obter informações de início do bitmap; inteiro, 1 para DATA, 2 para INODE
Descrição: retorna o bit da posição indicada
Saída: -1 em falha, um inteiro não negativo em sucesso
*/
int get_bitmap_pos_status(int number, superblock sb, int tipo)
{
	bitmap bm;

	// verificar se sb é válido
	int bloco = tipo == 1? sb.data_bitmap_begin : sb.inode_bitmap_begin;

	bloco = bloco + number / (BLOCK_SIZE * 8); // número do bloco onde a posição fica
	int offset = (number % (BLOCK_SIZE * 8)) / 8; // byte onde está o bit referente ao bloco ou inode
	int final_offset = (number % (BLOCK_SIZE * 8)) % 8; // offset do bit dentro do byte
	
	// VERIFICA SE OS VALORES OBTIDOS FAZEM SENTIDO
	
	if( ler_bloco(bloco,&(bm.mat)) == 0)
		return 0;

	int i = 7,h;
	int resto, aux;
	char vetor[8] = {'0','0','0','0','0','0','0','0'};
	
	aux = bm.mat[offset];
	while(aux != 0)
	{
		resto = aux % 2;
		aux /= 2;
		
		vetor[i] = resto == 1 ? '1' : '0';
		i--;
	}

	return atoi(vetor[final_offset]);
}


int carregar_bitmap()
{
	// carregaria o bitmap para a memória
	// a operação seria composto de carregar os blocos de cada bitmap

}

int open()
{
	/*
		recebe o pathname
		precisamos usar strtok com delimitador '/' para ir quebrando o caminho até chegar no arquivo
		procuramos pelo arquivo ou diretório, alocamos uma estrutura do tipo inode e a preenchemos
		usamos um vetor de file descriptors e damos para o inode criado um numero
	
	*/

}

int read()
{
	/*
		recebe o file descriptor obtido por meio do open
		realizamos então uma busca pelos ponteiros de dados pelo bloco que contém os bytes requeridos
		
	
	*/
}

int write()
{
	/*
		recebe o file descriptor obtido por meio do open
		teremos que verificar se é pra fazer append ou se é pra reescrever o arquivo
		sempre que precisar de um novo bloco precisamos alocar mais para o arquivo, alterando assim o bitmap de dados e os dados no inode	
	*/
}

int seek()
{
	/*
		recebe o file descriptr obtido por meio do open
		altera o "cursor" do arquivo. Esse cursor é armazenado no file descriptor do arquivo em memória.
		dada uma quantidade de bytes, procuramos pelo bloco específico.
	*/
}

int close()
{
	/*
		recebe o file descriptr obtido por meio do open
		realiza o salvamento de qualquer dados não salvos em disco e então desaloca as estruturas criadas por meio do open
	*/
}

#endif
