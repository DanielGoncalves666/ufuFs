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


int main()
{
	//// solicitar o caminho para o dispositivo, armazenando em pathname

	char *pathname;
	superblock sb;
	int size, resul;

	// verificar se já existe um sistema de arquivo instalado
		// se sim, perguntar se o usuário quer sobreescrever
	    // se não, continua normalmente com a formatação

	size = abrir_dispositivo(pathname); // size tem a quantidade de blocos no dispositivo
	
	if(size == -1)
		return 0; // falha
	
	resul = ler_bloco(0,sb); // carrega o bloco 0, onde o superblock ficaria se existir
	if(resul == 0)
		return 0; // falha
	
	if(sb.magic_number == UFUFS_MAGIC_NUMBER)
	{
		// avisa que já existe um ufuFS no disco e pergunta se o usuário deseja sobreescrever
		
	}
	    
	// pergunta se o usuário quer um hard ou soft format
		// hard format preenche com blocos vazios todas as posições
		// soft deixa os blocos de dados do jeito que estão e atua apenas nas estruturas de matadados
	  
	sb.magic_number = UFUFS_MAGIC_NUMBER;
	sb.block_number = size;
	sb.block_size = BLOCK_SIZE;
	sb.inode_table_length = size * 0.1; // 10% de todos os blocos
	sb.inode_bitmap_begin = 1
    sb.data_bitmap_begin = sb.inode_bitmap_begin + ceil((size * 0.1) / (BLOCK_SIZE * 8));  
	sb.file_table_begin = sb.data_bitmap_begin + ceil ((size * 0.9) / (BLOCK_SIZE * 8)); // precisa verificar se é isso msm
	sb.data_table_begin = sb.file_table_begin + size * 0.1;
	
	char *empty_block = (char *) calloc(1,BLOCK_SIZE);
	
	write(0,empty_block);//limpa o bloco do superbloco
	if(write(0,sb) == 0)
		return 0; // falha
	
	for(int i = sb.inode_bitmap_begin; i < data_table_begin; i++);
	{
		// limpa os bitmaps e a file_table
		write(i,empty_block);
	}
	
	if(/*se o usuario tiver escolhido hard format*/)
	{
		for(int i = sb.data_table_begin; i < size; i++);
		{
			write(i,empty_block);
		}
	}
	
	// cria o diretório raiz no primeiro inode
		// aloca inode
		// preenche as informações
		// armazena o inode no disco
		// aloca um data_block pra raiz e cria a referencia pra ele mesmo ( . )
		// marca o bitmpa de dados e inode

}




#endif
