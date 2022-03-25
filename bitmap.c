#include<stdlib.h>
#include<math.h>
#include<string.h>
#include"estruturas.h"
#include"bitmap.h"
#include"bloco.h"


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
	unsigned char *bitmap = (char *) malloc(sizeof(unsigned char) * BLOCK_SIZE);
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
Saída: inteiro indicando o primeiro zero, da esquerda pra direita; -1 se não existir nenhum
*/
int localizar_bit(unsigned char valor, char bit, int sentido)
{
	char vetor[8] = {'0','0','0','0','0','0','0','0'};
	int i = 7, resto, aux;
	
	
	aux = valor;
	while(aux != 0)
	{
		resto = aux % 2;
		aux /= 2;
		
		vetor[i] = resto == 1 ? '1' : '0';
		i--;
	}	
	// preenche o vetor da direita pra esquerda de modo a deixar nele o número na ordem correta
	
	i = sentido == -1 ? 0 : 7;
	for(; i < (sentido == -1 ? 7 : 0); i += (sentido == -1? 1 : -1))
	{
		if(vetor[i] == bit)
			return i;
	}

	return -1;
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
	int offset = (number % (BLOCK_SIZE * 8)) / 8; // byte onde está o bit referente ao bloco ou inode
	int final_offset = (number % (BLOCK_SIZE * 8)) % 8; // offset do bit dentro do byte
	
	// VERIFICA SE OS VALORES OBTIDOS FAZEM SENTIDO
	
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
	int resto_inicial = 8 - ((inicio + 1) % 8); // quantidade de bits em um byte incompleto
	int resto_final = (fim + 1) % 8; // quantidade de bits final em um byte imcompleto
	int qtd = fim - inicio + 1;
	int inside_offset;

	int i,h,j;

	for(i = b; i < sb.file_table_begin; i++)
	{
		ler_bloco(fd,i,bitmap);
		
		inside_offset = (inicio % (BLOCK_SIZE * 8)) / 8; // byte dentro do bloco;
		if(resto_inicial != 0)
		{
			for(h = inicio, j=0; j < resto_inicial; j++, h++)
			{
				alterar_bitmap(fd, h, sb, 1); // altera posição por posição
			}
			qtd -= resto_inicial;
			inicio += resto_inicial; // como alteramos os primeiros bits, movemos o apontador de inicio
			resto_inicial = 0;
			inside_offset++;
		}
		
		while(qtd > 8 && inside_offset < BLOCK_SIZE)
		{
			if(bitmap[inside_offset] == 0)// inverte o byte
				bitmap[inside_offset] = 1;
			else
				bitmap[inside_offset] = 0;
		
			inicio += 8;
			inside_offset++;
			qtd -= 8;
		}
		
		if(qtd < 8 && inside_offset < BLOCK_SIZE)
		{
			for(h = inicio, j=0; j < resto_final; j++, h++)
			{
				alterar_bitmap(fd, h,sb, 1); // altera posição por posição
			}
			
			resto_final = 0;
			qtd -= resto_final;
			inside_offset++;
		}
		
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
	int offset = (number % (BLOCK_SIZE * 8)) / 8; // byte onde está o bit referente ao bloco ou inode
	int final_offset = (number % (BLOCK_SIZE * 8)) % 8; // offset do bit dentro do byte
	
	if(b >= (tipo == 1? sb.file_table_begin : sb.data_bitmap_begin))
		return -1; 
	
	if( ler_bloco(fd,b,buffer) == 0)
		return 0;

	int i = 7,h;
	int resto, aux;
	char vetor[8] = {'0','0','0','0','0','0','0','0'};
	
	aux = buffer[offset];
	while(aux != 0)
	{
		resto = aux % 2;
		aux /= 2;
		
		vetor[i] = resto == 1 ? '1' : '0';
		i--;
	}

	return vetor[final_offset] == '1'? 1 : 0;
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
	
	unsigned char *data_block = (unsigned char *) calloc(1,BLOCK_SIZE);
	int min_completos = (qtd / 8); // quantidade minima de bytes que precisam estar zerados, podem sobrar até 7 bits
	int resto = qtd % 8;
	unsigned char *vazio = (unsigned char *) calloc(1,min_completos);

	int j,k,i,h;
	int aux,antes = 0,depois;

	for(i = sb.data_bitmap_begin + comeco, j = 0; i < sb.file_table_begin; i++, j++)
	{
		ler_bloco(fd,i,data_block);
		
		for(h = 0; h < BLOCK_SIZE - min_completos - 1; h++)
		{
			if( memcmp(data_block + h, vazio, min_completos) == 0) // procura pela quantidade determinada de blocos zerados seguidos
			{
				antes = 0;
				if(h > 0) // se não começar no primeiro byte, verifica o anterior
				{
					antes = 8 - localizar_bit(*(data_block + h),'1',1); // quantidade de bits zerados na direita do byte
					aux = resto - antes; // quantidade de bits fora de bytes a serem alocados
				}
				else
					aux = resto;
					
				if(aux <= 0)
					return j * (BLOCK_SIZE * 8) + (h * 8) - antes;
									
				depois = localizar_bit(*(data_block + h + min_completos),'1',-1);// quantidade de bits zerados à esquerda do byte
				aux -= depois;
				
				if(aux <= 0)
					return j * (BLOCK_SIZE * 8) + (h * 8) - antes;
			}
		
		}
		
		for(k = 1;h < BLOCK_SIZE; h++, k++)
		{
			if( memcmp(data_block + h, vazio, min_completos - k) == 0)
			{
				antes = 0;
				if(h > 0)
				{
					antes = 8 - localizar_bit(*(data_block + h),'1',1); // quantidade de bits zerados na direita do byte
				}
				
				if(get_block_sequence(fd, (i - comeco) + 1, sb, qtd - (8 *(min_completos - k) + antes)) == (i + 1)*(BLOCK_SIZE * 8))// caso retorne a primeira posição do proximo bloco então temos uma área contigua 
					return j * (BLOCK_SIZE * 8) + (h * 8) - antes;
						
			}
		}
	
	}

	return -1;
}