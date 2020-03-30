#pragma once
#include "ch-console.h"


int		cls()
{	// limpa a tela no windows, do jeito oficial
	CONSOLE_SCREEN_BUFFER_INFO		info;
	HANDLE		H = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD		origem = { 0,0 };
	int			total;
	if (H == INVALID_HANDLE_VALUE) return -1;
	GetConsoleScreenBufferInfo(H, &info);
	int r = FillConsoleOutputCharacter(H, (TCHAR)' ',
		info.dwSize.X * info.dwSize.Y,
		origem, &total);
	int s = FillConsoleOutputAttribute(
		H, info.wAttributes,
		info.dwSize.X * info.dwSize.Y,
		origem, &total);
	SetConsoleCursorPosition(H, origem);
	return 0;
};	// end cls()


WORD cor_atual_fundo()
{
	HANDLE	H = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO		info;
	GetConsoleScreenBufferInfo(H, &info);
	return (info.wAttributes & 0xF0);
};	// cor_atual_fundo()


WORD cor_atual_letras()
{
	HANDLE	H = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO		info;
	GetConsoleScreenBufferInfo(H, &info);
	return (info.wAttributes & 0xF);
};	// cor_atual_letras()


char	choice()
{
	HANDLE		I = GetStdHandle(STD_INPUT_HANDLE);
	int		total = 0;
	typedef struct _INPUT_RECORD Input;
	char	c;
	Input	buffer[2];
	int n = PeekConsoleInput(I, buffer, 2, (LPDWORD)&total);
	if (total == 0) return 0;
	if (buffer[0].EventType == KEY_EVENT)
	{
		if (buffer[0].Event.KeyEvent.bKeyDown)
		{
			c = buffer[0].Event.KeyEvent.uChar.AsciiChar;
			FlushConsoleInputBuffer(I);
			return c;
		};	// if
	};	// if()
	return 0;
};	// choice()


void	gotoYX(int linha, int coluna)
{
	static COORD	coord;
	HANDLE			H = GetStdHandle(STD_OUTPUT_HANDLE);
	coord.X = coluna; coord.Y = linha;
	SetConsoleCursorPosition(H, coord);
	return;
};	// gotoXY


void	mensagem_em_video_reverso(char* mensagem)
{
	HANDLE	H = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO		info;
	GetConsoleScreenBufferInfo(H, &info);
	WORD foreground = info.wAttributes & 0xF;
	WORD background = info.wAttributes & 0xF0;
	text_color(background, foreground);
	printf("%s", mensagem);
	text_color(foreground, background);
	return;
}	// mensagem_em_video_reverso()


void mensagem_em_cores(char* mensagem, const char letras, const char fundo)
{
	// mostra a mensagem na posicao do cursor, usando as cores 'letras'
	// e o fundo 'fundo', mas retornando ao que estava configurado
	// antes
	if (strlen(mensagem) == 0) return; // sem mensagem
	text_color(letras, fundo); // muda a cor
	printf("%s\n", mensagem); // escreve
	restaura_cores(1); // volta ao original
	return;
};	// mensagem_em_cores()


void mostra_grade_256_cores()
{
	cls();
	text_color(15, 0);	/* preto sobre branco */
	printf("Gabarito (Frente|Fundo) escritona linha 5 coluna 10\n");
	printf("    os numeros sao das cores que o windows usa\n\n\n");

	for (int letra = 0; letra < 16; letra += 1)
	{
		for (int fundo = 0; fundo < 16; fundo += 1)
		{
			text_color(letra, fundo);
			printf(" %2d %2d ", letra, fundo);
			text_color(_branco_, _preto_);
			printf("  ");
		}
		text_color(_branco_, _preto_);
		printf(" \n");
	}

	printf("\nteste: antes de escrever em amarelo sobre preto\n");
	text_color(_amarelo_, _preto_);
	printf("Amarelo sobre preto: os numeros sao das cores que o windows usa\n");
	text_color(_branco_, _preto_);
	printf("teste depois de restaurar a cor\n");

	return;
};	// mostra_grade_256_cores()


void restaura_cores(int parm)
{
	HANDLE	H = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO		info;
	static WORD init_letras = -1; // invalidado
	static WORD init_fundo  = -1; // invalidado

	if (parm == 0)
	{	// chamada inicial: salva as cores em uso no terminal
		// para poder restaurar a qualqeur momento
		GetConsoleScreenBufferInfo(H, &info);
		init_letras = info.wAttributes & 0xF;
		init_fundo  = info.wAttributes & 0xF0;
	}
	else
	{	// restaura
		if (init_letras > 0) 
			text_color(init_letras, init_fundo);
	};	// if
	return;
};	// restaura_cores()


void	text_color(int letras, int fundo)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), letras | (fundo << 4));
}	// text_color

// fim de ch-console.c