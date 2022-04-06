#include<stdlib.h>
#include<math.h>
#include<string.h>
#include"ufuFS.h"
#include<stdio.h>


/*
obter_inode_livre
------------------
Entrada: inteiro contendo o file descriptor do device file
		 estrutura de superbloco para se obter informações de início do bitmap
Descrição: procura pelo primeiro inode não ocupado
Saída: -1, em falha, inteiro não negativo indicando o inode livre encontrado
*/
int obter_inode_livre(int fd, superblock sb)
{
	unsigned char *bitmap = (unsigned char *) calloc(1,BLOCK_SIZE);
	int num_inode = 0;

	if(fd < 0)
		return 0; // file descriptor inválido

	for(int i = sb.inode_bitmap_begin; i < sb.data_bitmap_begin; i++)
	{
		ler_bloco(fd, i, bitmap); // lê bloco a bloco do inode_bitmap
		
		for(int h = 0; h < BLOCK_SIZE; h++)
		{
			if(bitmap[h] != 255) // procura pelo primeiro byte que não tem apenas 1's
			{
				num_inode += h * 8;
				return num_inode + localizar_bit(bitmap[h],'0',-1);
			}
		}
		num_inode += BLOCK_SIZE * 8;
	}
	
	return -1;
}

/*
localizar_bit
--------------
Entrada: unsigned char, contendo o byte
         caracter, indicando o bit procurado, '0', ou '1'
         inteiro, indicando o sentido, -1 pra esquerda pra direita e 1 da direita pra esquerda
Descrição: procura pelo primeiro bit dado no sentido indicado
Saída: inteiro indicando o primeiro zero, da esquerda pra direita; -1 se não existir nenhum (incluindo quando sentido e bit são valores inválidos
*/
int localizar_bit(unsigned char valor, char bit, int sentido)
{
	/*
		O operador AND é 1 se ambos bit são 1 e 0 em qualquer outro caso.
		No sentido da esquerda pra direita (-1) começamos com 128U e diminuímos até 1U, se a operação (valor AND potência) resultar em 0, então a posição
			é 0, se for diferente de 0 aquele bit é 1.	
	*/

	int i;

	if(sentido == -1)
	{
		if(bit == '1')
		{
			for(i = 0; i < 8; i++)
			{
				if( (valor & (128U >> i)) > 0)
				{
					// encontrou o primeiro 1 da esquerda pra direita
					return i;
				}
			}
		
		}
		else
		{
			for(i = 0; i < 8; i++)
			{
				if( (valor & (128U >> i)) == 0)
				{
					// encontrou o primeiro 0 da esquerda pra direita
					return i;
				}
			}
		}
	}
	else if(sentido == 1)
	{
		if(bit == '1')
		{
			for(i = 0; i < 8; i++)
			{
				if( (valor & (1U << i)) > 0)
				{
					// encontrou o primeiro 1 da direita pra esquerda
					return (7 - i);
				}
			}
		
		}
		else
		{
			for(i = 0; i < 8; i++)
			{
				if( (valor & (1U << i)) == 0)
				{
					// encontrou o primeiro 0 da direita pra esquerda
					return (7 - i);
				}
			}
		}
	}
	
	return -1;// não existe nenhuma posição com o valor
}

/*
alterar_bitmap
---------------
Entrada: inteiro contendo o file descriptor do device file
		 inteiro, indicando a posição (em bits) a ser alterada
		 estrutura de superbloco para se obter informações de início do bitmap; 
		 inteiro, 1 para DATA, 2 para INODE
Descrição: realiza a alteração da posição indicada
Saída: 0, em fracasso, 1, em sucesso
*/
int alterar_bitmap(int fd, int number, superblock sb, int tipo)
{
	unsigned char *bitmap = (unsigned char *) calloc(1,BLOCK_SIZE);

	if(fd < 0)
		return 0; // file descriptor inválido

	// verificar se sb é válido
	int b = tipo == 1? sb.data_bitmap_begin : sb.inode_bitmap_begin;

	b = b + number / (BLOCK_SIZE * 8); // número do bloco onde a posição fica
	int in_block = number % (BLOCK_SIZE * 8);
	int offset = in_block / 8; // byte onde está o bit referente ao bloco ou inode
	int final_offset = in_block % 8; // offset do bit dentro do byte
	
	if( ler_bloco(fd,b,bitmap) == 0)
		return 0;
	
	bitmap[offset] = inverter_bit(bitmap[offset],final_offset);
	
	if(escrever_bloco(fd,b,bitmap) == 0)
		return 0;
	
	return 1; 
}


/*
inverter_bit
---------------
Entrada: um char, indicando um byte
		 inteiro, indicando a posiçãp a ser invertida
Descrição: inverte o bit de valor na posição indicada
Saída: retorna o char valor com o bit em pos invertido
*/
unsigned char inverter_bit(unsigned char valor, unsigned int pos)
{ 
	/* 
	   128U em binário: 1000 0000
	   pos determina a quantidade de posições que os bits devem ser deslocados para a direita ( >> ), 0 <= pos <= 7
	   o ^ é o operador de bitwase XOR, bits iguais viram 0 e bits diferentes viram 1.
	   
	   Nessa função o segundo byte do XOR sempre terá apenas uma posição com 1. 
	   Como todas as outras posições estarão zeradas e tendo-se em vista que 0 XOR 0 = 0 e 1 XOR 0 = 1, logo o resto do byte fica inalterado.
	   O bit na única posição com 1 é então invertido, pois 1 XOR 1 = 0 e 0 XOR 1 = 1.
	*/	
	
	return valor ^ (128U >> pos); 
}

/*
alterar_faixa_bitmap
---------------
Entrada: inteiro, indicando o file descriptor do device file
		 inteiro, indicando o bloco de inicio
		 inteiro, indicando o bloco de fim
		 superbloco
Descrição: inverte os bits no bitmap que indicam os blocos entre inicio e fim, inclusos
Saída: -1 em falha, 0 em sucesso
*/
int alterar_faixa_bitmap(int fd, int inicio, int fim, superblock sb)
{
	// verificação dos argumentos
		// return -1 em falha;

	unsigned char *bitmap = (unsigned char *) calloc(1,BLOCK_SIZE);

	int b = sb.data_bitmap_begin;
	
	b = b + inicio / (BLOCK_SIZE * 8); // número do bloco onde o inicia fica
	int resto_inicial = 8 - ((inicio + 1) % 8) + 1; // quantidade de bits em um byte incompleto
	int resto_final = (fim + 1) % 8; // quantidade de bits final em um byte imcompleto
	int qtd = fim - inicio + 1;
	int in_block = inicio % (BLOCK_SIZE * 8);
	int in_block_fim = fim % (BLOCK_SIZE * 8);
	int inside_offset;

	int i,h,j;
	
	if( in_block / 8 == in_block_fim / 8)
	{
		// faixa inteira dentro de um byte
		for(h = inicio; h <= fim; h++)// altera as posições do byte incompleto
		{
			alterar_bitmap(fd, h, sb, 1); // altera posição por posição
		}

		return 1;
	}	
	
	if(resto_inicial == 8)
		resto_inicial = 0;
	
	inside_offset = in_block / 8; // byte dentro do bloco
	for(i = b; i < sb.file_table_begin; i++)
	{
		if(resto_inicial != 0) // apenas quando existe um byte inicial incompleto
		{
			for(h = inicio, j=0; j < resto_inicial; j++, h++)// altera as posições do byte incompleto
			{
				alterar_bitmap(fd, h, sb, 1); // altera posição por posição
			}

			qtd -= resto_inicial;
			inicio += resto_inicial; // como alteramos os primeiros bits, movemos o apontador de inicio
			resto_inicial = 0;
			inside_offset++;

			if(qtd == 0)
				break;
		}

		ler_bloco(fd,i,bitmap);
		while(qtd > 8 && inside_offset < BLOCK_SIZE)
		{
			bitmap[inside_offset] = ~ bitmap[inside_offset];

			inicio += 8;
			inside_offset++;
			qtd -= 8;
		}
		escrever_bloco(fd,i,bitmap);
		
		if(qtd == 0)
			break;
		
		if(qtd < 8 && inside_offset < BLOCK_SIZE) // byte final incompleto
		{
			for(h = inside_offset * 8, j=0; j < resto_final; j++, h++)
			{
				alterar_bitmap(fd, h,sb, 1); // altera posição por posição
			}
			
			resto_final = 0;
			qtd -= resto_final;
		}

		inside_offset = 0;
				
		if(qtd == 0)
			break;
	}
	return 0;
}

/*
get_bitmap_pos_status
---------------
Entrada: inteiro contendo o file descriptor do device file
		 inteiro, indicando a posição a ser consultada 
	     estrutura de superbloco para se obter informações de início do bitmap
	     inteiro, 1 para DATA, 2 para INODE
Descrição: retorna o bit da posição indicada
Saída: -1 em falha, um inteiro não negativo em sucesso
*/
int get_bitmap_pos_status(int fd, int number, superblock sb, int tipo)
{
	unsigned char *buffer = (unsigned char *) calloc(1,BLOCK_SIZE);	

	// verificar se sb é válido
	int b = tipo == 1? sb.data_bitmap_begin : sb.inode_bitmap_begin;

	b = b + number / (BLOCK_SIZE * 8); // número do bloco onde a posição fica
	int in_block = number % (BLOCK_SIZE * 8);
	int offset = in_block / 8; // byte onde está o bit referente ao bloco ou inode
	int final_offset = in_block % 8; // offset do bit dentro do byte
	
	if(b >= (tipo == 1? sb.file_table_begin : sb.data_bitmap_begin))
		return -1; 
	
	if( ler_bloco(fd,b,buffer) == 0)
		return 0;

	int aux = buffer[offset];

	if( (aux & (128U >> final_offset)) > 0)
		return 1;
	else
		return 0;
}

/*
get_block_sequence
-------------------
Entrada: inteiro, indicando o file descript do dispositivo
		 inteiro, indicando o primeiro bloco do bitmap a ser vasculhado (0 para ser o primerio bloco do bitmap)
		 superblock, contendo as informações do superblock do sistema de arquivos
		 inteiro, indicando a quantidade de blocos necessária na sequência
Descrição: Procura por uma sequência de blocos de dados suficiente para o arquivo
Saída: -1, se nenhuma sequencia foi encontra, um inteiro, indicando o bloco que inicia a sequencia.
*/
int get_block_sequence(int fd, int comeco, superblock sb, int qtd)
{
	// sempre que formos chamar essa função em um programa, comeco é zerado. É uma variável de uso da própria função
	int min_completos = (qtd / 8); // quantidade minima de bytes que precisam estar zerados, podem sobrar até 7 bits
	int resto = qtd % 8;
	unsigned char *data_block = (unsigned char *) calloc(1,BLOCK_SIZE);
	unsigned char *vazio = (unsigned char *) calloc(1,min_completos);

	int j,k,i,h;
	int aux,antes = 0,depois;
	
	for(i = sb.data_bitmap_begin + comeco, j = 0; i < sb.file_table_begin; i++, j++)
	{
		ler_bloco(fd,i,data_block);
		
		for(h = 0; h < BLOCK_SIZE - min_completos - 1; h++)
		{
			if(min_completos == 0)
			{
				if(memcmp(data_block + h, vazio, 1) != 255)
				{
					antes = localizar_bit(data_block[h], '1', 1);
					
					if(antes + 1 < 8)
					{			
						aux = resto - (8 - (antes + 1));
						
						if(aux <= 0)
						{
							return j * (BLOCK_SIZE * 8) + (h * 8) + (antes + 1);
						}
						else
						{
							depois = localizar_bit(data_block[h + 1], '1', -1);
							
							if(aux - depois <= 0)
							{
								return j * (BLOCK_SIZE * 8) + (h * 8) + (antes + 1);
							}
						}
					}
				}
			}
		
			if( memcmp(data_block + h, vazio, min_completos) == 0) // procura pela quantidade determinada de blocos zerados seguidos
			{
				antes = 0;
				if(h > 0) // se não começar no primeiro byte, verifica o anterior
				{
					antes = 8 - (localizar_bit(*(data_block + h - 1),'1',1) + 1); // quantidade de bits zerados na direita do byte
					aux = resto - antes; // quantidade de bits fora de bytes a serem alocados
				}
				else
					aux = resto;
					
				if(aux <= 0 || (qtd - antes) / 8 == 0)
					return j * (BLOCK_SIZE * 8) + ((h - 1) * 8) + (8 - antes);
									
				depois = localizar_bit(*(data_block + h + min_completos + 1),'1',-1);// quantidade de bits zerados à esquerda do byte
				aux -= depois;
				
				if(aux <= 0)
					return j * (BLOCK_SIZE * 8) + (h * 8) + (8 - antes);
			}
		
		}

		for(k = 0;h < BLOCK_SIZE; h++, k++)
		{
			if(min_completos == 0)
			{
				if(memcmp(data_block + h, vazio, 1) != 255)
				{
					antes = localizar_bit(data_block[h], '1', 1);
					
					if(antes + 1 < 8)
					{
						aux = resto - (8 - (antes + 1));
						
						if(aux <= 0)
						{
							return j * (BLOCK_SIZE * 8) + (h * 8) + (antes + 1);
						}
						else
						{
							if(h + 1 == BLOCK_SIZE)
							{
								if ((depois = get_block_sequence(fd,(i - comeco) + 1,sb, aux)) == -1)
									return -1;
							}
							else
							{
								depois = localizar_bit(data_block[h + 1], '1', -1);
							}
							
							if(aux - depois <= 0)
							{
								return j * (BLOCK_SIZE * 8) + (h * 8) + (antes + 1);
							}
						}
					}
				}
			}
		
			if( memcmp(data_block + h, vazio, min_completos - k) == 0)
			{
				antes = 0;
				if(h > 0)
				{
					antes = 8 - (localizar_bit(*(data_block + h - 1),'1',1) + 1); // quantidade de bits zerados na direita do byte
				}
				
				// bits zerados até o fim do block, então precisamos olhar no próximo
				if(get_block_sequence(fd, (i - comeco) + 1, sb, qtd - (8 *(min_completos - k) + antes)) == (i + 1)*(BLOCK_SIZE * 8))// caso retorne a primeira posição do proximo bloco então temos uma área contigua 
					return j * (BLOCK_SIZE * 8) + (h * 8) + (8 - antes);	
			}
		}
	
	}

	return -1;
}
