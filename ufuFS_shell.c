#ifndef UFUFS_SHELL_H
#define UFUFS_SHELL_H

/*
Módulo do mini-shell
*/

int main()
{
	


	return 0;
}





/*
	Funcionalidades necessárias:
		criar arquivos e diretórios
		excluir arquivos e diretórios
		listar os arquivos armazenados (lista em ordem alfabetica, usar uma arvore AVL)
		copiar arquivo do ufuFS para o sistema base e vice-versa



	Pode ser útil saber como pegar o path em que o processo está trabalhando
*/

/*
	criar arquivos
	
	vai até o diretório onde será criado
	aloca um inode e o preenche
	coloca a entrada no diretório do arquivo
	marca o bitmap de inodes
*/

/*
	excluir arquivos e diretórios
	
	localiza o arquivo a ser excluido
	desmarca o bitmap de dados
	desmarca o bitmap do inode
	zera o inode
	remove a entrada do diretório
*/

/*
	listar arquivos
	
	localizar diretório
	CRIAR ESTRUTURA PARA ARMAZENAR OS DADOS NECESSÀRIOS JUNTO COM O NOME 
		talvez:
				struct no
				{
					char nome[11]
					inode in;
				};
	Recuperar todos os dentry do diretório e ir armazenando em uma estrutura na forma de árvore. (árvore simples, sem ser avl)
	Imprimir para o usuário em ordem alfabética
	Para a questão da árvore pegar código pronto do dicionário feito em AED2

*/

/*
	copiar arquivo do ufuFS para o sistema base
	
	obter caminho para onde o arquivo será copiado
	obter caminho do arquivo a ser copiado
	criar arquivo no destino
	abrir arquivo a ser copiado
	ler bloco por bloco do arquivo a ser copiado e escrever bloco por bloco no arquivo de destino
	fechar ambos arquivos

*/


/*
	copiar arquivo do sistema base para ufuFS
	
	obter caminho do arquivo a ser copiado
	obter caminho para onde o arquivo será copiado
	criar arquivo no destino
	abrir arquivo a ser copiado
	ler uma quantidade de BLOCK_SIZE e escrever no arquivo destino até que tudo tenha sido copiado
	fechar ambos arquivos.	
	

*/


#endif
