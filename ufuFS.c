#ifndef UFUFS_H
#define UFUFS_H

/*
Este módulo contém funções relacionadas com a operação do sistema de arquivos em si. Como abertura do device driver e manipulação das estruturas de metadados.

*/

#include<math.h>


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
	int b = tipo == 1? sb.data_bitmap_begin : sb.inode_bitmap_begin;

	b = b + number / (BLOCK_SIZE * 8); // número do bloco onde a posição fica
	int offset = (number % (BLOCK_SIZE * 8)) / 8; // byte onde está o bit referente ao bloco ou inode
	int final_offset = (number % (BLOCK_SIZE * 8)) % 8; // offset do bit dentro do byte
	
	// VERIFICA SE OS VALORES OBTIDOS FAZEM SENTIDO
	
	if( ler_bloco(b,&(bm.mat)) == 0)
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

int open(char *pathname)
{
	char *token;
	inode dir;
	
	int retorno = read_inode(0,&dir); // dir agora tem o inode do raiz
	if(!retorno)
		return -1;
		
	while(1)
	{
		token = strtok(pathname, "/");
		// se token for uma string vazia então temos em dir o inode do arquivo que queremos
		if(token[0] == '\0')
			break;
		
		while(1)
		{
			// percorre cada entrada do diretório em questão procurando pela próxima entrada
				// quando encontrar lê o inode para dir
			// se não encontrar a entrada, retorna -1
			
		}
	}

	/*
		recebe o pathname
		precisamos usar strtok com delimitador '/' para ir quebrando o caminho até chegar no arquivo
		procuramos pelo arquivo ou diretório, alocamos uma estrutura do tipo inode e a preenchemos
		usamos um vetor de file descriptors e damos para o inode criado um numero
	
	*/

}

/*
ufufs_read
----------
Entrada: inteiro indicando o file descriptor do arquivo, ponteiro para o posição da memória onde os dados lidos devem ser armazenados, inteiro indicando a quantidade de bytes a serem lidos
Descrição: 
Saída: -1 em falha, quantidade de bytes lidos em sucesso
*/
int ufufs_read(int fd, void *buffer, int qtd)
{
	// teremos que usar a estrutura em memória do inode, essa estrutura vai ter um valor chamado de CURSOR que indica o proximo byte a ser lido
	// em falta dessa estrutura, referencia a esse valor será dada por CURSOR
	
	int b = CURSOR / BLOCK_SIZE; //determina o bloco de dados onde está o próximo byte a ser lido
	int pode_lido = BLOCK_SIZE - (CURSOR % BLOCK_SIZE);//quantidade de bytes que podem ser lidos do primeiro bloco 

	//  void *memcpy(void *dest, const void *src, size_t n);

	/*
		recebe o file descriptor obtido por meio do open
		realizamos então uma busca pelos ponteiros de dados pelo bloco que contém os bytes requeridos
	*/
}

/*
ufufs_write
----------
Entrada: inteiro indicando o file descriptor do arquivo, ponteiro para o posição da memória onde os dados a serem escritos estão armazenados, inteiro indicando a quantidade de bytes a serem escritos
Descrição: 
Saída: -1 em falha, quantidade de bytes escritos em sucesso
*/
int ufufs_write(int fd, void *buffer, int qtd)
{
	// verifica se CURSOR (consultar explicação em ufufs_read) é 0, nesse caso todos os data_blocks devem ser desconsiderados e liberados se não forem ocupados
	// se o CURSOR for maior que zero é pra colocar os novos bytes à partir dessa posição alocando novos data_blocks sempre que necessário


	/*
		recebe o file descriptor obtido por meio do open
		teremos que verificar se é pra fazer append ou se é pra reescrever o arquivo
		sempre que precisar de um novo bloco precisamos alocar mais para o arquivo, alterando assim o bitmap de dados e os dados no inode	
	*/
}

int ufufs_seek(int fd)
{
	/*
		recebe o file descriptr obtido por meio do open
		altera o "cursor" do arquivo. Esse cursor é armazenado no file descriptor do arquivo em memória.
		dada uma quantidade de bytes, procuramos pelo bloco específico.
	*/
}

int ufufs_close(int fd)
{
	/*
		recebe o file descriptr obtido por meio do open
		realiza o salvamento de qualquer dados não salvos em disco e então desaloca as estruturas criadas por meio do open
	*/
}

#endif
