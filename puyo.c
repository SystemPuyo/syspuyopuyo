#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<string.h>
#include<sys/time.h>
#include<termios.h>
#include<unistd.h>
#include<sys/ioctl.h>
#include<sys/types.h>
#include<time.h>
#include<locale.h>
#include<wchar.h>

#define CCHAR 0

#ifdef CTIME
#undef CTIME
#endif

#define CTIME 1

#define LEFT 0
#define RIGHT 1
#define DOWN 2
#define LROTATE 3
#define RROTATE 4

#define GAME_START 0
#define GAME_END 1

/*puyo puyo table. acutally 12 * 8
 *left and right side is wall
 *and the below line is ground
 */
char puyo_table[13][10];

static struct result{
	char name[30];
	long point;
	int year;
	int month;
	int day;
	int hour;
	int min;
	int rank;
}temp_result;

/* puyo block rotates by only taking block A as a axis of rotation
 *
 * rotation table
 *
 * from the left to right. the state number is 0,1,2,3
 * A  A B  B  B A
 * B       A
 * R rotate increases state number. L rotate decreses state number
 */

int A_block_num = 0;
int B_block_num = 0;
int next_A_block_num = 0;
int next_B_block_num = 0;
int rotate_state = 0;

/*
 * to print unicode character
 * wprintf(L"%lc",wchar_tVal);
 */

/*function that set colors*/

void red () {
  printf("\033[1;31m");
}

void yellow() {
  printf("\033[1;33m");
}

void purple(){
    printf("\033[0;35m");
}
void green(){
    printf("\033[0;32m");
}
void blue(){
    printf("\033[0;34m");
}
void reset () {
  printf("\033[0m");
}

int display_menu(void);

int main(void){
	int menu = 1;

	while(menu){
		menu = display_menu();
	}
	return 0;
}

int display_menu(void){
	int menu = 0;
	while(1){
		system("clear");
		printf("\n\t\t\t\t Text Puyo Puyo");
		printf("\n\t\t\t===================");
		printf("\n\t\t\t\t GAME MENU\t\n");
		printf("\n\t\t\t===================");
		printf("\n\t\t\t=\t1) Start Game\t\t=");
		printf("\n\t\t\t=\t2) Search Record\t\t=");
		printf("\n\t\t\t=\t3) Display Record\t\t=");
		printf("\n\t\t\t=\t4) EXIT\t\t=");
		printf("\n\t\t\t===================");
		scanf("%d",&menu);
		if(menu<1||menu > 4){
			continue;
		}
		else{
			return menu;
		}

	}
	return 0;
}
