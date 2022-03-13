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


#endif
