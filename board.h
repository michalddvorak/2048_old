#ifndef BOARDH_323614351435413513545213
#define BOARDH_323614351435413513545213

struct board
{
	int** m_arr;
	int m_rows;
	int m_cols;
};

void alloc_board(struct board* board);
void zero_board(struct board* board);
void free_board(struct board* board);
int is_equal_board(const struct board* b1, const struct board* b2);
void clone_board(const struct board* old_board, struct board* new_board);

#endif /*BOARDH_323614351435413513545213*/