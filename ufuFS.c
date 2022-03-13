#ifndef UFUFS_H
#define UFUFS_H

/*
Este módulo contém funções relacionadas com a operação do sistema de arquivos em si. Como abertura do device driver e manipulação das estruturas de metadados.

*/

/*
abrir_dispositivo
------------------
Entrada: String indicando o caminho até o device file do dispositivo de armazenamento que será acessado.
Descrição:
Saída: 
*/
int abrir_dispositivo(const char *pathname)
{
	// system call open para abrir o device file
	// fstat para obter o tamanho em bytes do dispositivo

}


int alterar_bitmap()
{
	// PARA DADOS
	// tendo em mãos o bloco que tem que ter o status alterado realizamos a operação:  NUM_BLOCK / (TAMANHO_BLOCK * 8) 
	// o quociente será o bloco do bitmap (numeração começando com zero),
	// Com o resto em mãos, dividimos por 8, obtendo assim o byte onde o indicador do bloco está localizado. 
	// O novo resto será o offset da localização do bit referente.
	// Logo, queremos inverter apenas aquele bit do byte em específico.
	// podemos fazer isso copiando o byte, e de acordo com a operação fazer +- 1 * 2^NOVO_RESTO
	//  então aplicamos bitwase XOR (^)
	// escrever o bloco


	// PARA INODES
	// Oq muda em relação ao bitmap_dados é que invés de NUM_BLOCK a função receberá o INODE_NUMBER 
	// com isso, precisaremos de apenas uma função.
	
	// usamos unsigned char para operar sobre cada byte 
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
