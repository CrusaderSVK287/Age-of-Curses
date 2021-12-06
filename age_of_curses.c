#define _POSIX_C_SOURCE 200201L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <curses.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define GRASS_C 1
#define GOLD_C 2
#define WATER_C 3
#define TREES_C 4
#define LEAVES_C 5
#define PLAYER_C 6
#define STATUS_C 7
#define ENTRANCE_C 8
#define WWALL_C 9
#define SWALL_C 10
#define GRASSW_C 11
#define LEAVESW_C 12
#define PLAYERW_C 13
#define LOGO_C 14

struct timespec ts = 
{
    .tv_sec = 0,                    // nr of secs
    .tv_nsec = 0.1 * 100000000L  // nr of nanosecs
};

int game(char map[50], const bool debug);
int get_world_info(int *world_y, int *world_x, int *player_y, int *player_x, int *year, int *actual_pop, int *food, int *wood, int *gold, const char map[15], int *offset_y, int *offset_x, int *max_pop, int *age, int *no_lumber, int *no_gold, int *no_farms, int *lumber_around, int *gold_around, int *pop_on_lumber, int *pop_on_farms, int *pop_on_gold, int *lumber_upgrade, int *farming_upgrade, int *mining_upgrade, int *building_upgrade, int *score, int *advancing_stage, int *season_stage, float *market_coeficient, bool *smithy, bool *winter, bool *market, bool *has_castle, bool *won);
int load_world(const int world_y, const int world_x, char world[world_y][world_x], const char map[15]);
void render_world(const int world_y, const int world_x, const char world[world_y][world_x], const int player_y, const int player_x, const int player_facing, const bool leaves_on, const bool winter, const int building, const int offset_y, const int offset_x);
void render_status_line(const int world_y, const int year, const int actual_pop, const int max_pop, const int food, const int wood, const int gold);
int build_menu(int *food, int *wood, int *gold, const int player_y, const int player_x, int *actual_pop, int *max_pop, const int age,const int world_y, const int world_x, char world[world_y][world_x],int *no_lumber, int *no_gold, int *no_farms, int *lumber_around, int *gold_around, bool *smithy, bool *market, bool *has_castle, int *score);
int upgrade_menu(int *food, int *wood, int *gold, int *actual_pop, const int max_pop, const int age, int *lumber_upgrade, int *farming_upgrade, int *mining_upgrade, int *building_upgrade,const bool smithy, int *score);
void pop_management(const int no_lumber, const int no_farms, const int no_gold, const int actual_pop, int *pop_on_wood, int *pop_on_farms, int *pop_on_gold);
void market_menu(int *food, int *wood, int *gold, float *market_coeficient);
int savegame(const char save_file[50], const int world_y,const int world_x, const int player_y, const int player_x, const int year, const int actual_pop, const int food, const int wood, const int gold, const char world[world_y][world_x], const int offset_y, const int offset_x, const int max_pop, const int age, const int no_lumber, const int no_gold, const int no_farms, const int lumber_around, const int gold_around, const int pop_on_lumber, const int pop_on_farms, const int pop_on_gold, const int lumber_upgrade, const int farming_upgrade, const int mining_upgrade, const int building_upgrade, const int score, const int advancing_stage, const int season_stage, const float market_coeficient, const bool smithy, const bool winter, const bool market, const bool has_castle, const bool won);
void hint();
void menu_background();
char overwite_notification();

int main(int argc, char *argv[])
{
	mkdir("data/saved_games", 0777);
	mkdir("data/autosave", 0777);
	bool debug=FALSE;
	srand(time(NULL));
	initscr();	//inicialization of curses library
	if(COLS<134 || LINES<40)	//checks whether the terminal window has the required sizes to play the game
	{
		printf("\nTerminal window is too small, not enough LINES OR COLS, try to move the taskbar or try to go fullscreen\n");
		endwin();
		return EXIT_FAILURE;
	}
	start_color();	//inicialization of colour usage
	cbreak();	//disabled buffering
	noecho();	//no iput is shown on display
	keypad(stdscr,TRUE);	//makes sure the user can utilise function keys
	curs_set(FALSE);	//Visibility of cursor
	nodelay(stdscr, TRUE);	//no enter requiered to get input from keyboard

	init_color(COLOR_RED, 150, 45, 60);	//makes dark brown	(for tree bark)
	init_color(COLOR_MAGENTA, 0, 80, 0);	//makes dark green	(for tree leaves)
	init_color(COLOR_CYAN, 180, 180, 180);	//makes grey (for stone)

	init_pair(GRASS_C, COLOR_GREEN, COLOR_GREEN);
    init_pair(GOLD_C, COLOR_YELLOW, COLOR_YELLOW);
    init_pair(WATER_C, COLOR_BLUE, COLOR_BLUE);
	init_pair(TREES_C, COLOR_RED, COLOR_RED);
	init_pair(LEAVES_C, COLOR_MAGENTA, COLOR_MAGENTA);
	init_pair(PLAYER_C, COLOR_BLACK, COLOR_GREEN);
	init_pair(STATUS_C, COLOR_WHITE, COLOR_RED);
	init_pair(ENTRANCE_C, COLOR_BLACK, COLOR_BLACK);
	init_pair(WWALL_C, COLOR_RED, COLOR_RED);
	init_pair(SWALL_C, COLOR_CYAN, COLOR_CYAN);
	init_pair(GRASSW_C, COLOR_WHITE, COLOR_WHITE);
	init_pair(LEAVESW_C, COLOR_WHITE, COLOR_MAGENTA);
	init_pair(PLAYERW_C, COLOR_BLACK, COLOR_WHITE);
	init_pair(LOGO_C, COLOR_YELLOW, COLOR_RED);
	
	char logo[6][126]={
		{"     ___       _______  _______      ______    _______      ______  __    __   ______          _______  _______     _______ "},
		{"    /   \\     /  _____||   ____|    /  __  \\  |   ____|    /      ||  |  |  | |   _  \\        /       ||   ____|   /       |"},
		{"   /  ^  \\   |  |  __  |  |__      |  |  |  | |  |__      |  ,----'|  |  |  | |  |_)  |      |   (----`|  |__     |   (----`"},
		{"  /  /_\\  \\  |  | |_ | |   __|     |  |  |  | |   __|     |  |     |  |  |  | |      /        \\   \\    |   __|     \\   \\    "},
		{" /  _____  \\ |  |__| | |  |____    |  `--'  | |  |        |  `----.|  `--'  | |  |\\  \\----.----)   |   |  |____.----)   |   "},
		{"/__/     \\__\\ \\______| |_______|    \\______/  |__|         \\______| \\______/  |__| `._____|_______/    |_______|_______/    "}
	};
	if(argc==2)
	{
		if(strcmp("debug",argv[1])==0)
		{
			debug=TRUE;
		}
		else if(strcmp("quick_island",argv[1])==0)
		{
			game("data//worlds//island.txt", debug);
			clear();
		}
		else if(strcmp("quick_forests",argv[1])==0)
		{
			game("data//worlds//forests.txt", debug);
			clear();
		}
		else if(strcmp("quick_rivers",argv[1])==0)
		{
			game("data//worlds//rivers.txt", debug);
			clear();
		}
	}
	int menu_input=0, menu=0, selection=16, tmp, del=1;	//0=main, 1=new_game, 2=saved games
	do
	{
		refresh();
		menu_background();
		attron(COLOR_PAIR(LOGO_C));
		for(int indexy=4;indexy<10;indexy++)
		{
			for(int indexx=COLS/2-63;indexx<COLS/2+63;indexx++)
			{
				mvprintw(indexy,indexx+1,"%c", logo[indexy-3][indexx-COLS/2-63]);
			}
		}
		attroff(COLOR_PAIR(LOGO_C));
		if(debug)
		{
			mvprintw(0,0,"debug mode");
			mvprintw(1,0,"while in game press:");
			mvprintw(2,0,"F3 to bring up debug information");
			mvprintw(3,0,"F4 to get 1000 of each resource");
		}
		attron(COLOR_PAIR(STATUS_C));
		mvprintw(LINES-4, COLS-10,"v1.1.4");	//shows current version of the game
		if(menu==0)	//renders main menu
		{
			mvprintw(selection, COLS/2-8,">               <");
			mvprintw(16, COLS/2-3,"NEW MAP");
			mvprintw(18, COLS/2-6,"LOAD AUTOSAVE");
			mvprintw(20, COLS/2-5,"SAVED GAMES");
			mvprintw(22, COLS/2-5,"HOW TO PLAY");
			mvprintw(24, COLS/2-4,"EXIT GAME");
			menu_input=getch();
			switch (menu_input)
			{
			case KEY_DOWN:
			{
				if(selection<24)
				{
					mvprintw(selection, COLS/2-8,"                  ");
					selection+=2;
				}
				break;
			}
			case KEY_UP:
			{
				if(selection>16)
				{
					mvprintw(selection, COLS/2-8,"                  ");
					selection-=2;
				}
				break;
			}
			case '\n':
			{
				clear();
				if(selection==16)
				{
					menu=1;	//new map
					break;
				}
				if(selection==18)
				{
					if( access("data//autosave//autosave.txt", F_OK)==0)
					{
						game("data//autosave//autosave.txt", debug);	//load autosave
					}
					else
					{
						mvprintw(LINES/2,COLS/2-24,"There is no saved game! Press enter to continue");
						do
						{
							tmp = getch();
						} while (tmp != '\n');
						
					}
					clear();
					break;
				}
				if(selection==20)
				{
					menu=2;
					break;
				}
				if(selection==22)
				{
					hint();
					clear();
					break;
				}
				if(selection==24)
				{
					menu_input=0;
					break;
				}
			}
			default:
				menu_input=1;
				break;
			}
		}
		if(menu==1)
		{
			mvprintw(selection, COLS/2-9,">                  <");
			mvprintw(16, COLS/2-6,"ISLAND (SMALL)");
			mvprintw(18, COLS/2-7,"FORESTS (MEDIUM)");
			mvprintw(20, COLS/2-7,"2 RIVERS (LARGE)");
			mvprintw(22, COLS/2-1,"BACK");
			menu_input=getch();
			switch (menu_input)
			{
				case KEY_DOWN:
				{
					if(selection<22)
					{
						mvprintw(selection, COLS/2-9,"                    ");
						selection+=2;
					}
					break;
				}
				case KEY_UP:
				{
					if(selection>16)
					{
						mvprintw(selection, COLS/2-9,"                    ");
						selection-=2;
					}
					break;
				}
				case '\n':
				{
					clear();
					if(selection==16)
					{
						if(game("data//worlds//island.txt", debug))
						{
							mvprintw(LINES/2,COLS/2-52,"The required file is missing or is damaged, please choose another map or try reinstalling the game files");
							do
							{
								tmp = getch();
							} while (tmp != '\n');
						}
						clear();
						break;
					}
					if(selection==18)
					{
						if(game("data//worlds//forests.txt", debug))
						{
							mvprintw(LINES/2,COLS/2-52,"The required file is missing or is damaged, please choose another map or try reinstalling the game files");
							do
							{
								tmp = getch();
							} while (tmp != '\n');
						}
						clear();
						break;
					}
					if(selection==20)
					{
						if(game("data//worlds//rivers.txt", debug))
						{
							mvprintw(LINES/2,COLS/2-52,"The required file is missing or is damaged, please choose another map or try reinstalling the game files");
							do
							{
								tmp = getch();
							} while (tmp != '\n');
						}
						clear();
						break;
					}
					if(selection==22)
					{
						menu=0;
						break;
					}
				}
				default:
				menu_input=1;
				break;
			}
		}
		if(menu==2)
		{
			mvprintw(selection, COLS/2-9,">                  <");
			mvprintw(16, COLS/2-5,"SAVED GAME 1");
			mvprintw(18, COLS/2-5,"SAVED GAME 2");
			mvprintw(20, COLS/2-5,"SAVED GAME 3");
			mvprintw(22, COLS/2-5,"SAVED GAME 4");
			mvprintw(24, COLS/2-5,"SAVED GAME 5");
			mvprintw(26, COLS/2-1,"BACK");
			mvprintw(30, COLS/2-14,"PRESS ""D"" TO DELETE A SAVED GAME");
			menu_input=getch();
			switch (menu_input)
			{
				case KEY_DOWN:
				{
					if(selection<26)
					{
						mvprintw(selection, COLS/2-9,"                    ");
						selection+=2;
					}
					break;
				}
				case KEY_UP:
				{
					if(selection>16)
					{
						mvprintw(selection, COLS/2-9,"                    ");
						selection-=2;
					}
					break;
				}
				case '\n':
				{
					if(selection==26)
					{
						selection=24;
						menu=0;
						clear();
						break;
					}
					if(selection==16)
					{
						if(game("data//saved_games//game_1.txt", debug))
						{
							clear();
							mvprintw(LINES/2,COLS/2-23,"There is no game saved on this slot, try another one");
							do
							{
								tmp = getch();
							} while (tmp != '\n');
						}
						clear();
						break;
					}
					if(selection==18)
					{
						if(game("data//saved_games//game_2.txt", debug))
						{
							clear();
							mvprintw(LINES/2,COLS/2-23,"There is no game saved on this slot, try another one");
							do
							{
								tmp = getch();
							} while (tmp != '\n');
						}
						clear();
						break;
					}
					if(selection==20)
					{
						if(game("data//saved_games//game_3.txt", debug))
						{
							clear();
							mvprintw(LINES/2,COLS/2-23,"There is no game saved on this slot, try another one");
							do
							{
								tmp = getch();
							} while (tmp != '\n');
						}
						clear();
						break;
					}
					if(selection==22)
					{
						if(game("data//saved_games//game_4.txt", debug))
						{
							clear();
							mvprintw(LINES/2,COLS/2-23,"There is no game saved on this slot, try another one");
							do
							{
								tmp = getch();
							} while (tmp != '\n');
						}
						clear();
						break;
					}
					if(selection==24)
					{
						if(game("data//saved_games//game_5.txt", debug))
						{
							clear();
							mvprintw(LINES/2,COLS/2-23,"There is no game saved on this slot, try another one");
							do
							{
								tmp = getch();
							} while (tmp != '\n');
						}
						clear();
						break;
					}
				}
				case 'D': case 'd':
				{
					clear();
					mvprintw(LINES/2,COLS/2-38,"Are you sure you want to delete this saved game? You cannot get it back Y/N");
					do
					{
						tmp = getch();
					} while (tmp != 'Y' && tmp!='y' && tmp!='N' && tmp!='n');
					if(tmp=='Y' || tmp=='y')
					{
						if(selection==16)
						{
							del=remove("data//saved_games//game_1.txt");
						}
						else if(selection==18)
						{
							del=remove("data//saved_games//game_2.txt");
						}
						else if(selection==20)
						{
							del=remove("data//saved_games//game_3.txt");
						}
						else if(selection==22)
						{
							del=remove("data//saved_games//game_4.txt");
						}
						else if(selection==24)
						{
							del=remove("data//saved_games//game_5.txt");
						}
					}
				}
				default:
				{
					break;
				}
			}
			if(del==0)
			{
				clear();
				mvprintw(LINES/2,COLS/2-23,"Saved game removed succesfully, press enter to continue");
				do
				{
					tmp = getch();
				} while (tmp != '\n');
			}
			else if(del==-1)
			{
				clear();
				mvprintw(LINES/2,COLS/2-30,"There is no game saved on this slot, press enter to continue");
				do
				{
					tmp = getch();
				} while (tmp != '\n');
			}
			del=1;
		}
		attroff(COLOR_PAIR(STATUS_C));
	}while(menu_input!=0);
	endwin();
	return EXIT_SUCCESS;
}

int game(char map[50], const bool debug)
{
	clear();
	int world_y=0, world_x=0, player_y=0, player_x=0, offset_y=0, offset_x=0, player_facing=0, error=0, error_stage=0, game_end=0, rng=0;
	int wood=0,gold=0,food=0,max_pop=0,actual_pop=0,year=1000, age=1;
	int no_lumber=0,no_gold=0,no_farms=0,lumber_around=0,gold_around=0,pop_on_lumber=0,pop_on_farms=0,pop_on_gold=0;
	int lumber_upgrade=1, farming_upgrade=1, mining_upgrade=1, building_upgrade=1, score=0;
	int input=0, tmp=0, year_stage=0, gathering_stage=0, advancing_stage=0, season_stage=0, savegame_input=10;
	float market_coeficient=1.2;
	bool leaves_on=TRUE, smithy=FALSE, winter=FALSE, market=FALSE, debug_info=FALSE, has_castle=FALSE, won=FALSE, show_score=FALSE, new_game=FALSE;
	if(strcmp(map,"data//worlds//rivers.txt")==0||strcmp(map,"data//worlds//forests.txt")==0||strcmp(map,"data//worlds//island.txt")==0)
	{
		new_game=TRUE;
	}
	if(access(map, F_OK)!=0)
	{
		return 1;
	}
	get_world_info(&world_y, &world_x, &player_y, &player_x, &year, &actual_pop, &food, &wood, &gold, map, &offset_y, &offset_x,&max_pop,&age,&no_lumber, &no_gold,&no_farms,&lumber_around,&gold_around,&pop_on_lumber,&pop_on_farms,&pop_on_gold,&lumber_upgrade,&farming_upgrade,&mining_upgrade,&building_upgrade,&score,&advancing_stage,&season_stage,&market_coeficient,&smithy,&winter,&market,&has_castle,&won);
	char world[world_y][world_x];
	if(load_world(world_y, world_x, world, map)==1)
	{
		return EXIT_FAILURE;
	}
	do	//main game loop
	{
		nanosleep(&ts, NULL);
		render_world(world_y,world_x,world,player_y,player_x,player_facing,leaves_on, winter, building_upgrade, offset_y, offset_x);
		render_status_line(world_y,year, actual_pop,max_pop,food,wood,gold);
		if(advancing_stage>=1)	//makes advancing take some time and displays the current % of progress
		{
			attron(COLOR_PAIR(STATUS_C));
			advancing_stage++;
			mvprintw(LINES/2-16, COLS/2-10, "ADVANCING %d%%", advancing_stage/20);
			if(advancing_stage==2000)
			{
				advancing_stage=0;
				age++;
				error=13;
			}
			attroff(COLOR_PAIR(STATUS_C));
		}
		if(debug_info && debug)	//prints information usefull for debugging if enabled
		{
			mvprintw(0,0,"OFFSET Y/X: %d %d", offset_y,offset_x);
			mvprintw(1,0,"PLAYER_POSITION Y/X: %d %d", player_y, player_x);
			mvprintw(2,0,"YEAR_STAGE, GATHERING_STAGE, SEASON_STAGE:%d %d %d", year_stage, gathering_stage, season_stage);
			mvprintw(3,0,"LUMBER AROUND, GOLD AROUND: %d %d", lumber_around, gold_around);
			mvprintw(4,0,"AGE, ADVANCING_STAGE: %d %d", age, advancing_stage);
			mvprintw(5,0,"MARKET_COEFICIENT: %f", market_coeficient);
			mvprintw(6,0,"WINTER: %s",(winter)?"TRUE":"FALSE");
			mvprintw(7,0,"SMITHY: %s",(smithy)?"TRUE":"FALSE");
			mvprintw(8,0,"MARKET: %s",(market)?"TRUE":"FALSE");
			mvprintw(9,0,"HAS_CASTLE: %s",(has_castle)?"TRUE":"FALSE");
			mvprintw(10,0,"VICTORY: %s",(won)?"TRUE":"FALSE");
			mvprintw(11,0,"MAP_PATH: %s", map);
		}
		if(show_score)
		{
			attron(COLOR_PAIR(STATUS_C));
			mvprintw(LINES/2+18,1,"SCORE: %d", score+food+wood+gold*3+actual_pop*50);
			attroff(COLOR_PAIR(STATUS_C));
		}
		if(error!=0)	//updating and showing of errors if there are any present
		{
			error_stage++;
			if(error_stage==199)
			{
				error=0, error_stage=0;
			}
			attron(COLOR_PAIR(STATUS_C));
			if(error==1)
			{
				mvprintw(LINES/2, COLS/2-19,"NOT ENOUGH RESOUCES OR WRONG POSITION");
			}
			else if(error==2)
			{
				mvprintw(LINES/2, COLS/2-19,"NOT ENOUGH RESOUCES OR MAX POPULATION");
			}
			else if(error==3)
			{
				mvprintw(LINES/2, COLS/2-9,"ALREADY RESEARCHED");
			}
			else if(error==4)
			{
				mvprintw(LINES/2, COLS/2-9,"YOU NEED A SMITHY");
			}
			else if(error==5)
			{
				mvprintw(LINES/2, COLS/2-9,"YOU NEED A MARKET!");
			}
			else if(error==6)
			{
				mvprintw(LINES/2, COLS/2-9,"NOT YET AVAILABLE");
			}
			else if(error==7)
			{
				mvprintw(LINES/2, COLS/2-18,"CANNOT DELETE TREES, GOLD NOR WATER");
			}
			else if(error==8)
			{
				mvprintw(LINES/2, COLS/2-17,"SOMETHING IS OBSTRUCTING PLACEMENT");
			}
			else if(error==9)
			{
				mvprintw(LINES/2, COLS/2-30,"YOU CANNOT HAVE 2 BUILDINGS OF THIS TYPE CLOSE TO EACH OTHER");
			}
			else if(error==10)
			{
				mvprintw(LINES/2, COLS/2-15,"YOU NEED 500 FOOD TO ADVANCE");
			}
			else if(error==11)
			{
				mvprintw(LINES/2, COLS/2-21,"YOU NEED 800 FOOD AND 300 GOLD TO ADVANCE");
			}
			else if(error==12)
			{
				mvprintw(LINES/2, COLS/2-12,"YOU ARE IN THE LAST AGE");
			}
			else if(error==13)
			{
				mvprintw(LINES/2, COLS/2-14,"YOU HAVE ENTERED THE %d. AGE",age);
			}
			else if(error==14)
			{
				mvprintw(LINES/2, COLS/2-13,"YOU ARE ALREADY ADVANCING");
			}
			else if(error==20)
			{
				mvprintw(LINES/2, COLS/2-9,"WINTER IS NEAR...");
			}
			else if(error==30)
			{
				mvprintw(LINES/2, COLS/2-13,"A CITIZEN DIED OF HUNGER!");
			}
			else if(error==31)
			{
				mvprintw(LINES/2, COLS/2-13,"A CITIZEN FROZE TO DEATH!");
			}
			attroff(COLOR_PAIR(STATUS_C));
		}
		refresh();
		input = getch();
		switch (input)
		{
			case 'q': case 'Q':
			{
				attron(COLOR_PAIR(STATUS_C));
				mvprintw(LINES/2, COLS/2-10,"PRESS ENTER TO QUIT");
				attroff(COLOR_PAIR(STATUS_C));
				refresh();
				game_end=2;
				break;
			}
			case 'p': case 'P':
			{
				attron(COLOR_PAIR(STATUS_C));
				mvprintw(LINES/2, COLS/2-17,"PAUSED... PRESS P AGAIN TO RESUME");
				attroff(COLOR_PAIR(STATUS_C));
				refresh();
				do
				{
					tmp=getch();
				}while (tmp!='p' && tmp!='P');
				break;
			}
			case 's': case 'S':
			{
				int selection=-7;
				char choice='X';
				bool saved=FALSE;
				do
				{
					attron(COLOR_PAIR(TREES_C));
					for(int indexy=LINES/2-10;indexy<=LINES/2+5;indexy++)
					{
						mvprintw(indexy, COLS/2-25, "########################################");
					}
					attroff(COLOR_PAIR(TREES_C));
					attron(COLOR_PAIR(STATUS_C));
					mvprintw(LINES/2-4,COLS/2-17,"                  ");
					mvprintw(LINES/2-2,COLS/2-17,"                  ");
					mvprintw(LINES/2+selection,COLS/2-14,">                <");
					mvprintw(LINES/2-9, COLS/2-15,"CHOOSE SAVE GAME SLOT");
					mvprintw(LINES/2-7, COLS/2-11,"SAVED GAME 1");
					mvprintw(LINES/2-5, COLS/2-11,"SAVED GAME 2");
					mvprintw(LINES/2-3, COLS/2-11,"SAVED GAME 3");
					mvprintw(LINES/2-1, COLS/2-11,"SAVED GAME 4");
					mvprintw(LINES/2+1, COLS/2-11,"SAVED GAME 5");
					mvprintw(LINES/2+4, COLS/2-13,"PRESS 0 TO EXIT");
					refresh();
					savegame_input=getch();
					switch (savegame_input)
					{
						case KEY_DOWN:
						{
							if(selection<0)
							{
								selection+=2;
							}
							break;
						}
						case KEY_UP:
						{
							if(selection>-6)
							{
								selection-=2;
							}
							break;
						}
						case '\n':
						{
							saved=TRUE;
							if(selection==-7)
							{
								if(access("data//saved_games//game_1.txt", F_OK) == 0 && new_game==FALSE)
								{
									choice=overwite_notification();
									if(choice=='N' || choice=='n')
									{
										break;
									}
								}
								new_game=FALSE;
								map="data//saved_games//game_1.txt";
								savegame("data//saved_games//game_1.txt", world_y,world_x,player_y,player_x,year,actual_pop,food,wood,gold, world, offset_y, offset_x,max_pop,age,no_lumber, no_gold,no_farms,lumber_around,gold_around,pop_on_lumber,pop_on_farms,pop_on_gold,lumber_upgrade,farming_upgrade,mining_upgrade,building_upgrade,score,advancing_stage,season_stage,market_coeficient,smithy,winter,market,has_castle,won);
								break;
							}
							else if(selection==-5)
							{
								if(access("data//saved_games//game_2.txt", F_OK) == 0 && new_game==FALSE)
								{
									choice=overwite_notification();
									if(choice=='N' || choice=='n')
									{
										break;
									}
								}
								new_game=FALSE;
								map="data//saved_games//game_2.txt";
								savegame("data//saved_games//game_2.txt", world_y,world_x,player_y,player_x,year,actual_pop,food,wood,gold, world, offset_y, offset_x,max_pop,age,no_lumber, no_gold,no_farms,lumber_around,gold_around,pop_on_lumber,pop_on_farms,pop_on_gold,lumber_upgrade,farming_upgrade,mining_upgrade,building_upgrade,score,advancing_stage,season_stage,market_coeficient,smithy,winter,market,has_castle,won);
								break;
							}
							else if(selection==-3)
							{
								if(access("data//saved_games//game_3.txt", F_OK) == 0 && new_game==FALSE)
								{
									choice=overwite_notification();
									if(choice=='N' || choice=='n')
									{
										break;
									}
								}
								new_game=FALSE;
								map="data//saved_games//game_3.txt";
								savegame("data//saved_games//game_3.txt", world_y,world_x,player_y,player_x,year,actual_pop,food,wood,gold, world, offset_y, offset_x,max_pop,age,no_lumber, no_gold,no_farms,lumber_around,gold_around,pop_on_lumber,pop_on_farms,pop_on_gold,lumber_upgrade,farming_upgrade,mining_upgrade,building_upgrade,score,advancing_stage,season_stage,market_coeficient,smithy,winter,market,has_castle,won);
								break;
							}
							else if(selection==-1)
							{
								if(access("data//saved_games//game_4.txt", F_OK) == 0 && new_game==FALSE)
								{
									choice=overwite_notification();
									if(choice=='N' || choice=='n')
									{
										break;
									}
								}
								new_game=FALSE;
								map="data//saved_games//game_4.txt";
								savegame("data//saved_games//game_4.txt", world_y,world_x,player_y,player_x,year,actual_pop,food,wood,gold, world, offset_y, offset_x,max_pop,age,no_lumber, no_gold,no_farms,lumber_around,gold_around,pop_on_lumber,pop_on_farms,pop_on_gold,lumber_upgrade,farming_upgrade,mining_upgrade,building_upgrade,score,advancing_stage,season_stage,market_coeficient,smithy,winter,market,has_castle,won);
								break;
							}
							else if(selection==1)
							{
								if(access("data//saved_games//game_5.txt", F_OK) == 0 && new_game==FALSE)
								{
									choice=overwite_notification();
									if(choice=='N' || choice=='n')
									{
										break;
									}
								}
								new_game=FALSE;
								map="data//saved_games//game_5.txt";
								savegame("data//saved_games//game_5.txt", world_y,world_x,player_y,player_x,year,actual_pop,food,wood,gold, world, offset_y, offset_x,max_pop,age,no_lumber, no_gold,no_farms,lumber_around,gold_around,pop_on_lumber,pop_on_farms,pop_on_gold,lumber_upgrade,farming_upgrade,mining_upgrade,building_upgrade,score,advancing_stage,season_stage,market_coeficient,smithy,winter,market,has_castle,won);
								break;
							}
							break;
						}
						case '0':
						{
							break;
						}	
						default:
						{
							break;
						}
					}
				}while(savegame_input!='0' && saved==FALSE);
				break;
			}
			case 'b': case 'B':
			{
				error = build_menu(&food,&wood,&gold,player_y,player_x,&actual_pop,&max_pop,age,world_y,world_x,world, &no_lumber, &no_gold, &no_farms, &lumber_around, &gold_around, &smithy, &market, &has_castle, &score);
				break;
			}
			case 'n': case 'N':
			{
				error = upgrade_menu(&food,&wood,&gold,&actual_pop,max_pop,age,&lumber_upgrade,&farming_upgrade,&mining_upgrade,&building_upgrade, smithy, &score);
				break;
			}
			case 'v': case 'V':
			{
				pop_management(no_lumber,no_farms,no_gold,actual_pop,&pop_on_lumber,&pop_on_farms,&pop_on_gold);
				break;
			}
			case 'm': case 'M':
			{
				(market) ? market_menu(&food,&wood,&gold,&market_coeficient) : (error=5);
				break;
			}
			case 'D':
			{
				if(world[player_y-1][player_x]!='G' && world[player_y-1][player_x]!='T' && world[player_y-1][player_x]!='W' && player_y>0)
				{
					if(world[player_y-1][player_x]=='L' || world[player_y][player_x]=='L')
					{
						int correction_condition = (world[player_y][player_x]=='L') ? 5:4;
						int correction_initial = (world[player_y][player_x]=='L') ? 5:6;
						for(int indexy=player_y-correction_initial;indexy<player_y+correction_condition;indexy++)
						{
							for(int indexx=player_x-10;indexx<player_x+10;indexx++)
							{
								if(world[indexy][indexx]=='T')
								{
									lumber_around--;
								}
							}
						}
					}
					if(world[player_y-1][player_x]=='M' || world[player_y][player_x]=='M')
					{
						int correction_condition = (world[player_y][player_x]=='M') ? 5:4;
						int correction_initial = (world[player_y][player_x]=='M') ? 5:6;
						for(int indexy=player_y-correction_initial;indexy<player_y+correction_condition;indexy++)
						{
							for(int indexx=player_x-10;indexx<player_x+10;indexx++)
							{
								if(world[indexy][indexx]=='G')
								{
									gold_around--;
								}
							}
						}
					}
					if(world[player_y-1][player_x]=='H' || world[player_y][player_x]=='H')
					{
						max_pop-=3;
					}
					world[player_y-1][player_x]='E';
					world[player_y][player_x]='E';
				}
				else
				{
					error=7;
				}
				break;
			}
			case 'a': case 'A':
			{
				if(advancing_stage!=0)
				{
					error=14;
				}
				else if(age==1 && food>=500 && advancing_stage==0)
				{
					advancing_stage=1;
					food-=500;
				}
				else if(age==1)
				{
					error=10;
				}
				else if(age==2 && food>=800 && gold >=300 && advancing_stage==0)
				{
					advancing_stage=1;
					food-=800;
					gold-=300;
				}
				else if(age==2)
				{
					error=11;
				}
				else if(age==3)
				{
					error=12;
				}
				break;
			}
			case KEY_UP:
			{
				if(world[player_y-1][player_x]=='E' && player_y>0)
				{
					player_y--;
					if(player_y%40==0 && player_y>0)
					{
						offset_y-=40;
						player_y--;
					}
				}
				player_facing=0;
				break;
			}
			case KEY_RIGHT:
			{
				if(world[player_y][player_x+1]=='E' && player_x<world_x-1)
				{
					player_x++;
					if(player_x%120==0)
					{
						offset_x+=120;
						player_x++;
					}
				}
				player_facing=1;
				break;
			}
			case KEY_DOWN:
			{
				if(world[player_y+1][player_x]=='E' && player_y<world_y)
				{
					player_y++;
					if(player_y%40==0)
					{
						offset_y+=40;
						player_y++;
					}
				}
				player_facing=2;
				break;
			}
			case KEY_LEFT:
			{
				if(world[player_y][player_x-1]=='E' && player_x>0)
				{
					player_x--;
					if(player_x%120==0 && player_x>0)
					{
						offset_x-=120;
						player_x--;
					}
				}
				player_facing=3;
				break;
			}
			case KEY_F(1):
			{
				leaves_on=  (leaves_on) ? FALSE : TRUE;
				break;
			}
			case KEY_F(2):
			{
				show_score= (show_score) ? FALSE : TRUE;
				break;
			}
			case KEY_F(3):
			{
				debug_info= (debug_info) ? FALSE : TRUE;
				break;
			}
			case KEY_F(4):
			{
				if(debug)
				{
					food+=1000;
					wood+=1000;
					gold+=1000;
				}
				break;
			}
			default:
			{
				//mvprintw(20,20,"NOT WORKING!");	//used for debugging
				break;
			}
		}
		year_stage++;
		if(year_stage==500)	//updating years and actions done at the end of the year such as consuming food/wood by pops
		{
			(won)? score=score:(score+=10);
			year++;
			year_stage=0;
			season_stage++;
			if(winter)
			{
				wood-=5*(max_pop/3);
				if(wood <= 0 || max_pop<actual_pop)
				{
					if(wood<=0)
					{
						wood=0;
					}
					if(actual_pop>0)
					{
						score-=30;
						actual_pop--;
						tmp=0;
						do
						{
							rng=rand()%3;
							if(rng==0 && pop_on_farms>0)
							{
								pop_on_farms--;
								tmp+=1;
							}
							else if(rng==1 && pop_on_lumber>0)
							{
								pop_on_lumber--;
								tmp+=1;
							}
							else if(rng==2 && pop_on_gold>0)
							{
								pop_on_gold--;
								tmp+=1;
							}
							else if(pop_on_gold==0 && pop_on_farms==0 && pop_on_lumber==0)
							{
								tmp+=1;
							}
						}while(tmp!=1);
					error=31;
					}
				}
			}
			if(season_stage==25)
			{
				error=20;
			}
			if(food>=actual_pop)
			{
				food-=actual_pop;
			}
			else if(food<=actual_pop)
			{
				food=0;
			}
			if(actual_pop>0 && food<=0)
			{
				if(rand()%2==1)
				{
					score-=30;
					actual_pop--;
					tmp=0;
					do
					{
						rng=rand()%3;
						if(rng==0 && pop_on_farms>0)
						{
							pop_on_farms--;
							tmp+=1;
						}
						else if(rng==1 && pop_on_lumber>0)
						{
							pop_on_lumber--;
							tmp+=1;
						}
						else if(rng==2 && pop_on_gold>0)
						{
							pop_on_gold--;
							tmp+=1;
						}
						else if(pop_on_gold==0 && pop_on_farms==0 && pop_on_lumber==0)
						{
							tmp+=1;
						}
					}while(tmp!=1);
					error=30;
				}
			}
			if(year%5==0 && market_coeficient>1.25)
			{
				market_coeficient-=0.1;
			}
		}
		if(pop_on_farms > 0 || pop_on_gold >0 || pop_on_lumber>0)	//update of gathering resources if there is at least one worker on any resource (to reduce unnnecesery load)
		{
			gathering_stage++;
			if(gathering_stage==250)
			{
				gathering_stage=0;
				if(pop_on_farms>0 && !winter)
				{
					food+=pop_on_farms*4*farming_upgrade;
				}
				if(pop_on_lumber>0)
				{
					wood+=(((lumber_around/no_lumber)*pop_on_lumber)/6)*lumber_upgrade;
				}
				if(pop_on_gold>0)
				{
					gold+=(((gold_around/no_gold)*pop_on_gold)/2)*mining_upgrade;
				}
			}
		}
		if(season_stage == ((winter==TRUE) ? 10 : 30))	// makes seasons take different amount of time
		{
			winter = (winter) ? FALSE : TRUE;
			season_stage=0;
			if(!winter)
			{
				savegame("data//autosave//autosave.txt",world_y,world_x,player_y,player_x,year,actual_pop,food,wood,gold, world, offset_y, offset_x,max_pop,age,no_lumber, no_gold,no_farms,lumber_around,gold_around,pop_on_lumber,pop_on_farms,pop_on_gold,lumber_upgrade,farming_upgrade,mining_upgrade,building_upgrade,score,advancing_stage,season_stage,market_coeficient,smithy,winter,market,has_castle,won);
			}
		}
		if(actual_pop<=0 && food<50 && !market)	//ends game due to no workers
		{
			game_end=2;
			render_status_line(world_y,year, actual_pop,max_pop,food,wood,gold);
			mvprintw(LINES/2, COLS/2-45,"GAME OVER, ALL YOUR WORKERS DIED AND YOU CANNOT PRODUCE NEW ONES, PRESS ENTER TO CONTINUE");
		}
		if(year==1444)	//ends game due to running out of time
		{
			game_end=2;
			render_status_line(world_y,year, actual_pop,max_pop,food,wood,gold);
			score=score+food+wood+gold*3+actual_pop*50;
			attron(COLOR_PAIR(STATUS_C));
			mvprintw(LINES/2-2, COLS/2-35,"                                                                       ");
			mvprintw(LINES/2-1, COLS/2-35,"                                                                       ");
			mvprintw(LINES/2-1, COLS/2-35,"                           YOUR SCORE: %d                              ", score);
			mvprintw(LINES/2, COLS/2-35," GAME OVER, NEW ERA DAWNS... ERA OF GUNPOWDER, PRESS ENTER TO CONTINUE ");
			mvprintw(LINES/2+1, COLS/2-35,"                                                                       ");
			attroff(COLOR_PAIR(STATUS_C));
			refresh();
			won=TRUE;
		}
		else if(actual_pop>49 && building_upgrade==2 && has_castle && !won)
		{
			won=TRUE;
			score=score+food+wood+gold*3+actual_pop*50+(1444-year)*50;
			attron(COLOR_PAIR(STATUS_C));
			mvprintw(LINES/2-2, COLS/2-25,"                                              ");
			mvprintw(LINES/2-1, COLS/2-25,"                                              ");
			mvprintw(LINES/2-1, COLS/2-25," GAME OVER, YOUR SCORE: %d                    ", score);
			mvprintw(LINES/2, COLS/2-25," PRESS ENTER TO CONTINUE PLAYING OR Q TO QUIT ");
			mvprintw(LINES/2+1, COLS/2-25,"                                              ");
			attroff(COLOR_PAIR(STATUS_C));
			refresh();
			do
			{
				input=getch();
			}while(input!='Q' && input !='q' && input !='\n');
			game_end=(input=='Q' || input=='q') ? 1:0;
		}
	}while(game_end==0);
	attron(COLOR_PAIR(STATUS_C));
	if(game_end!=2)
	{
		mvprintw(18, COLS/2-25," GAME OVER, YOUR SCORE: %d", score);
	}
	refresh();
	if(game_end!=1)
	{
		do
		{
			input=getch();
		}while (input!='\n');
	}
	if(strcmp("data//worlds//island.txt", map)==0 ||strcmp("data//worlds//forests.txt", map)==0 ||strcmp("data//worlds//rivers.txt", map)==0)
	{
		savegame("data//autosave//autosave.txt",world_y,world_x,player_y,player_x,year,actual_pop,food,wood,gold, world, offset_y, offset_x,max_pop,age,no_lumber, no_gold,no_farms,lumber_around,gold_around,pop_on_lumber,pop_on_farms,pop_on_gold,lumber_upgrade,farming_upgrade,mining_upgrade,building_upgrade,score,advancing_stage,season_stage,market_coeficient,smithy,winter,market,has_castle,won);
	}
	else
	{
		savegame(map,world_y,world_x,player_y,player_x,year,actual_pop,food,wood,gold, world, offset_y, offset_x,max_pop,age,no_lumber, no_gold,no_farms,lumber_around,gold_around,pop_on_lumber,pop_on_farms,pop_on_gold,lumber_upgrade,farming_upgrade,mining_upgrade,building_upgrade,score,advancing_stage,season_stage,market_coeficient,smithy,winter,market,has_castle,won);
	}
	attroff(COLOR_PAIR(STATUS_C));
	return EXIT_SUCCESS;
}

int load_world(const int world_y, const int world_x, char world[world_y][world_x], const char map[15])
{
	int y=0,x=0;
	FILE *world_load;
	world_load = fopen(map, "r");
	char tmp;
	attron(COLOR_PAIR(STATUS_C));
	for(int indexy=0;indexy<world_y;indexy++)
	{
		for(int indexx=0;indexx<world_x;indexx++)
		{
			world[indexy][indexx]='E';
		}
		mvprintw(LINES/2-1,COLS/2-13, "                        ");
		mvprintw(LINES/2,COLS/2-13, "                        ");
		mvprintw(LINES/2,COLS/2-13, "     LOADING... %.2f%%",indexy/(float)world_y*100);
		mvprintw(LINES/2+1,COLS/2-13, "                        ");
		refresh();
		nanosleep(&ts, NULL);
	}
	attroff(COLOR_PAIR(STATUS_C));
	int c;
	fscanf(world_load, "%*[^\n]");	//to skip the first line containing information about world and resources
	while((c=getc(world_load))!=EOF)
	{
		fscanf(world_load, "%d %d", &y, &x);
		fscanf(world_load, "%c", &tmp);
		//printf("%c ",tmp);	//used for debugging
		if(y > world_y || x > world_x)
		{
				endwin();
				printf("\nCritical error generating world, trees/gold/water outside of world borders, exiting now...\n");
				return 1;
		}
		switch (tmp)
		{
			case 'H':
			{
				world[y][x]='H';
				break;
			}
			case 'P':
			{
				world[y][x]='P';
				break;
			}
			case 'I':
			{
				world[y][x]='I';
				break;
			}
			case 'L':
			{
				world[y][x]='L';
				break;
			}
			case 'V':
			{
				world[y][x]='V';
				break;
			}
			case 'M':
			{
				world[y][x]='M';
				break;
			}
			case 'S':
			{
				world[y][x]='S';
				break;
			}
			case 'C':
			{
				world[y][x]='C';
				break;
			}
			case 'N':
			{
				world[y][x]='N';
				break;
			}
			case 'O':
			{
				world[y][x]='O';
				break;
			}
			case 'T':
			{
				world[y][x]='T';
				break;
			}
			case 'f':
			{
				int forest_radius=0,number_of_trees=0;
				fscanf(world_load,"%d",&forest_radius);
				number_of_trees = forest_radius*forest_radius;
				do
				{
					for(int i=0;i<forest_radius*2;i++)
					{
						for(int z=0;z<forest_radius*2;z++)
						{	
							if(rand()%4 == 0 && number_of_trees>0)
							{
								world[i+y-forest_radius][z+x-forest_radius]='T';
								number_of_trees--;
							}
						}
					}
				}while(number_of_trees!=0);
				break;
			}
			case 'G':
			{
				world[y][x]='G';
				break;
			}
			case 'g':
			{
				for(int i=0;i<3;i++)
				{
					for(int z=0;z<5;z++)
					{
						if(i==0 && z==0)
						{continue;}
						if(i==2 && z==0)
						{continue;}
						if(i==0 && z==4)
						{continue;}
						if(i==2 && z==4)
						{continue;}
						world[i+y-2][z+x-2]='G';
					}
				}
				break;
			}
			case 'w':
			{
				int radius;
				fscanf(world_load, "%d", &radius);
				for(int i=0;i<radius;i++)
				{
					for(int z=0;z<radius;z++)
					{
						if(i==0 && z==0)
						{continue;}
						if(i==radius-1 && z==0)
						{continue;}
						if(i==0 && z==radius-1)
						{continue;}
						if(i==radius-1 && z==radius-1)
						{continue;}
						world[i+y-(radius/2)][z+x-(radius/2)]='W';
					}
				}
				break;
			}
			case 'W':
			{
				world[y][x]='W';
				break;
			}
			default:
			{
				endwin();
				printf("\nCritical error reading file, exiting now...\n");
				return 1;
			}
		}
	}
	fclose(world_load);
	return 0;
}

int get_world_info(int *world_y, int *world_x, int *player_y, int *player_x, int *year, int *actual_pop, int *food, int *wood, int *gold, const char map[15], int *offset_y, int *offset_x, int *max_pop, int *age, int *no_lumber, int *no_gold, int *no_farms, int *lumber_around, int *gold_around, int *pop_on_lumber, int *pop_on_farms, int *pop_on_gold, int *lumber_upgrade, int *farming_upgrade, int *mining_upgrade, int *building_upgrade, int *score, int *advancing_stage, int *season_stage, float *market_coeficient, bool *smithy, bool *winter, bool *market, bool *has_castle, bool *won)
{
	FILE *world_load;
	int smithy_i=0, winter_i=0, market_i=0, has_castle_i=0, won_i=0;
	world_load = fopen(map, "r");
	fscanf(world_load, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %f %d %d %d %d %d ", world_y, world_x, player_y, player_x, year, actual_pop, food, wood, gold, offset_y, offset_x, max_pop, age, no_lumber, no_gold, no_farms, lumber_around, gold_around, pop_on_lumber, pop_on_farms, pop_on_gold, lumber_upgrade, farming_upgrade, mining_upgrade, building_upgrade, score,advancing_stage, season_stage, market_coeficient, &smithy_i, &winter_i, &market_i, &has_castle_i, &won_i);
	*smithy=smithy_i;
	*winter=winter_i;
	*market=market_i;
	*has_castle=has_castle_i;
	*won=won_i;
	fclose(world_load);
	return 0;
}

void render_world(const int world_y, const int world_x, const char world[world_y][world_x], const int player_y, const int player_x, const int player_facing,const bool leaves_on, const bool winter, const int building, const int offset_y, const int offset_x)
{
	for(int indexy=0,world_indexy=0+offset_y ; indexy<40 ; indexy++, world_indexy++)
	{
		indexy+=LINES/2-20;
		for(int indexx=0, world_indexx=0+offset_x ; indexx<120 ; indexx++,world_indexx++)
		{
			if(world[world_indexy][world_indexx]=='E')
			{
				(winter) ? attron(COLOR_PAIR(GRASSW_C)) : attron(COLOR_PAIR(GRASS_C));
			}
			else if(world[world_indexy][world_indexx]=='I')
			{
				continue;
			}
			else if(world[world_indexy][world_indexx]=='G')
			{
				attron(COLOR_PAIR(GOLD_C));
			}
			else if(world[world_indexy][world_indexx]=='W')
			{
				attron(COLOR_PAIR(WATER_C));
			}
			else if(world[world_indexy][world_indexx]=='V')
			{
				(building == 2) ? attron(COLOR_PAIR(SWALL_C)) : attron(COLOR_PAIR(WWALL_C));
			}
			else if(world[world_indexy][world_indexx]=='O')
			{
				attron(COLOR_PAIR(SWALL_C));
				mvprintw(indexy-1,indexx,"#");
				mvprintw(indexy-2,indexx,"#");
				mvprintw(indexy-3,indexx,"#");
				mvprintw(indexy-4,indexx-1,"###");
				mvprintw(indexy-6,indexx,"#");
				attron(COLOR_PAIR(TREES_C));
				mvprintw(indexy-5,indexx-1,"###");
			}
			else if(world[world_indexy][world_indexx]=='N')
			{
				for(int temporary=indexx-2;temporary<=indexx+2;temporary++)
				{
					(temporary%2 == 1)	?	attron(COLOR_PAIR(GOLD_C)) : attron(COLOR_PAIR(WATER_C));
					mvprintw(indexy-2,temporary,"#");
					mvprintw(indexy-1,temporary,"#");
				}
				attroff(COLOR_PAIR(WATER_C));
				attroff(COLOR_PAIR(GOLD_C));
				attron(COLOR_PAIR(TREES_C));
				mvprintw(indexy,indexx-2,"#####");
			}
			else if(world[world_indexy][world_indexx]=='C')
			{
				(building == 2) ? attron(COLOR_PAIR(SWALL_C)) : attron(COLOR_PAIR(TREES_C));
				if(building==2)
				{
					mvprintw(indexy-3,indexx-1,"###");
					mvprintw(indexy-4,indexx-1,"###");
					mvprintw(indexy-5,indexx-2,"#####");
					for(int temporary=indexx-2;temporary<=indexx+2;temporary+=2)
					{
						mvprintw(indexy-6,temporary,"#");
					}
					attron(COLOR_PAIR(ENTRANCE_C));
					mvprintw(indexy-4,indexx,"#");
					attroff(COLOR_PAIR(ENTRANCE_C));
					attron(COLOR_PAIR(SWALL_C));
				}
				for(int temporary=indexx-5;temporary<=indexx+5;temporary+=2)
				{
					mvprintw(indexy-3,temporary,"#");
				}
				mvprintw(indexy-2,indexx-5,"###########");
				mvprintw(indexy-1,indexx-4,"#########");
				mvprintw(indexy,indexx-4,"#########");
				attroff(COLOR_PAIR(TREES_C));
				attron(COLOR_PAIR(ENTRANCE_C));
			}
			else if(world[world_indexy][world_indexx]=='S')
			{
				attron(COLOR_PAIR(ENTRANCE_C));
				mvprintw(indexy, indexx-3, "####");
				mvprintw(indexy-1,indexx-2, "##");
			}
			else if(world[world_indexy][world_indexx]=='M')
			{
				attron(COLOR_PAIR(ENTRANCE_C));
				mvprintw(indexy-1,indexx-2,"##");
				mvprintw(indexy-2,indexx-2,"##");
				attroff(COLOR_PAIR(ENTRANCE_C));
				attron(COLOR_PAIR(TREES_C));
				mvprintw(indexy,indexx-3,"####");
			}
			else if(world[world_indexy][world_indexx]=='L')
			{
				attron(COLOR_PAIR(TREES_C));
				mvprintw(indexy,indexx-2,"#####");
				mvprintw(indexy-1,indexx-2,"#####");
				mvprintw(indexy-2,indexx,"# #");
				mvprintw(indexy-3,indexx,"# #");
				attroff(COLOR_PAIR(TREES_C));
				attron(COLOR_PAIR(ENTRANCE_C));
				mvprintw(indexy-2,indexx+1,"#");
				mvprintw(indexy-3,indexx+1,"#");
			}
			else if(world[world_indexy][world_indexx]=='P')
			{
				attron(COLOR_PAIR(GOLD_C));
					mvprintw(indexy-1,indexx-2, "#####");
					mvprintw(indexy-2,indexx-2, "#####");
					mvprintw(indexy-3,indexx-2, "#####");
				attroff(COLOR_PAIR(GOLD_C));
				attron(COLOR_PAIR(TREES_C));
				mvprintw(indexy,indexx-1, "###");
			}
			else if(world[world_indexy][world_indexx]=='T')
			{
				if(leaves_on)
				{
					(winter) ? attron(COLOR_PAIR(LEAVESW_C)) : attron(COLOR_PAIR(LEAVES_C));
					mvprintw(indexy-3,indexx, "#");
					mvprintw(indexy-2,indexx-1, "###");
					mvprintw(indexy-1,indexx-1, "###");
					attroff(COLOR_PAIR(LEAVES_C));
					attroff(COLOR_PAIR(LEAVESW_C));
				}
				attron(COLOR_PAIR(TREES_C));
			}
			else if(world[world_indexy][world_indexx]=='H')
			{
				attron(COLOR_PAIR(TREES_C));
				mvprintw(indexy-3, indexx,"#");
				mvprintw(indexy-2, indexx-1,"###");
				mvprintw(indexy-1, indexx-2,"#####");
				mvprintw(indexy, indexx-2,"##");
				mvprintw(indexy, indexx+1,"##");
				attroff(COLOR_PAIR(TREES_C));
				attron(COLOR_PAIR(ENTRANCE_C));

			}
			mvprintw(indexy,indexx, "%c",world[world_indexy][world_indexx]);
			attroff(COLOR_PAIR(GRASS_C));
			attroff(COLOR_PAIR(GOLD_C));
			attroff(COLOR_PAIR(WATER_C));
			attroff(COLOR_PAIR(TREES_C));
			attroff(COLOR_PAIR(ENTRANCE_C));
			attroff(COLOR_PAIR(WWALL_C));
			attroff(COLOR_PAIR(SWALL_C));
			attroff(COLOR_PAIR(LEAVESW_C));
			attroff(COLOR_PAIR(GRASSW_C));
		}
		indexy-=LINES/2-20;
	}
	(winter) ? attron(COLOR_PAIR(PLAYERW_C)) : attron(COLOR_PAIR(PLAYER_C));
	if(player_facing==0)
	{
		mvprintw(player_y-offset_y+LINES/2-20,player_x-offset_x,"^");
	}
	else if(player_facing==1)
	{
		mvprintw(player_y-offset_y+LINES/2-20,player_x-offset_x,">");
	}
	else if(player_facing==2)
	{
		mvprintw(player_y-offset_y+LINES/2-20,player_x-offset_x,"V");
	}
	else if(player_facing==3)
	{
		mvprintw(player_y-offset_y+LINES/2-20,player_x-offset_x,"<");
	}
	attroff(COLOR_PAIR(PLAYER_C));
	attroff(COLOR_PAIR(PLAYERW_C));
}

void render_status_line(const int world_y, const int year, const int actual_pop, const int max_pop, const int food, const int wood, const int gold)
{
	attron(COLOR_PAIR(STATUS_C));
	for(int indexy =0; indexy<world_y;indexy++)
	{
		mvprintw(indexy+LINES/2-20, COLS-14, "              ");
	}
	mvprintw(1+LINES/2-20,COLS-13,"YEAR: %d",year);
	mvprintw(3+LINES/2-20,COLS-13,"POP: %d/%d", actual_pop,max_pop);
	mvprintw(5+LINES/2-20,COLS-13,"FOOD: %d",food);
	mvprintw(7+LINES/2-20,COLS-13,"WOOD: %d",wood);
	mvprintw(9+LINES/2-20,COLS-13,"GOLD: %d",gold);
	attroff(COLOR_PAIR(STATUS_C));
}

int build_menu(int *food, int *wood, int *gold, const int player_y, const int player_x, int *actual_pop, int *max_pop, const int age, const int world_y, const int world_x, char world[world_y][world_x], int *no_lumber, int *no_gold, int *no_farms, int *lumber_around, int *gold_around, bool *smithy, bool *market, bool *has_castle, int *score)
{
	attron(COLOR_PAIR(STATUS_C));
	mvprintw(13, COLS-13,"BUILD MENU:");
	if(age > 0)
	{
		mvprintw(15, COLS-13,"HOUSE: 50 W");
		mvprintw(17, COLS-13,"FARM: 75 W");
		mvprintw(19, COLS-13,"LUMBER: 50 W");
		mvprintw(38, COLS-13,"EXIT: 0");
	}
	if(age > 1)
	{
		mvprintw(21, COLS-13,"WALL: 3 W");
		mvprintw(23, COLS-13,"MINE: 100 W");
		mvprintw(25, COLS-13,"SMITHY: 150 W");
		mvprintw(27, COLS-13,"CASTLE: 300 W");
		mvprintw(28, COLS-13,"        200 G");
	}
	if(age > 2)
	{
		mvprintw(30, COLS-13,"MARKET: 200 W");
		mvprintw(31, COLS-13,"        100 G");
		mvprintw(33, COLS-13,"TOWER: 75 W");
		mvprintw(34, COLS-13,"       25 G");
	}
	attroff(COLOR_PAIR(STATUS_C));
	refresh();
	int input_build;	//dal som preč getch();
	do
	{
		input_build = getchar();
		switch (input_build)
		{
			case '1':
			{
				for(int indexy=player_y-2;indexy<=player_y;indexy++)
				{
					for(int indexx=player_x-2;indexx<=player_x+2;indexx++)
					{
						if(world[indexy][indexx]!='E')
						{
							return 8;
						}
					}
				}
				if(player_y>2 && player_y<world_y-2 && player_x>2 && player_x <world_x-2 && world[player_y][player_x]=='E' && *wood>=50)
				{
					for(int indexy=player_y-1;indexy<=player_y;indexy++)
					{
						for(int indexx=player_x-2;indexx<=player_x+2;indexx++)
						{
							world[indexy][indexx]='I';
						}
					}
					world[player_y][player_x]='H';
					*max_pop+=3;
					*wood-=50;
					*score+=50;
					return 0;
				}
				else return 1;
				break;
			}
			case '2':
			{
				for(int indexy=player_y-3;indexy<=player_y;indexy++)
				{
					for(int indexx=player_x-2;indexx<=player_x+2;indexx++)
					{
						if(world[indexy][indexx]!='E')
						{
							return 8;
						}
					}
				}
				if(player_y>2 && player_y<world_y-2 && player_x>2 && player_x <world_x-2 && world[player_y][player_x]=='E' && *wood>=75)
				{
					world[player_y][player_x]='P';
					world[player_y][player_x-1]='I';
					world[player_y][player_x+1]='I';
					*wood-=75;
					*no_farms+=1;
					*score+=30;
					return 0;
				}
				else return 1;
				break;
			}
			case '3':
			{
				for(int indexy=player_y-2;indexy<=player_y;indexy++)
				{
					for(int indexx=player_x-2;indexx<=player_x+2;indexx++)
					{
						if(world[indexy][indexx]!='E' && world[indexy][indexx]!='T')
						{
							return 8;
						}
					}
				}
				for(int indexy=player_y-10;indexy<player_y+10;indexy++)
				{
					for(int indexx=player_x-20;indexx<player_x+20;indexx++)
					{
						if(world[indexy][indexx]=='L')
						{
							return 9;
						}
					}
				}
				if(player_y>2 && player_y<world_y-2 && player_x>2 && player_x <world_x-2 && world[player_y][player_x]=='E' && *wood>=100)
				{
					for(int indexy=player_y-1;indexy<=player_y;indexy++)
					{
						for(int indexx=player_x-2;indexx<=player_x+2;indexx++)
						{
							world[indexy][indexx]='I';
						}
					}
					world[player_y][player_x]='L';
					for(int indexy=player_y-5;indexy<player_y+5;indexy++)
					{
						for(int indexx=player_x-10;indexx<player_x+10;indexx++)
						{
							if(world[indexy][indexx]=='T')
							{
								*lumber_around+=1;
								*score+=3;
							}
						}
					}
					*wood-=100;
					*no_lumber+=1;
					return 0;
				}
				else return 1;
				break;
			}
			case '4':
			{
				if(age < 2)
				{
					return 6;
				}
				if(world[player_y][player_x]=='E' && *wood>=3)
				{
					world[player_y][player_x]='V';
					*wood-=3;
					return 0;
				}
				else return 1;
				break;
			}
			case '5':
			{
				if(age < 2)
				{
					return 6;
				}
				for(int indexy=player_y-1;indexy<=player_y;indexy++)
				{
					for(int indexx=player_x-3;indexx<=player_x;indexx++)
					{
						if(world[indexy][indexx]!='E')
						{
							return 8;
						}
					}
				}
				for(int indexy=player_y-10;indexy<player_y+10;indexy++)
				{
					for(int indexx=player_x-20;indexx<player_x+20;indexx++)
					{
						if(world[indexy][indexx]=='M')
						{
							return 9;
						}
					}
				}
				if(world[player_y][player_x]=='E' && *wood>=100 && player_y>1 && player_x>2)
				{
					world[player_y][player_x]='M';
					world[player_y][player_x-1]='I';
					world[player_y][player_x-2]='I';
					world[player_y][player_x-3]='I';
					for(int indexy=player_y-5;indexy<player_y+5;indexy++)
					{
						for(int indexx=player_x-10;indexx<player_x+10;indexx++)
						{
							if(world[indexy][indexx]=='G')
							{
								*gold_around+=1;
								*score+=10;
							}
						}
					}
					*wood-=100;
					*no_gold+=1;
					return 0;
				}
				else return 1;
				break;
			}
			case '6':
			{
				if(age < 2)
				{
					return 6;
				}
				for(int indexy=player_y-1;indexy<=player_y;indexy++)
				{
					for(int indexx=player_x-3;indexx<=player_x;indexx++)
					{
						if(world[indexy][indexx]!='E')
						{
							return 8;
						}
					}
				}
				if(world[player_y][player_x]=='E' && *wood>=150 && player_y>0 && player_x>2)
				{
					world[player_y][player_x]='S';
					world[player_y][player_x-1]='I';
					world[player_y][player_x-2]='I';
					world[player_y][player_x-3]='I';
					*wood-=150;
					*smithy=TRUE;
					*score+=200;
					return 0;
				}
				else return 1;
				break;
			}
			case '7':
			{
				if(age < 2)
				{
					return 6;
				}
				for(int indexy=player_y-3;indexy<=player_y;indexy++)
				{
					for(int indexx=player_x-4;indexx<=player_x+4;indexx++)
					{
						if(world[indexy][indexx]!='E')
						{
							return 8;
						}
					}
				}
				if(world[player_y][player_x]=='E' && *wood>=300 && *gold>=200 && player_y>4 && player_x>4 && player_x<world_x)
				{
					for(int indexy=player_y-2;indexy<=player_y;indexy++)
					{
						for(int indexx=player_x-4;indexx<=player_x+4;indexx++)
						{
							world[indexy][indexx]='I';
						}
					}
					world[player_y][player_x]='C';
					*has_castle=TRUE;
					*wood-=300;
					*gold-=200;
					*score+=1000;
					return 0;
				}
				else return 1;
				break;
			}
			case '8':
			{
				if(age < 3)
				{
					return 6;
				}
				for(int indexy=player_y-3;indexy<=player_y;indexy++)
				{
					for(int indexx=player_x-2;indexx<=player_x+2;indexx++)
					{
						if(world[indexy][indexx]!='E')
						{
							return 8;
						}
					}
				}
				if(world[player_y][player_x]=='E' && *wood>=200 && *gold>=100 && player_y>1 && player_x>2 && player_x<world_x)
				{
					for(int indexy=player_y-2;indexy<=player_y;indexy++)
					{
						for(int indexx=player_x-2;indexx<=player_x+2;indexx++)
						{
							world[indexy][indexx]='I';
						}
					}
					world[player_y][player_x]='N';
					*market=TRUE;
					*score+=500;
					*wood-=200;
					*gold-=100;
					return 0;
				}
				else return 1;
				break;
			}
			case '9':
			{
				if(age < 3)
				{
					return 6;
				}
				for(int indexy=player_y-1;indexy<=player_y;indexy++)
				{
					for(int indexx=player_x-1;indexx<=player_x+1;indexx++)
					{
						if(world[indexy][indexx]!='E')
						{
							return 8;
						}
					}
				}
				if(world[player_y][player_x]=='E' && *wood>=75 && *gold>=25)
				{
					world[player_y][player_x]='O';
					world[player_y][player_x+1]='O';
					world[player_y][player_x-1]='O';
					*wood-=200;
					*gold-=100;
					*score+=150;
					return 0;
				}
				else return 1;
				break;
			}
			case '0':
			{
				return 0;
			}
			default:
			{
				break;
			}
		}
	}while(input_build!=0);
	return 0;
}

int upgrade_menu(int *food, int *wood, int *gold, int *actual_pop,const int max_pop, const int age, int *lumber_upgrade, int *farming_upgrade, int *mining_upgrade, int *building_upgrade,const bool smithy, int *score)
{
	attron(COLOR_PAIR(STATUS_C));
	mvprintw(13, COLS-13,"UPGRADE MENU:");
	if(age > 0)
	{
		mvprintw(15, COLS-13,"WORKER: 50 F");
		mvprintw(38, COLS-13,"EXIT: 0");
	}
	if(age > 1)
	{
		mvprintw(17, COLS-13,"LUMBER");
		(*lumber_upgrade == 1) ?  mvprintw(18, COLS-13,"200F 200G") : mvprintw(18, COLS-13,"RESEARCHED");
		mvprintw(20, COLS-13,"FARMING");
		(*farming_upgrade == 1) ?  mvprintw(21, COLS-13,"175W 100G") : mvprintw(21, COLS-13,"RESEARCHED");
	}
	if(age > 2)
	{
		mvprintw(23, COLS-13,"MINING");
		(*mining_upgrade == 1) ?  mvprintw(24, COLS-13,"300F 400G") : mvprintw(24, COLS-13,"RESEARCHED");
		mvprintw(26, COLS-13,"BUILDINGS");
		(*building_upgrade == 1) ?  mvprintw(27, COLS-14,"300F 200G 300W") : mvprintw(27, COLS-13,"RESEARCHED");
	}
	attroff(COLOR_PAIR(STATUS_C));
	refresh();
	int input_upgrade=0;
	do
	{
		input_upgrade = getch();
		switch (input_upgrade)
		{
			case '1':
			{
				if(max_pop>*actual_pop && *food>=50)
				{
					*food-=50;
					*actual_pop+=1;
					*score+=15;
					return 0;
				}
				return 2;
			}
			case '2':
			{
				if(age < 2)
				{
					return 6;
				}
				if(*lumber_upgrade==1 && *food>=200 && *gold>=200 && smithy)
				{
					*lumber_upgrade+=1;
					*food-=200;
					*gold-=200;
					*score+=2000;
					return 0;
				}
				else if(!smithy)
				{
					return 4;
				}
				else if(*lumber_upgrade==2)
				{
					return 3;
				}
				else 
				{
					return 1;
				}
			}
			case '3':
			{
				if(age < 2)
				{
					return 6;
				}
				if(*farming_upgrade==1 && *wood>=175 && *gold>=100 && smithy)
				{
					*farming_upgrade+=1;
					*food-=175;
					*gold-=100;
					*score+=3000;
					return 0;
				}
				else if(!smithy)
				{
					return 4;
				}
				else if(*farming_upgrade==2)
				{
					return 3;
				}
				else 
				{
					return 1;
				}
			}
			case '4':
			{
				if(age < 3)
				{
					return 6;
				}
				if(*mining_upgrade==1 && *food>=300 && *gold>=400 && smithy && age>2)
				{
					*mining_upgrade+=1;
					*food-=300;
					*gold-=400;
					*score+=3000;
					return 0;
				}
				else if(!smithy)
				{
					return 4;
				}
				else if(*mining_upgrade==2)
				{
					return 3;
				}
				else 
				{
					return 1;
				}
			}
			case '5':
			{
				if(age < 3)
				{
					return 6;
				}
				if(*building_upgrade==1 && *food>=300 && *gold>=200 && *wood>=300 && smithy && age>2)
				{
					*building_upgrade+=1;
					*food-=300;
					*wood-=300;
					*gold-=200;
					*score+=5000;
					return 0;
				}
				else if(!smithy)
				{
					return 4;
				}
				else if(*building_upgrade==2)
				{
					return 3;
				}
				else 
				{
					return 1;
				}
			}
			case '0':
			{
				return 0;
			}
			default:
			{
				break;
			}
		}
	}while(input_upgrade!=0);
	return 0;
}

void pop_management(const int no_lumber, const int no_farms, const int no_gold, const int actual_pop, int *pop_on_lumber, int *pop_on_farms, int *pop_on_gold)
{
	attron(COLOR_PAIR(TREES_C));
	for(int indexy=LINES/2-10;indexy<=LINES/2+5;indexy++)
	{
		mvprintw(indexy, COLS/2-25, "########################################");
	}
	attroff(COLOR_PAIR(TREES_C));
	attron(COLOR_PAIR(STATUS_C));
	mvprintw(LINES/2-9,COLS/2-16,"POPULATION MANAGEMENT:");
	mvprintw(LINES/2-6,COLS/2+2,"MAX: %d",no_farms);
	mvprintw(LINES/2-4,COLS/2+2,"MAX: %d",no_lumber);
	mvprintw(LINES/2-2,COLS/2+2,"MAX: %d",no_gold);
	mvprintw(LINES/2-6,COLS/2-24,"POP ON FARMS:");
	mvprintw(LINES/2-4,COLS/2-24,"POP ON LUMBER:");
	mvprintw(LINES/2-2,COLS/2-24,"POP ON GOLD:");
	mvprintw(LINES/2+4,COLS/2-24,"EXIT: 0");
	int pop_input=0, selection=-6;
	do
	{
		mvprintw(LINES/2-6,COLS/2-7,"        ");
		mvprintw(LINES/2-4,COLS/2-7,"        ");
		mvprintw(LINES/2-2,COLS/2-7,"        ");
		mvprintw(LINES/2+selection,COLS/2-7,">    <");
		mvprintw(LINES/2-6,COLS/2-5,"%d",*pop_on_farms);
		mvprintw(LINES/2-4,COLS/2-5,"%d",*pop_on_lumber);
		mvprintw(LINES/2-2,COLS/2-5,"%d",*pop_on_gold);
		mvprintw(LINES/2+1,COLS/2-24,"UNEMPLOYED POPULATION: %d ",actual_pop-*pop_on_farms-*pop_on_lumber-*pop_on_gold);
		refresh();
		pop_input=getch();
		switch (pop_input)
		{
			case KEY_DOWN:
			{
				if(selection<-2)
				{
					selection+=2;
				}
				break;
			}
			case KEY_UP:
			{
				if(selection>-6)
				{
					selection-=2;
				}
				break;
			}
			case KEY_RIGHT:
			{
				if(selection==-2 && actual_pop-*pop_on_farms-*pop_on_lumber-*pop_on_gold>0 && *pop_on_gold<no_gold)
				{
					*pop_on_gold+=1;
				}
				else if(selection==-4 && actual_pop-*pop_on_farms-*pop_on_lumber-*pop_on_gold>0 && *pop_on_lumber<no_lumber)
				{
					*pop_on_lumber+=1;
				}
				else if(selection==-6 && actual_pop-*pop_on_farms-*pop_on_lumber-*pop_on_gold>0 && *pop_on_farms<no_farms)
				{
					*pop_on_farms+=1;
				}
				break;
			}
			case KEY_LEFT:
			{
				if(selection==-2 && *pop_on_gold>0)
				{
					*pop_on_gold-=1;
				}
				else if(selection==-4 && *pop_on_lumber>0)
				{
					*pop_on_lumber-=1;
				}
				else if(selection==-6 && *pop_on_farms>0)
				{
					*pop_on_farms-=1;
				}
				break;
			}
			case '0':
			{
				break;
			}	
			default:
			{
				break;
			}
		}
	}while(pop_input!='0');
	attroff(COLOR_PAIR(STATUS_C));
}

void market_menu(int *food, int *wood, int *gold, float *market_coeficient)
{
	int price=*market_coeficient*45;
	int market_input=1, selection=-4, market_error_phase=0;
	do
	{
		nanosleep(&ts, NULL);
		price=(price > 295) ? 300 : *market_coeficient*45;
		attron(COLOR_PAIR(TREES_C));
		for(int indexy=LINES/2-10;indexy<=LINES/2+5;indexy++)
		{
			mvprintw(indexy, COLS/2-25, "########################################");
		}
		attroff(COLOR_PAIR(TREES_C));
		attron(COLOR_PAIR(STATUS_C));
		mvprintw(LINES/2-9,COLS/2-10,"MARKET:");
		mvprintw(LINES/2-7,COLS/2-24,"YOUR GOLD: %d", *gold);
		mvprintw(LINES/2-4,COLS/2-24,"PRICE FOR 50 FOOD: %d", price);
		mvprintw(LINES/2-2,COLS/2-24,"PRICE FOR 50 LUMBER: %d", price);
		mvprintw(LINES/2+2,COLS/2-24,"YOU HAVE %d FOOD AND %d LUMBER", *food,*wood);
		mvprintw(LINES/2+4,COLS/2-24,"EXIT: 0      BUY: ENTER");
		mvprintw(LINES/2-4,COLS/2+2,"        ");
		mvprintw(LINES/2-2,COLS/2+2,"        ");
		mvprintw(LINES/2+selection,COLS/2+2,">     <");
		mvprintw(LINES/2-4,COLS/2+4,"BUY");
		mvprintw(LINES/2-2,COLS/2+4,"BUY");
		if(market_error_phase>0)
		{
			market_error_phase++;
			mvprintw(LINES/2, COLS/2-24, "NOT ENOUGH GOLD!");
			if(market_error_phase==200)
			{
				market_error_phase=0;
			}
		}
		refresh();
		market_input=getch();
		switch (market_input)
		{
			case KEY_DOWN:
			{
				if(selection<-2)
				{
					selection+=2;
				}
				break;
			}
			case KEY_UP:
			{
				if(selection>-4)
				{
					selection-=2;
				}
				break;
			}
			case '\n':
			{
				if(selection==-4 && *gold>=price)
				{
					*food+=50;
					*gold-=price;
					*market_coeficient+=0.1;
				}
				else if(selection==-2 && *gold>=price)
				{
					*wood+=50;
					*gold-=price;
					*market_coeficient+=0.1;
				}
				else
				{
					market_error_phase=1;
				}
				break;
			}
			case '0':
			{
				break;
			}	
			default:
			{
				break;
			}
		}
	}while(market_input!='0');
}

int savegame(const char save_file[50], const int world_y,const int world_x, const int player_y, const int player_x, const int year, const int actual_pop, const int food, const int wood, const int gold, const char world[world_y][world_x], const int offset_y, const int offset_x, const int max_pop, const int age, const int no_lumber, const int no_gold, const int no_farms, const int lumber_around, const int gold_around, const int pop_on_lumber, const int pop_on_farms, const int pop_on_gold, const int lumber_upgrade, const int farming_upgrade, const int mining_upgrade, const int building_upgrade, const int score, const int advancing_stage, const int season_stage, const float market_coeficient, const bool smithy, const bool winter, const bool market, const bool has_castle, const bool won)
{
	FILE *autosave;
	attron(COLOR_PAIR(STATUS_C));
	autosave=fopen(save_file, "w");
	if(autosave==NULL)
	{
		return 1;
	}
	fprintf(autosave,"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %f %d %d %d %d %d \n", world_y, world_x, player_y, player_x, year, actual_pop, food, wood, gold, offset_y, offset_x, max_pop, age, no_lumber, no_gold, no_farms, lumber_around, gold_around, pop_on_lumber, pop_on_farms, pop_on_gold, lumber_upgrade, farming_upgrade, mining_upgrade, building_upgrade, score,advancing_stage, season_stage, market_coeficient, smithy, winter, market, has_castle, won);
	for(int indexy=0;indexy<world_y;indexy++)
	{
		for(int indexx=0;indexx<world_x;indexx++)
		{
			if(world[indexy][indexx] != 'E')
			{
				fprintf(autosave,"%d %d%c\n",indexy,indexx,world[indexy][indexx]);
			}
		}
		mvprintw(LINES/2-2,COLS/2-13, "                                  ");
		mvprintw(LINES/2-1,COLS/2-13, "                                  ");
		mvprintw(LINES/2-1,COLS/2-13, "  %s", save_file);
		mvprintw(LINES/2,COLS/2-13, "                                  ");
		mvprintw(LINES/2,COLS/2-13, "          SAVING... %.2f%%",indexy/(float)world_y*100);
		mvprintw(LINES/2+1,COLS/2-13, "                                  ");
		refresh();
		nanosleep(&ts, NULL);
	}
	attroff(COLOR_PAIR(STATUS_C));
	fclose(autosave);
	return 0;
}

void hint()
{
	int input=0;
	menu_background();
	attron(COLOR_PAIR(STATUS_C));
	mvprintw(3, 4,"Controls: move by arrows, you can only move on grass tiles");
	mvprintw(4, 4,"Use theese keys to access different menus: V - population management, here you can tell where your citizens should be working");
	mvprintw(5, 4,"  +------------------------------------+   B - build menu, here you can build buildings using resouces (select by pressing 1-9)");
	mvprintw(6, 4,"  |To navigate upgrade and build menus:|   N - upgrades menu, here you can upgrade various aspects of your economy or buildings");
	mvprintw(7, 4,"  |Press keys from 0-9                 |   M - market menu, here you can buy resources by spending gold");
	mvprintw(8, 4,"  |To navigate population management,  |   F1 - turn leaves on/off");
	mvprintw(9, 4,"  |market and main menus:              |   F2 - show/hide score");
	mvprintw(10,4,"  |use arrow keys to move around and   |   CAPITAL D - DELETES building you are standing on AND one tile above you!");
	mvprintw(11,4,"  |change values, use Enter to confirm |   A - advancing to the next age");
	mvprintw(12,4,"  +------------------------------------+   P - pause, Q - quit game, S - save game");
	mvprintw(13,4,"Gameplay: You start with nothing but a few resources and are supposed through careful planing develop you land, gather");
	mvprintw(14,4,"resources and expand your town and influence over the region. To gather resources, you have to place a building specified for");
	mvprintw(15,4,"gathering the said resource, namely lumber camp around for wood, mine for gold and farms for food. Mines and lumber camps need");
	mvprintw(16,4,"to be placed NEAR the resource they are supposed to be gathering, they also cannot be build too near each other");
	mvprintw(17,4,"To gather resources you also need workers, which you can buy from upgrades menu. You win when you build a castle, upgrade it");
	mvprintw(18,4,"and have atleast 50 population. After this, your score is shown and you can quit the game or continue playing if you wish so.");
	mvprintw(19,4,"The game can also end prematurely when eihter year 1444 is reached or you lose all your workers and lack the resources to");
	mvprintw(20,4,"produce more. This also extends to the market, you cannot buy anything if there are no people that would make the trade.");
	mvprintw(22,4,"Worker needs: Workers are not robots, they are people, they need to be fed and kept warm or else they die! Each worker will");
	mvprintw(23,4,"consume food from your stockpile at the end a year. Also during winter, each HOUSE, even empty, will consume 5 wood for heating.");
	mvprintw(24,4,"If your stockpile runs out of food or wood, there is a big chance that each year one of your citizens will die!");
	mvprintw(26,4,"Winter: Winter is a bane of all medieval cities, lack of food and fuel for fires everywhere. This is interpreted in a game by 2");
	mvprintw(27,4,"factors. For one, you cannot harvest any food during winter, so make sure you can survive it!. Also houses will consume 5 wood");
	mvprintw(28,4,"for heating. At the start of the game, you have 30 \"years\" to prepare and the winter lasts for 10 \"years\". After that, you have");
	mvprintw(29,4,"again 30 \"years\" to prepare.");
	mvprintw(31,4,"Advancing though the ages: The game is divided through 3 different ages, each one provides new buildings and upgrades, but");
	mvprintw(32,4,"require a lot of resources and time to research, unlike normal upgrades that just improve you economy, buildings etc...");
	mvprintw(34,4,"User interface: Left part of the screen is the playing area, where you control your character, build buildings etc..., on the");
	mvprintw(35,4,"right you have information panel that shows you all important information about your kingdom such as population and resources.");
	mvprintw(LINES-4,COLS/2-10,"PRESS ENTER TO EXIT");
	attroff(COLOR_PAIR(STATUS_C));
	do
	{
		input=getch();
	}while (input!='\n');
	refresh();
}

void menu_background()
{
	attron(COLOR_PAIR(TREES_C));
	for(int indexy=0;indexy<LINES;indexy++)
	{
		for(int indexx=0;indexx<COLS;indexx++)
		{
			mvprintw(indexy,indexx,"#");
		}
	}
	attroff(COLOR_PAIR(TREES_C));
	attron(COLOR_PAIR(GOLD_C));
	for(int indexy=1;indexy<LINES-1;indexy+=3)
	{
		mvprintw(indexy,1,"#");
		mvprintw(indexy,COLS-2,"#");
	}
	for(int indexx=1;indexx<COLS-1;indexx+=4)
	{
		mvprintw(1,indexx,"#");
		mvprintw(LINES-2,indexx,"#");
	}
	attroff(COLOR_PAIR(GOLD_C));
}

char overwite_notification()
{
	clear();
	char choice;
	mvprintw(LINES/2-1,COLS/2-24, "                                               ");
	mvprintw(LINES/2,COLS/2-24, "                                               ");
	mvprintw(LINES/2,COLS/2-24, "     DO YOU WANT TO OVERWITE THE FILE? Y/N     ");
	mvprintw(LINES/2+1,COLS/2-24, "                                               ");
	refresh();
	do
	{
		choice=getch();
	}while(choice!='Y' &&choice!='y' &&choice!='N' &&choice!='n');
	clear();
	return choice;
}