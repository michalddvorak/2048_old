#include <stdio.h>
#include "printer.h"


struct rgb
{
	int r, g, b;
};

#define LIGHT_BLUE {66, 135, 245}
#define DARK_BLUE {1,49,125}
#define BROWN {125, 65, 1}
#define YELLOW {247,210,22}
#define LIGHT_GREEN {146,247,22}
#define TEAL {22, 247, 202}
#define PURPLE {191,22,247}
#define LIGHT_RED {191,86,86}
#define PINK {255, 0, 204}
#define DARK_GREEN {9,117,4}
#define ORANGE {255,170,0}
#define DARK_PURPLE {70,8,94}
#define REDRANGE {235,87,38}

struct rgb colors[12] = {
		LIGHT_BLUE,
		DARK_BLUE,
		BROWN,
		ORANGE,
		DARK_GREEN,
		PURPLE,
		DARK_PURPLE,
		LIGHT_RED,
		PINK,
		REDRANGE,
		ORANGE
};


void clear_screen()
{
	printf("\033[2J");
}

void move_cursor(int i, int j)
{
	printf("\033[%d;%dH", i, j);
}

void print_corner(int n)
{
	printf("+");
	for(int j = 0; j < n; ++j)
		printf("----+");
	printf("\n");
}


void print_middle(const struct board* board, int i, int is_num)
{
	printf("|");
	for(int j = 0; j < board->m_cols; ++j)
	{
		int x = board->m_arr[i][j];
		int idx = __builtin_popcount(x - 1) - 1;
		if(x != 0)
			printf("\033[48;2;%d;%d;%dm", colors[idx].r, colors[idx].g, colors[idx].b);
		if(is_num && x) //don't print zeroes
			printf("%4d", x);
		else
			printf("    ");
		if(x != 0)
			printf("\033[0m");
		printf("|");
	}
	printf("\n");
}

void print_board(const struct board* board)
{
	move_cursor(1, 1);
	print_corner(board->m_cols);
	for(int i = 0; i < board->m_rows; ++i)
	{
		print_middle(board, i, 0);
		print_middle(board, i, 1);
		print_middle(board, i, 0);
		print_corner(board->m_cols);
	}
}