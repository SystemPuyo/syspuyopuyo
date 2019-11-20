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
#include<stdbool.h>

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
char puyo_table[13][10];//floor = wall |

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
int A_position_x = 0;
int A_position_y = 0;
int B_position_x = 0;
int B_position_y = 0;
int game = GAME_END;
int best_point = 0;
long point = 0;
int xTable[4] = {0,1,0,-1};//to calculate x cordinate of B
int yTable[4] = {1,0,-1,0};//to calculate y cordinate of B


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
void color(int c){
	switch(c){
		case 0: red();break;
		case 1: yellow();break;
		case 2: purple();break;
		case 3: green(); break;
		case 4: blue(); break;
	}
}
int display_menu(void);
void init_table(void);
void display_table(void);
int game_start(void);
void refresh(int);
void move_block(int);
bool isNotColide(int);
int check_drop(void);//chekc if the destruction can be done
void print_result(void);
void search_result(void);
int getch(void);//fast character input


int main(void){
	int menu = 1;

	while(menu){
		menu = display_menu();

		if(menu == 1){
			game = GAME_START;
			menu = game_start();
		}
		else if(menu == 2){
			search_result();
		}
		else if(menu == 3){
			print_result();
		}
		else if(menu == 4){
			exit(0);
		}
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

int game_start(void){
	static struct itimerval timer;
	time_t ptime;
	struct tm *t;
	FILE *fp = NULL;

	if(game == GAME_START){
		init_table();

		signal(SIGVTALRM,refresh);
		//configure the timer to expire after 1 us
		timer.it_value.tv_sec = 0;
		timer.it_value.tv_usec = 1;
		
		//and every 1 us after that
		timer.it_interval.tv_sec = 0;
		timer.it_interval.tv_usec = 1;

		setitimer(ITIMER_VIRTUAL,&timer,NULL);
		while(1){
			if(game == GAME_END){
				timer.it_value.tv_sec = 0;
				timer.it_value.tv_usec = 0;
				timer.it_value.tv_sec = 0;
				timer.it_interval.tv_usec = 0;
				setitimer(ITIMER_VIRTUAL,&timer,NULL);

				printf("\n\n final score %ld",point);
				printf("\n\n enter your name: ");
				scanf("%s",temp_result.name);
				temp_result.point = point;

				if(temp_result.point >= best_point)
					best_point = temp_result.point;

				ptime = time(NULL);
				t = localtime(&ptime);

				temp_result.year = t->tm_year + 1900;
				temp_result.month = t->tm_mon + 1;
				temp_result.day = t->tm_mday;
				temp_result.hour = t->tm_hour;
				temp_result.min = t->tm_min;

				fp = fopen("result","ab");
				fseek(fp,1,SEEK_END);
				fwrite(&temp_result,sizeof(struct result),1,fp);
				fclose(fp);
				
				A_position_x = 3;
				A_position_y = 0;
				B_position_x = 3;
				B_position_y = 1;
				point = 0;

				return 1;
			}
		}
	}
	return 0;
}

void display_table(void){
	system("clear");

	printf("\n\n Next Block\n");
	printf("\n");
	color(next_A_block_num);
	printf("@\n");
	color(next_B_block_num);
	printf("@\n");
	reset();

	for(int i = 0;i<13;i++){
		for(int j = 0;j<10;j++){
			if(puyo_table[i][j] == -1) printf("|");
			else if(puyo_table[i][j] == -2)printf("-");
			else if(puyo_table[i][j] == 10) printf(" ");
			else{
				color(puyo_table[i][j]);
				printf("@");
			}
		}
		printf("\n");
	}
}

void refresh(int signum){
	if(signum != SIGVTALRM){
		printf("I don't know what to do\n");
	}
	static int downcount = 0;
	static int setcount = 0;
	static long speedcount = 0;
	static int countrange = 5;
	static bool isFirst = true;

	char ch;

	srand((unsigned)time(NULL));

	if(isFirst){
		A_block_num = rand()%5;
		B_block_num = rand()%5;
		isFirst = false;
	}

	printf("\n score : %ld | speed : %d | highscore : %d",point,countrange,best_point);

	display_table();
	check_drop();

	printf("\n game GiveUp : Q");

	if(downcount == countrange - 1){
		point += 1;
		move_block(DOWN);
	}

	if(speedcount == 499){
		if(countrange != 1)
			countrange--;
	}

	downcount++;
	downcount %= countrange;
	speedcount++;
	speedcount %= 500;

	/*
	 * game over condition test here
	 */

	if(isNotColide(DOWN)){
		if(setcount == 9){
			A_block_num = next_A_block_num;
			B_block_num = next_B_block_num;
			next_A_block_num = rand()%5;
			next_B_block_num = rand()%5;
			rotate_state = 0;
			A_position_x = 3;
			A_position_y = 0;
			B_position_x = 3;
			B_position_y = 1;
		}
		setcount++;
		setcount %= 10;
	}

	ch = getch();
	if(ch == -32)
		ch = getch();
	switch(ch){
		case 75:
			move_block(LEFT);
			break;
		case 77:
			move_block(RIGHT);
			break;
		case 80:
			move_block(DOWN);
			break;
		case 90:
		case 112:
			move_block(LROTATE);
			break;
		case 88:
		case 120:
			move_block(RROTATE);
			break;
		case 'q':
		case 'Q': downcount = 0;
			  setcount = 0;
			  countrange = 5;
			  isFirst = 0;
			  game = GAME_END;
			  break;
		default: break;
	}
}

void init_table(void){
	for(int i = 0;i<13;i++){
		for(int j = 1;j<9;j++){
			puyo_table[i][j] = 10;
		}
	}
	for(int i = 0;i<13;i++){
		puyo_table[i][0] = -1;
		puyo_table[i][9] = -1;
	}
	for(int i = 1;i<9;i++){
		puyo_table[12][i] = -2;
	}
	return;
}

bool isNotColide(int command){
	int tempAx, tempAy, tempBx, tempBy;
	int oldAx, oldAy, oldBx, oldBy;
	int tempState;
	char tempTable[13][10];
	oldAx = tempAx = A_position_x;
	oldAy = tempAy = A_position_y;
	oldBx = tempBx = B_position_x;
	oldBy = tempBy = B_position_y;
	tempState = rotate_state;

	switch(command){
		case LEFT: tempAx--; tempBx--;break;
		case RIGHT:tempAx++;tempBx++;break;
		case DOWN: tempAy++;tempBy++;break;
		case LROTATE:if(tempState == 0)tempState = 3;
				     else tempState--;
				     break;
		case RROTATE:tempState = (tempState+1)%4;break;
		default:printf("swomething has wrong in isNotColide");
	}

	for(int i = 0;i<13;i++){
		for(int j  = 0;j<10;j++){
			tempTable[i][j] = puyo_table[i][j];
		}
	}
	tempBx = tempAx + xTable[tempState];
	tempBy = tempAy + yTable[tempState];

	if(puyo_table[tempAy][tempAx] + puyo_table[tempBy][tempBx] == 20) return true;
	return false;
}

int getch(void){//fast character input
             char   ch;
             int   error;
             static struct termios Otty, Ntty;

             fflush(stdout);
             tcgetattr(0, &Otty);
             Ntty = Otty;
             Ntty.c_iflag  =  0;
             Ntty.c_oflag  =  0;
             Ntty.c_lflag &= ~ICANON;
#if 1
            Ntty.c_lflag &= ~ECHO;
#else
            Ntty.c_lflag |=  ECHO;
#endif
            Ntty.c_cc[VMIN]  = CCHAR;
            Ntty.c_cc[VTIME] = CTIME;

#if 1
#define FLAG TCSAFLUSH
#else
#define FLAG TCSANOW
#endif

            if (0 == (error = tcsetattr(0, FLAG, &Ntty))){
	                           error  = read(0, &ch, 1 );
	                           error += tcsetattr(0, FLAG, &Otty);
	                }

            return (error == 1 ? (int) ch : -1 );
}

void search_result(void){
	FILE *fp = NULL;
	char name[30];
	char ch;
	bool isFound = 0;

	fp = fopen("result","rb");

	if(fp == NULL){
		printf("problem opening result file");
		return;
	}

	system("clear");

	printf("\n\n\t\t Text Puyo Puyo");
	printf("\n\t\t\t\t Game record\n\n");
	printf("\n\t\tname\t\tscore\tdate\t\ttime");

	while(1){
		fread(&temp_result,sizeof(struct result),1,fp);
		if(!feof(fp)){
			if(!strcmp(temp_result.name,name)){
				if(!strcmp(temp_result.name,name)){
					isFound = true;
					printf("\n\t=======================================================");
					printf("\n\t\t%s\n\t\t\t\t%ld\t%d. %d. %d.  |  %d : %d\n",
						       	temp_result.name, temp_result.point,
						       	temp_result.year, temp_result.month,
						       	temp_result.day, temp_result.hour,
						       	temp_result.min);
				}
			}
		}
		else break;
	}

	if(isFound == false)
		printf("\n\n\n\t\tThere is no such name");

	printf("\n\n\n\t\tReturn to Game Menu : M");
	while(1){
		ch = getch();
		if(ch == 77 || ch == 109) break;
	}
}

void print_result(void)
{ 
    FILE *fp = NULL; 
    char ch = 1 ;
 
    fp = fopen("result", "rb"); 
 
    if(fp == NULL) 
        return;

   system("clear"); 

   printf("\n\t\t\t\tText Puyo Puyo"); 
   printf("\n\t\t\t\t Game record\n\n"); 
   printf("\n\t\tName\t\tScore\tDate\t\t Tiie"); 

   while(1) { 
              fread(&temp_result, sizeof(struct result), 1, fp); 
              if(!feof(fp)){ 
	                     printf("\n\t========================================================"); 
	                     printf("\n\t\t%s\n\t\t\t\t%ld\t %d. %d. %d.  |  %d : %d\n", temp_result.name, 
					     temp_result.point, temp_result.year, temp_result.month,
					     temp_result.day, temp_result.hour, temp_result.min);
	                 } 
              else break;
      } 

   fclose(fp); 

   printf("\n\n\tReturn to Game Menu : M"); 
   while(1) 
   { 
          ch = getch(); 
          if(ch == 77 || ch == 109)
              break; 
      } 

} 
 
void move_block(int command){
	if(isNotColide(command)){
		switch(command){
			case LEFT : A_position_x--;
				    B_position_x--;
			    	    break;
			case RIGHT:
			   	 A_position_x++;
			    	 B_position_x++;
			    	 break;
			case DOWN:
			   	 A_position_y++;
			    	 B_position_y++;
			   	 break;
			case RROTATE:
				 rotate_state = (rotate_state + 1) % 4;
			    	B_position_x = A_position_x + xTable[rotate_state];
			    	B_position_y = A_position_y + yTable[rotate_state];
			case LROTATE:
				rotate_state = rotate_state == 0 ? 3 : rotate_state - 1;
				B_position_x = A_position_x + xTable[rotate_state];
				B_position_y = A_position_y + yTable[rotate_state];
		}

	}
}
