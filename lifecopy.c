#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>

#define COL 50
#define ROW 50
#define DIMENSION (ROW*COL)
#define DEAD ' '
#define ALIVE 'o'
#define CURSOR '*'



// Get the index of the ARRAY given COORDINATES x,y
int get_index(int row, int col, int maxRow, int maxCol) {
	if ( row < 0 ) {
		row = row % maxRow;
		row = maxRow + row;
	}

	if (col < 0) {
		col = col % maxCol;
		col = maxCol + col;
	}
	
	if (row >= maxRow) row = row % maxRow;
	if (col >= maxCol) col = col % maxCol;

	int index = (row) * maxCol + col;
	return index;
}

// Initialize matrix to all DEAD cells
void init_matrix(char *matrix_vector, char state, int maxRow, int maxCol) {
	for (int i=0; i<maxRow; i++) {
		for (int j=0; j<maxCol; j++) {
			matrix_vector[get_index(i, j,maxRow,maxCol)] = state;
		}
	}
}

// Set the state of a matrix element
void set_matrix(char *matrix_vector, int x, int y, int cursor) {

	matrix_vector[get_index(x, y,ROW,COL)] = ALIVE;
	if (cursor ) matrix_vector[get_index(x, y,ROW,COL)] = CURSOR;
}

void set_dead(char *matrix_vector, int x, int y) {    // implement this in set matrix
	matrix_vector[get_index(x, y,ROW,COL)] = DEAD;
}

void screen(char *matrix_vector, int maxRow, int maxCol) {
	
	printf("\x1b[3J\x1b[H\x1b[2J"); // Clear screen.
	for (int i=0; i<ROW; i++) {
		for (int j=0; j<COL; j++) {
			printf("%c ", matrix_vector[get_index(i,j,maxRow,maxCol)]);
		}
		printf("\n");
	}
}

int counting_neigh(char *matrix_vector, int row, int col) {
	int count = 0;
	if ( matrix_vector[get_index(row,col,ROW,COL)] == ALIVE ) count--;
	for (int i=-1; i <= 1; i++) {
		for (int j=-1; j <= 1; j++) {
			if ( matrix_vector[get_index(row+i,col+j,ROW,COL)] == ALIVE ) count++;
		}

	}

	return count;
}

void compute_new_state(char *matrix_vector) {
	int alive;                                           //Rethink this one
	int current;
	char new_matrix[DIMENSION];
	for (int i=0; i<ROW; i++) {
		for (int j=0; j<COL; j++) {

			current = matrix_vector[get_index(i,j,ROW,COL)];
			alive = counting_neigh(matrix_vector, i,j );
			new_matrix[get_index(i,j,ROW,COL)] = DEAD;
			if (current == ALIVE) {
				if (alive == 2 || alive == 3) {
					new_matrix[get_index(i,j,ROW,COL)] = ALIVE;
				}
			}
			if (current == DEAD) {
				if (alive == 3) {
					new_matrix[get_index(i,j,ROW,COL)] = ALIVE;
				}
			}
		}
	}

	

	for (int i=0; i<DIMENSION; i++) {
		matrix_vector[i] = new_matrix[i];
	}
	screen(matrix_vector,ROW,COL);
}

struct termios original_mode;

void enableRAW() {
	tcgetattr(STDIN_FILENO, &original_mode);
	struct termios raw_mode = original_mode;

	raw_mode.c_lflag &= ~(ICANON);
	tcsetattr(STDIN_FILENO,TCSAFLUSH, &raw_mode);
}


void diasbleRAW() {
	tcsetattr(STDIN_FILENO,TCSAFLUSH, &original_mode);
}

void read_cursor(char *matrix_vector){
	enableRAW();
	char c;
	int curs_x = 0;
	int curs_y = 0;

	screen(matrix_vector,ROW,COL);
	
	char old_state = DEAD ;
	while (read(STDIN_FILENO, &c, 1) == 1) {
		if ( old_state == DEAD) set_dead(matrix_vector, curs_x, curs_y);
		if ( old_state == ALIVE) set_matrix(matrix_vector ,curs_x, curs_y, 0);
		switch (c) {
			case 'q' : 	return;
			case 'h' : curs_y--  ; break;
			case 'j' : curs_x++  ; break;
			case 'u' : curs_x--  ; break;
			case 'k' : curs_y++  ; break;
			case 's' :
				if ( old_state == ALIVE) set_dead(matrix_vector, curs_x, curs_y);
				if ( old_state == DEAD) set_matrix(matrix_vector ,curs_x, curs_y, 0);
				break;	
		}
		old_state = matrix_vector[get_index(curs_x,curs_y,ROW,COL)];
		set_matrix(matrix_vector, curs_x, curs_y, 1);
		screen(matrix_vector,ROW,COL);

	}	
}

void read_file(char *matrix_vector) {
	char matrix_read[38*38];
	init_matrix(matrix_read, DEAD,38, 38);
	FILE *fptr = fopen("pattern.txt","r");
	if (fptr == NULL) {
	perror("Error opening file");
	return;
	}
	char ch;
	int count = 0;
	while ( (ch = fgetc(fptr)) != EOF ){
		if (ch == '\n' || ch == '\r') continue;
		else {
			matrix_read[count++] = (char)ch;
		}
	}
	
	fclose(fptr);

	for (int i=0; i<38; i++) {
		for (int j=0; j<38; j++) {
			matrix_vector[get_index(i,j,ROW,COL)] =  matrix_read[get_index(i,j,38,38)];
		}
	}
	screen(matrix_vector,ROW,COL);
}

void save_config(char *matrix_vector){
	FILE *fptr = fopen("logConfig.txt","w");
	if (!fptr) {
		perror("Error opening file");
		return;
	}
	
	for(int i=0; i<ROW; i++) {
		for(int j=0; j<COL; j++) {
			fprintf(fptr,"%c",matrix_vector[get_index(i,j,ROW,COL)]);
		}
		fprintf(fptr,"\n");
	}
	fclose(fptr);
}


int main(void) {
	
	char matrix_vector[DIMENSION];
	init_matrix( matrix_vector, DEAD,ROW,COL);
	printf("Read from file?\n");
	char answer;
	read(STDIN_FILENO,&answer,1);
	if (answer  == 'y') read_file(matrix_vector);
	else read_cursor(matrix_vector);
	save_config(matrix_vector);
	while (1) {
		compute_new_state(matrix_vector);
		usleep(200000);
	}
	

	return 0;
}
