/**************************************************************************
 *                                                                        *
 * Apocalypse Dawn: Adventure in Post-Apocalyptic America                 *
 * by Mike Kramlich                                                       *
 * ZodLogic Games                                                         *
 *                                                                        *
 * file: dawn.c                                                           *
 * dependencies:                                                          *
 *     lib.c - generic code extracted from but bundled with this project  *
 *     the Standard C Library                                             *
 *                                                                        *
 * project started: July 8, 2002                                          *
 * last revised:    March 30, 2008                                        *
 *                                                                        *
 * Author Contact Info:                                                   *
 *                                                                        *
 *       email:  groglogic@gmail.com                                      *
 *       website: synisma.com                                             *
 *                                                                        *
 *                                                                        *
 * Copyright 2002-2008 by Mike Kramlich                                   *
 * All rights reserved worldwide                                          *
 *************************************************************************/

/*************************************************/
/* Includes                                      */
/*************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdarg.h>
/*#include <sys/times.h>  times(), clock_t, struct tms */
/*#include <sys/timeb.h>  ftime(), struct timeb */
#include <time.h> /* time() */
#include <math.h> /* sqrt() */
/*#include <unistd.h>     sysconf(), _SC_CLK_TCK, usleep() */

#ifdef UNIX
/*#include <dirent.h> opendir(), readdir()... */
#endif


/*************************************************/
/* Constants                                     */
/*************************************************/

#define VERSION_MAJOR 0
#define VERSION_MINOR 0

#define AUTHOR  "Mike Kramlich"
#define COMPANY "ZodLogic Games"
#define WEBSITE "zodlogic.webfactional.com"
#define EMAIL   "groglogic@gmail.com"

#define TRUE 1
#define FALSE 0

#define MAX_SAFE_STRLEN 500

#define SCREEN_CHARS_WIDTH 78
#define SCREEN_CHARS_HEIGHT 25

#define MAPX 500
#define MAPY 500

#define TERRAINS 7
#define PERSONTYPES 21
#define GROUPS 2000
#define PILES 100
#define ITEMTYPES 23
#define ORGS 3

#define XMARGIN 4
#define YMARGIN 3

#define CITY_OWN_PER_TO_WIN 0.1

#define CONTINENT_NAME "Nor Amerka"
#define CONTINENT_NAME_PLURAL "Nor Amerkan"

#define YOUR_STARTING_FOOD 20
#define FORMER_REFUGEE_COUNT 13

#define STARTX (MAPX/2)
#define STARTY (MAPY/2)

#define YOU_SYMBOL "@"
#define GROUP_SYMBOL "P"
#define PILE_SYMBOL "%%"

#define MAX_S16INT 32767


/*************************************************/
/* Data Types                                    */
/*************************************************/

#include "lib.c"

struct area {
	s16int terraintype;
	s16int groupid;
	s16int pileid;
	bool youown;
};

struct terrain {
	char name[30];
	char symbol;
	bool impassable;
	s16int movetickcost;
};

struct persontype {
	char name[30];
	char nameplural[30];
	char desc[501];
	u32int basefrequency;/*remember to add this and the unique stuff to itemtype as well!*/
	float terrfreqmultiplier[TERRAINS];
	bool unique;
	bool unique_inplay;
	bool unique_permgone;
	bool is_animal;
	bool is_dangerous;
	bool known;
	u16int combatvalue;
	char symbol;
};

struct itemtype {
	char name[30];
	char nameplural[30];
	char desc[501];
	u32int basefrequency;/*remember to add this and the unique stuff to itemtype as well!*/
	float terrfreqmultiplier[TERRAINS];
	bool unique;
	bool unique_inplay;
	bool unique_permgone;
	bool known;
	u16int combatvalue;
};

struct group {
	bool used;
	s16int x, y;
	s16int persons[PERSONTYPES];
	s16int items[ITEMTYPES];
	s16int owner;
};

struct pile {
	bool used;
	s16int x, y;
	s16int items[ITEMTYPES];
};

struct org {
	char name[75];
};

/*************************************************/
/* Data                                          */
/*************************************************/

		/*********************************/
		/*    Primary/Persistent Data    */
		/*********************************/
		
		/* This is data which must be
			written to disk as part of any
			saved game. */

u32int turn;
s16int yourx, youry;
s16int yourgroup;
struct area areas[MAPX][MAPY];
struct terrain terrains[TERRAINS];
struct persontype persontypes[PERSONTYPES];
struct itemtype itemtypes[ITEMTYPES];
struct group groups[GROUPS];
struct pile piles[PILES];
struct org orgs[ORGS];
char lastmsg[500]="";
bool showlastmsg;
bool allowtojoin[PERSONTYPES];
bool gamewon;
bool enforce_eat, enforce_rndmvmt;

		/***********************************/
		/*    Derivative/Transient Data    */
		/***********************************/
		
		/* This is data which is transient
			and fleeting in nature, or can be
			derived from the primary data when
			needed (usually at program startup
			or after restoring state from a
			save game file. */
			
bool moving;
			
			
		/************************************/
		/*   Convenience/Compile-Time Data  */
		/************************************/
		
s16int terraintype_plains, terraintype_grassland, terraintype_urban,
			terraintype_sea, terraintype_forest, terraintype_desert,
			terraintype_mountains;
			
s16int persontype_you, persontype_thug, persontype_refugee,
			persontype_greenberet, persontype_drillsergeant, persontype_farmer,
			persontype_hunter, persontype_hippie, persontype_engineer,
			persontype_doctor, persontype_lawyer, persontype_beggar, persontype_leper,
			persontype_thief, persontype_zombie, persontype_mutant, persontype_psycho,
			persontype_biker, persontype_rat, persontype_wolf, persontype_bear;
			
s16int itemtype_food, itemtype_gun, itemtype_briefcase_of_power,
			itemtype_crudemap, itemtype_knife, itemtype_bootpair, itemtype_blanket,
			itemtype_coat, itemtype_medicine, itemtype_handtool, itemtype_bullet,
			itemtype_club, itemtype_spear, itemtype_bow, itemtype_arrow, itemtype_axe,
			itemtype_lumber, itemtype_nail, itemtype_rock, itemtype_bag,
			itemtype_backpack, itemtype_twig, itemtype_branch;
			
s16int org_you, org_independent, org_enemy;
char dirnames[10][20];
int dir2xrel[10], dir2yrel[10];


/*************************************************/
/* Functions                                     */
/*************************************************/

void handle_help() {
	printf("Here's a guide to which dir num points which dir:\n");
	printf("  N\n"\
				 "\\ | /\n"\
				 " 789\n"\
				 "-4 6-\n"\
				 " 123\n"\
				 "/ | \\\n\n");
	printf("Here is a list of commands and a short description of each:\n");
	printf("1-4,6-9                           - move one space in that dir\n");
	printf("k                                 - map key\n");
	printf("r                                 - recruit\n");
	printf("l                                 - loot\n");
	printf("e                                 - examine area\n");
	printf("p                                 - pickup all items from pile in your area\n");
	printf("t                                 - takeover city\n");
	printf("c                                 - cities report\n");
	printf("w                                 - wait (do nothing for a day)\n");
	printf("foo                               - report stuff around me\n");
	printf("people                            - give complete list of people in your party\n");
	printf("items                             - give complete list of items in your party\n");
	printf("dp <persontype>                   - describe person type\n");
	printf("di <itemtype>                     - describe item type\n");
	printf("g <dir>                           - glance in direction\n");
	printf("a <dir>                           - attack in direction\n");
	printf("pt                                - list person types and their ID #'s\n");
	printf("it                                - list item types and their ID #'s\n");
	printf("d <persontype> <qty>              - dismiss people from your party\n");
	printf("th <itemtype> <qty>               - throwaway items from your party\n");
	printf("givep <persontype> <qty> <dir>    - give people to group in dir\n");
	printf("givei <itemtype> <qty> <dir>      - give items to group in dir\n");
	printf("transi <itemtype> <qty> <dir>     - transfer items to pile in dir\n");
	printf("getp <persontype> <qty> <dir>     - get people from group in dir\n");
	printf("geti <itemtype> <qty> <dir>       - get items from group in dir\n");
	printf("grabi <itemtype> <qty> <dir>      - grab items from pile in dir\n");
	printf("drop <itemtype> <qty>             - drop items into the pile in your area\n");
	printf("pickup <itemtype> <qty>           - pickup items from the pile in your area\n");
	printf("rp                                - show your policy on who to allow/refuse join\n");
	printf("allowall                          - allow all people types when recruiting\n");
	printf("refuseall                         - refuse all people types when recruiting\n");
	printf("allow <persontype>                - allow recruiting of person type\n");
	printf("refuse <persontype>               - refuse recruiting of person type\n");
	printf("h,help,?                          - this help menu\n");
	printf("intro                             - read the intro story\n");
	printf("credits                           - see the credits\n");
	printf("rules                             - description of game rules and mechanics\n");
	printf("ver                               - version info\n");
	printf("q,quit,exit                       - quit game and exit program\n");
	printf("F                                 - double food (FOR DEV ONLY)\n");
	printf("D                                 - dev report\n");
	printf("Dxy <x> <y>                       - teleport your group to (x,y)\n");
	printf("De                                - toggle enforce_eat\n");
	printf("Dm                                - toggle enforce_rndmvmt\n");
	printf("Dpt                               - persontypes\n");
	printf("Dit                               - itemtypes\n");
	printf("Dcv <dir>                         - combatvalue of group in dir\n");
	printf("Ddai                              - double all your non-unique items\n");
	printf("Ddap                              - double all your non-unique persons\n");
}

void g_items_add(s16int g, s16int i, s16int qty) {
	if ((s32int)groups[g].items[i] + (s32int)qty >= (s32int)MAX_S16INT) {
		groups[g].items[i] = MAX_S16INT - 1;
	} else {
		groups[g].items[i] += qty;
	}
	if (g == yourgroup) itemtypes[i].known = TRUE;
}

void p_items_add(s16int p, s16int i, s16int qty) {
	if ((s32int)piles[p].items[i] + (s32int)qty >= (s32int)MAX_S16INT) {
		piles[p].items[i] = MAX_S16INT - 1;
	} else {
		piles[p].items[i] += qty;
	}
}

void g_items_lose(s16int g, s16int i, s16int qty) {
	if (groups[g].items[i] > qty) {
		groups[g].items[i] -= qty;
	} else {
		groups[g].items[i] = 0;
	}
}

void p_items_lose(s16int p, s16int i, s16int qty) {
	if (piles[p].items[i] > qty) {
		piles[p].items[i] -= qty;
	} else {
		piles[p].items[i] = 0;
	}
}

void g_persons_add(s16int g, s16int pt, s16int qty) {
	if ((s32int)groups[g].persons[pt] + (s32int)qty >= (s32int)MAX_S16INT) {
		groups[g].persons[pt] = MAX_S16INT - 1;
	} else {
		groups[g].persons[pt] += qty;
	}
	if (g == yourgroup) persontypes[pt].known = TRUE;
}

void g_persons_lose(s16int g, s16int pt, s16int qty) {
	if (groups[g].persons[pt] > qty) {
		groups[g].persons[pt] -= qty;
	} else {
		groups[g].persons[pt] = 0;
	}
}

/* TODO is_valid fns are generic could be moved into lib if associated with a struct for MAPX,MAPY*/
bool is_valid_x(s16int x) {
	if ((x >= 0) && (x < MAPX)) return TRUE;
	else return FALSE;
}

bool is_valid_y(s16int y) {
	if ((y >= 0) && (y < MAPY)) return TRUE;
	else return FALSE;
}

bool is_valid_xy(s16int x, s16int y) {
	if (is_valid_x(x) && is_valid_y(y)) return TRUE;
	else return FALSE;
}

bool is_tooclosetoedge(s16int x, s16int y) {
	if ((x < XMARGIN) || (x >= MAPX - XMARGIN)
		|| (y < YMARGIN) || (y >= MAPY - YMARGIN)) return TRUE;
	else return FALSE;
}

void msg(char *txt) {
	strcpy(lastmsg,txt);
	if (strlen(lastmsg) != 0) showlastmsg = TRUE;
}

void hunters_hunt() {
	s16int hunters;
	u16int foodfound;
	
	hunters = groups[yourgroup].persons[persontype_hunter];
	if (hunters == 0) return;
	foodfound = get_rnd_u16int_with_notinclusive_max((hunters*4)+1);
	if (foodfound > 0) printf("hunters captured %hu food!\n",foodfound);
	g_items_add(yourgroup,itemtype_food,foodfound);
}

void farmers_farm() {
	int g;
	s16int farmers;
	u16int foodfound;
	
	for (g = 0 ; g < GROUPS ; g++) {
		if (!groups[g].used) continue;
		if ((g == yourgroup) && moving) continue;
		farmers = groups[g].persons[persontype_farmer];
		if (farmers == 0) continue;
		foodfound = get_rnd_u16int_with_notinclusive_max((farmers*6)+1);
		if ((foodfound < farmers) && (farmers > 1)) foodfound = farmers / 2;
		if ((foodfound > 0) && (g == yourgroup)) printf("farmers grew %hu food!\n",foodfound);
		g_items_add(g,itemtype_food,foodfound);
	}
}

s32int get_people_count(s16int groupid) {
	int pt;
	s32int count;
	
	count = 0;
	for (pt = 0 ; pt < PERSONTYPES ; pt++) {
		count += groups[groupid].persons[pt];
	}
	return count;
}

s32int get_items_count(s16int groupid) {
	int it;
	s32int count;
	
	count = 0;
	for (it = 0 ; it < ITEMTYPES ; it++) {
		count += groups[groupid].items[it];
	}
	return count;
}

s32int get_your_people_count() {
	return get_people_count(yourgroup);
}

bool does_group_have_people(s16int groupid) {
	int pt;

	for (pt = 0 ; pt < PERSONTYPES ; pt++) {
		if (groups[groupid].persons[pt] > 0) return TRUE;
	}
	return FALSE;
}

void my_exit() {
	printf("Exitting game...\n");
	exit(EXIT_SUCCESS);
}

void show_final_score() {
	printf("Were you alive at the time you quit? %s\n",(groups[yourgroup].persons[persontype_you]>0?"YES!!!":"no"));
	printf("Did you ever win the game? %s\n",(gamewon?"YES!!!":"no"));
	if ((yourgroup > -1) && (groups[yourgroup].items[itemtype_briefcase_of_power] > 0))
		printf("You possessed the Briefcase of Power!\n");
	printf("Here are your stats: (none)\n");
	printf("Here is your score: 0\n");
}

void gameover() {
	printf("GAME OVER!\n");
	show_final_score();
	my_exit();
}

void calc_city_vals(u32int *yourcities, u32int *totalcities, float *yourpercent) {
	int x, y;
	
	*totalcities = 0;
	*yourcities = 0;
	for (x = 0 ; x < MAPX ; x++) {
		for (y = 0 ; y < MAPY ; y++) {
			if (areas[x][y].terraintype == terraintype_urban) {
				(*totalcities)++;
				if (areas[x][y].youown) (*yourcities)++;
			}
		}
	}
	*yourpercent = (float)*yourcities / (float)*totalcities;
}

void check_for_victory() {
	u32int totalcities, yourcities;
	float yourpercent;
	bool youwin;
	
	/*printf("checking for victory...\n");*/
	
	if (gamewon) return;
	
	calc_city_vals(&yourcities,&totalcities,&yourpercent);
	
	youwin = FALSE;
	if (yourpercent >= CITY_OWN_PER_TO_WIN) {
		printf("you control %lu of the %lu cities in " CONTINENT_NAME ", which is %hu%%!\n",yourcities,totalcities,(u16int)(yourpercent*100));
		youwin = TRUE;
	}
	
	if (groups[yourgroup].items[itemtype_briefcase_of_power] > 0) {
		printf("holy cow! you possess the legendary Briefcase of Power!\n");
		youwin = TRUE;
	}
	
	if (youwin) {
		printf("you win the game!\n");
		printf("you may now retire at any time, or, continue playing...\n");
		gamewon = TRUE;
	}
}

void wipe_pile(int p) {
	int it;
	
	piles[p].used = FALSE;
	piles[p].x = 0;
	piles[p].y = 0;
	for (it = 0 ; it < ITEMTYPES ; it++) {
		piles[p].items[it] = 0;
	}
}

void wipe_group(int g) {
	int it, pt;
	
	groups[g].used = FALSE;
	groups[g].x = 0;
	groups[g].y = 0;
	groups[g].owner = org_independent;
	for (pt = 0 ; pt < PERSONTYPES ; pt++) {
		groups[g].persons[pt] = 0;
	}
	for (it = 0 ; it < ITEMTYPES ; it++) {
		groups[g].items[it] = 0;
	}
}

void destroy_group(s16int g) {
	groups[g].used = FALSE;
	areas[groups[g].x][groups[g].y].groupid = -1;
	if (g == yourgroup) yourgroup = -1;
}

void destroy_pile(s16int p) {
	piles[p].used = FALSE;
	areas[piles[p].x][piles[p].y].pileid = -1;
}

int create_pile(s16int x, s16int y) {
	int p;
	bool found;
	
	found = FALSE;
	for (p = 0 ; p < PILES ; p++) {
		if (piles[p].used) continue;
		found = TRUE;
		break;
	}
	
	if (!found) return -1;
	
	if (areas[x][y].pileid != -1) destroy_pile(areas[x][y].pileid);

	areas[x][y].pileid = p;
	wipe_pile(p);
	piles[p].used = TRUE;
	piles[p].x = x;
	piles[p].y = y;
	
	return p;
}

int create_group(s16int x, s16int y, s16int org) {
	int g;
	bool found;
	
	found = FALSE;
	for (g = 0 ; g < GROUPS ; g++) {
		if (groups[g].used) continue;
		found = TRUE;
		break;
	}
	
	if (!found) return -1;
	
	if (areas[x][y].groupid != -1) destroy_group(areas[x][y].groupid);

	areas[x][y].groupid = g;
	wipe_group(g);
	groups[g].used = TRUE;
	groups[g].x = x;
	groups[g].y = y;
	groups[g].owner = org;
	
	return g;
}

int get_or_create_pile_at(s16int x, s16int y) {
	if (areas[x][y].pileid != -1) return areas[x][y].pileid;
	return create_pile(x,y);
}

int get_or_create_group_at(s16int x, s16int y, s16int org) {
	if (areas[x][y].groupid != -1) return areas[x][y].groupid;
	return create_group(x,y,org);
}

void destroy_group_if_empty(s16int g) {
	int it, pt;

	for (it = 0 ; it < ITEMTYPES ; it++) {
		if (groups[g].items[it] > 0) return;
	}
	
	for (pt = 0 ; pt < PERSONTYPES ; pt++) {
		if (groups[g].persons[pt] > 0) return;
	}

	destroy_group(g);
}

void destroy_group_if_nobody(s16int g) {
	int it, pt, p;
	bool hasitems;
	
	for (pt = 0 ; pt < PERSONTYPES ; pt++) {
		if (groups[g].persons[pt] > 0) return;
	}
	
	hasitems = FALSE;
	for (it = 0 ; it < ITEMTYPES ; it++) {
		if (groups[g].items[it] > 0) {
			hasitems = TRUE;
			break;
		}
	}
	
	if (hasitems) {
		p = get_or_create_pile_at(groups[g].x, groups[g].y);
		if (p != -1) {
			for (it = 0 ; it < ITEMTYPES ; it++) {
				if (groups[g].items[it] > 0) {
					p_items_add(p,it,groups[g].items[it]);
					g_items_lose(g,it,groups[g].items[it]);
				}
			}
		}
	}

	destroy_group(g);
}

/*int get_or_create_pile_at(s16int x, s16int y)*/

void destroy_pile_if_empty(s16int p) {
	int it;

	for (it = 0 ; it < ITEMTYPES ; it++) {
		if (piles[p].items[it] > 0) return;
	}

	destroy_pile(p);
}

void people_eat() {
	int g;
	s16int type;
	s32int i, starve, total_people;
	bool youstarve;
	
	youstarve = FALSE;	
	for (g = 0 ; g < GROUPS ; g++) {
		if (!groups[g].used) continue;
	
		total_people = get_people_count(g);
		if ((s32int)groups[g].items[itemtype_food] >= total_people) {
			/* everybody eats okay... */
			g_items_lose(g,itemtype_food,total_people);
		} else {
			/* some persons starve to death... */
			starve = total_people - (s32int)groups[g].items[itemtype_food];
			groups[g].items[itemtype_food] = 0;
			if ((g == yourgroup) && (starve == total_people)) {
				youstarve = TRUE;
				starve--;
			}
			if ((starve > 0) && (g == yourgroup)) printf("%li of your people starve to death:\n",starve);
			for (i = 0 ; i < starve ; i++) {
				do {
				type = 1 + get_rnd_u16int_with_notinclusive_max(PERSONTYPES-1);
				} while (groups[g].persons[type] < 1);
				g_persons_lose(g,type,1);
				if (g == yourgroup) printf("%s starves!\n",persontypes[type].name);
			}
		}
	}
	if (groups[yourgroup].items[itemtype_food] == 0) printf("you have no food left!\n");
	if (youstarve) {
			printf("you starve to death!!!\n");
			groups[yourgroup].persons[persontype_you] = 0;
			yourgroup = -1;
			persontypes[persontype_you].unique_inplay = FALSE;
			persontypes[persontype_you].unique_permgone = TRUE;
			gameover();
	}
	for (g = 0 ; g < GROUPS ; g++) {
		if (!groups[g].used) continue;
		destroy_group_if_nobody(g);
	}
}

int get_biggest_persontype_in_group(s16int groupid) {
	int biggest_pt, pt;
	
	biggest_pt = 0;	
	for (pt = 0 ; pt < PERSONTYPES ; pt++) {
		if (pt == 0) {
			biggest_pt = pt;
		} else {
			if (groups[groupid].persons[pt] > groups[groupid].persons[biggest_pt]) {
				biggest_pt = pt;
			}
		}
	}
	return biggest_pt;
}

void random_attacks() {
}

void random_moves() {
	int g, xrel, yrel, destx, desty, x, y;
	bool isemptyadjacentspot;
	
	for (g = 0 ; g < GROUPS ; g++) {
		if (!groups[g].used) continue;
		if ((g == yourgroup) || (groups[g].owner == org_you)) continue;
		if (!does_group_have_people(g)) continue;
		if (rollforsuccess(5,10)) continue;
		isemptyadjacentspot = FALSE;
		for (x = groups[g].x - 1 ; x <= groups[g].x + 1; x++) {
			for (y = groups[g].y - 1 ; y <= groups[g].y + 1; y++) {
				if (!is_valid_xy(x,y)) continue;
				if (is_tooclosetoedge(x,y)) continue;
				if (areas[x][y].groupid == -1) {
					isemptyadjacentspot = TRUE;
					break;
				}
			}
			if (isemptyadjacentspot) break;
		}
		if (!isemptyadjacentspot) continue;
		/*printf("will move group %i in (%hi,%hi)\n",g,groups[g].x,groups[g].y);*/
		do {
			xrel = get_rnd_u16int_with_notinclusive_max(3) - 1;
			yrel = get_rnd_u16int_with_notinclusive_max(3) - 1;
			destx = groups[g].x + xrel;
			desty = groups[g].y + yrel;
			/*printf("   ... how about to (%hi + %i, %hi + %i)?\n",groups[g].x,xrel,groups[g].y,yrel);*/
		} while (!is_valid_xy(destx,desty) || is_tooclosetoedge(destx,desty) || (areas[destx][desty].groupid != -1));
		areas[groups[g].x][groups[g].y].groupid = -1;
		groups[g].x = destx;
		groups[g].y = desty;
		areas[destx][desty].groupid = g;
	}
}

void tick() {
	printf("time passes...%s\n",(moving?" (while moving)":""));
	random_attacks();
	if (enforce_rndmvmt) random_moves();
	if (moving) hunters_hunt();
	farmers_farm();
	if (enforce_eat) people_eat();
	check_for_victory();
	turn++;
}

void tick_whilemoving() {
	moving = TRUE;
	tick();
	moving = FALSE;
}

void moverel(s16int xrel, s16int yrel) {
	s16int newx, newy;
	char tmp[200]="";
	int i;
	
	newx = yourx + xrel;
	newy = youry + yrel;
	
	if (!is_valid_xy(newx,newy)) {
		msg("you can't move there! it would be located somewhere outside of the known universe!");
		return;
	}
	
	if (is_tooclosetoedge(newx,newy)) {
		msg("you can't move there! it would be too close to the edge of the world!");
		return;
	}
	
	if (areas[newx][newy].groupid != -1) {
		msg("you can't move there! it's already occupied!");
		return;
	}
	
	if (terrains[areas[newx][newy].terraintype].impassable) {
		sprintf(tmp,"you cannot move into %s terrain!",terrains[areas[newx][newy].terraintype].name);
		msg(tmp);
		return;
	}
	
	areas[groups[yourgroup].x][groups[yourgroup].y].groupid = -1;
	yourx = newx;
	youry = newy;
	groups[yourgroup].x = newx;
	groups[yourgroup].y = newy;
	areas[groups[yourgroup].x][groups[yourgroup].y].groupid = yourgroup;
	
	for (i = 0 ; i < terrains[areas[newx][newy].terraintype].movetickcost ; i++)
		tick_whilemoving();
}

void handle_F() {
	if (groups[yourgroup].items[itemtype_food] < 16000) groups[yourgroup].items[itemtype_food] *= 2;
	if (groups[yourgroup].items[itemtype_food] == 0) groups[yourgroup].items[itemtype_food] = 1;
}

void handle_Ddai() {
	int it;
	
	for (it = 0 ; it < ITEMTYPES ; it++) {
		if (itemtypes[it].unique) continue;
		if (groups[yourgroup].items[it] < 16000) groups[yourgroup].items[it] *= 2;
		if (groups[yourgroup].items[it] == 0) groups[yourgroup].items[it] = 1;
	}
}

void handle_Ddap() {
	int pt;
	
	for (pt = 0 ; pt < PERSONTYPES ; pt++) {
		if (persontypes[pt].unique) continue;
		if (groups[yourgroup].persons[pt] < 16000) groups[yourgroup].persons[pt] *= 2;
		if (groups[yourgroup].persons[pt] == 0) groups[yourgroup].persons[pt] = 1;
	}
}

u32int get_combatvalue_of_group(s16int groupid) {
	int pt, it, qty;
	u32int cv;
	s32int max_weaponusers, actual_weaponusers, unassigned_weaponusers;
	
	cv = 0;	
	for (pt = 0 ; pt < PERSONTYPES ; pt++) {
		cv += groups[groupid].persons[pt] * persontypes[pt].combatvalue;
	}
	
	max_weaponusers = get_people_count(groupid);
	actual_weaponusers = 0;
	unassigned_weaponusers = max_weaponusers;
	
	/*if (groupid==1) printf("before guns: max %li, act %li, una %li\n",max_weaponusers, actual_weaponusers, unassigned_weaponusers);*/
	
	/* Guns & Bullets */
	if (groups[groupid].items[itemtype_bullet] < groups[groupid].items[itemtype_gun])
		qty = groups[groupid].items[itemtype_bullet];
	else qty = groups[groupid].items[itemtype_gun];
	qty = ((qty<unassigned_weaponusers)?qty:unassigned_weaponusers);
	actual_weaponusers += qty;
	unassigned_weaponusers = max_weaponusers - actual_weaponusers;
	/*if (groupid==1) printf("cv before guns/bullets %lu\n",cv);*/
	cv += qty * itemtypes[itemtype_gun].combatvalue;
	/*if (groupid==1) printf("cv after guns/bullets %lu\n",cv);*/
	
	/*if (groupid==1) printf("before bows: max %li, act %li, una %li\n",max_weaponusers, actual_weaponusers, unassigned_weaponusers);*/
	
	/* Bows & Arrows */
	if (groups[groupid].items[itemtype_arrow] < groups[groupid].items[itemtype_bow])
		qty = groups[groupid].items[itemtype_arrow];
	else qty = groups[groupid].items[itemtype_bow];
	qty = ((qty<unassigned_weaponusers)?qty:unassigned_weaponusers);
	actual_weaponusers += qty;
	unassigned_weaponusers = max_weaponusers - actual_weaponusers;
	/*if (groupid==1) printf("cv before bows/arrows %lu\n",cv);*/
	cv += qty * itemtypes[itemtype_bow].combatvalue;
	/*if (groupid==1) printf("cv after bows/arrows %lu\n",cv);*/
	
	
	/*if (groupid==1) printf("before item loop: max %li, act %li, una %li\n",max_weaponusers, actual_weaponusers, unassigned_weaponusers);*/
	
	/* below sucks because it would be better to iterate through a sorted array */
	/* of itemtypes, sorted with highest combatvalue weapons first, that way */
	/* your people will try to use the best weapons first, which is common */
	/* sense */
	/* below sucks because it assumes that all types with >0 cv are wieldable, */
	/* handheld, one-person-per-item weapons...what about things that add to */
	/* your CV but don't have to be wielded? can be worn? or can have multiple */
	/* per person? or don't matter how many people you have? */
	for (it = 0 ; it < ITEMTYPES ; it++) {
		if (it == itemtype_gun || it == itemtype_bow) continue;
		if (itemtypes[it].combatvalue == 0) continue;
		/*if ((groupid==1) && (it==itemtype_knife)) printf("max %li, act %li, una %li\n",max_weaponusers, actual_weaponusers, unassigned_weaponusers);*/
		qty = groups[groupid].items[it];
		qty = ((qty<unassigned_weaponusers)?qty:unassigned_weaponusers);
		actual_weaponusers += qty;
		unassigned_weaponusers = max_weaponusers - actual_weaponusers;
		/*if ((groupid==1) && (it==itemtype_knife)) printf("cv before knives %lu\n",cv);*/
		/*if ((groupid==1) && (it==itemtype_knife)) printf("about to add qty %i * cv %hu\n",qty,itemtypes[it].combatvalue);*/
		cv += qty * itemtypes[it].combatvalue;
		/*if ((groupid==1) && (it==itemtype_knife)) printf("cv after knives %lu\n",cv);*/
	}
	
	return cv;
}

void handle_foo() {
	int x, y, g, p, fx, fy, i, j, padcount, pt, it;
	char tmp[2000] = "", tmp2[2000] = "", tmp3[2000] = "";
	char fields[3][3][501];
	int fieldptr[3][3];
	bool first, gpt_first, git_first, pit_first;
	bool alllinesofallfieldsinthisrowdone;
	char *chptr;
	int numcharsactuallyprinted;
	
	/* Wipe */
	for (fx = 0 ; fx < 3 ; fx++) {
		for (fy = 0 ; fy < 3 ; fy++) {
			strcpy(fields[fx][fy],"");
			fieldptr[fx][fy] = 0;
		}
	}
	
	/* Populate */
	for (y = youry - 1, fy = 0 ; y <= youry + 1 ; y++, fy++) {
		for (x = yourx - 1, fx = 0 ; x <= yourx + 1 ; x++, fx++) {
			first = TRUE;
			if (areas[x][y].groupid == -1) {
				/*sprintf(tmp,"no group");
				strcat(fields[fx][fy],tmp);*/
			} else {
				g = areas[x][y].groupid;
				sprintf(tmp,"%-8.8s %5lip %5lii", orgs[groups[g].owner].name, get_people_count(g), get_items_count(g));
				/*if ((fx == 1) && (fy == 1)) printf("tmp = \"%s\"\n",tmp);*/
				if (first) first = FALSE;
				else strcat(fields[fx][fy],"; ");
				strcat(fields[fx][fy],tmp);
				
				/*if ((fx == 1) && (fy == 1)) printf("fields[1][1] is now \"%s\"\n",fields[fx][fy]);*/
				
				strcpy(tmp,"");
				gpt_first = TRUE;
				for (pt = 0 ; pt < PERSONTYPES ; pt++) {
					if (groups[g].persons[pt] > 0) {
						sprintf(tmp2,"%s%hi%1.1s", (gpt_first?"p: ":" "), groups[g].persons[pt], persontypes[pt].name);
						strcat(tmp,tmp2);
						if (gpt_first) gpt_first = FALSE;
					}
				}
				if (first) first = FALSE;
				else strcat(fields[fx][fy],"; ");
				strcat(fields[fx][fy],tmp);
				
				/*if ((fx == 1) && (fy == 1)) printf("fields[1][1] is now \"%s\"\n",fields[fx][fy]);*/
				
				strcpy(tmp,"");
				git_first = TRUE;
				for (it = 0 ; it < ITEMTYPES ; it++) {
					if (groups[g].items[it] > 0) {
						sprintf(tmp2,"%s%hi%1.1s", (git_first?"i: ":" "), groups[g].items[it], itemtypes[it].name);
						strcat(tmp,tmp2);
						if (git_first) git_first = FALSE;
					}
				}
				if (first) first = FALSE;
				else strcat(fields[fx][fy],"; ");
				strcat(fields[fx][fy],tmp);
			}
			
			if (areas[x][y].pileid != -1) {
				p = areas[x][y].pileid;
				strcpy(tmp,"");
				pit_first = TRUE;
				for (it = 0 ; it < ITEMTYPES ; it++) {
					if (piles[p].items[it] > 0) {
						sprintf(tmp2,"%s%hi%1.1s", (pit_first?"pile: ":" "), piles[p].items[it], itemtypes[it].name);
						strcat(tmp,tmp2);
						if (pit_first) pit_first = FALSE;
					}
				}
				if (first) first = FALSE;
				else strcat(fields[fx][fy],"; ");
				strcat(fields[fx][fy],tmp);
			}
			
			/*if ((fx == 1) && (fy == 1)) printf("fields[1][1] is now \"%s\"\n",fields[fx][fy]);*/
			sprintf(tmp,"%s",terrains[areas[x][y].terraintype].name);
			if (first) first = FALSE;
			else strcat(fields[fx][fy],";  ");
			strcat(fields[fx][fy],tmp);
		}
	}
	
	/* Pack/Fill to 25 Wide Each*/
	for (fx = 0 ; fx < 3 ; fx++) {
		for (fy = 0 ; fy < 3 ; fy++) {
			if (strlen(fields[fx][fy]) < 50) {
				padcount = 50 - strlen(fields[fx][fy]);
				for (j = 0 ; j < padcount ; j++) {
					strcat(fields[fx][fy]," ");
				}
			}
		}
	}
	
	/* Display */
	for (fy = 0 ; fy < 3 ; fy++) {
		alllinesofallfieldsinthisrowdone = FALSE;
		for (i = 0 ; !alllinesofallfieldsinthisrowdone ; i++) {
			for (fx = 0 ; fx < 3 ; fx++) {
				/*if ((fx == 1) && (fy == 1)) printf("display: fields[1][1] is now \"%s\"\n",fields[fx][fy]);*/
				strcpy(tmp,"");
				strcpy(tmp2,"");
				strcpy(tmp3,"");
				numcharsactuallyprinted = 25;
				if (fieldptr[fx][fy] < strlen(fields[fx][fy])) {
					if (((fieldptr[fx][fy] + 25 + 1) < strlen(fields[fx][fy])) &&
						(fields[fx][fy][fieldptr[fx][fy]+24] != ' ')
							&&
							(fields[fx][fy][fieldptr[fx][fy]+25] != ' '))
					{
						/*if ((fx == 1) && (fy == 1))*/ /*printf("\nCause for special wrap! (i %i)\n",i);*/
						/*printf("because ch[rel24] is '%c' and ch[rel25] is '%c'\n",fields[fx][fy][fieldptr[fx][fy]+24],fields[fx][fy][fieldptr[fx][fy]+25]);*/
						strncpy(tmp3,&fields[fx][fy][fieldptr[fx][fy]],25);
						strcat(tmp3,"");
						/*printf("tmp3 is \"%s\"\n",tmp3);*/
						chptr = strrchr(tmp3,' ');
						if (chptr == NULL) {
							printf("\nThis shouldn't happen!\nin handle_foo(), a call to strrchr() returned NULL\n");
							exit(EXIT_FAILURE);
						}
						/*printf("chptr (last space) is \"%1.1s\" at +%i\n",chptr,(chptr - tmp3));*/
						/*printf("tmp before is \"%s\"\n",tmp);*/
						/*printf("about to copy up to %i chars from tmp3 to tmp\n",(chptr - tmp3) + 1);*/
						strncpy(tmp,tmp3, (chptr - tmp3) + 1);
						strcat(tmp,"");
						/*printf("\ntmp is now \"%s\"\n",tmp);*/
						numcharsactuallyprinted = (chptr - tmp3) + 1;
						/*printf("\nnumcharsactuallyprinted = %i\n",numcharsactuallyprinted);*/
						tmp[numcharsactuallyprinted] = 0; /*HACK!*/
					} else {
						strcpy(tmp,&fields[fx][fy][fieldptr[fx][fy]]);
					}
					printf("%-25.25s",tmp);
					numcharsactuallyprinted = strlen(tmp);
					/*if ((fx == 1) && (fy == 1))printf("\nsetting numcharsactuallyprinted to strlen(tmp) which is %i\n",numcharsactuallyprinted);*/
					if (numcharsactuallyprinted > 25) numcharsactuallyprinted = 25;
				} else printf("                         ");
				fieldptr[fx][fy] += numcharsactuallyprinted;
				if (fx != 2) printf("|");
			}
			printf("x\n");
			alllinesofallfieldsinthisrowdone = TRUE;
			for (fx = 0 ; fx < 3 ; fx++) {
				if (fieldptr[fx][fy] < strlen(fields[fx][fy])) {
					alllinesofallfieldsinthisrowdone = FALSE;
					break;
				}
			}
			/*printf("\ni++\n");*/
		}
		if (fy != 2) printf("-------------------------+-------------------------+--------------------------\n");
	}
}

void handle_D() {
	int g, p;
	u32int totalcities, yourcities;
	float yourpercent;
	bool foundbrief;
	s32int gcount, pcount;
	/*s16int nearest_ratg;*/
	
	printf("turn %lu\n",turn);
	printf("real coords (%hi,%hi)\n",yourx,youry);
	
	gcount = 0;
	for (g = 0 ; g < GROUPS ; g++) {
		if (groups[g].used) gcount++;
	}
	printf("groups: %li\n",gcount);
	
	pcount = 0;
	for (p = 0 ; p < PILES ; p++) {
		if (piles[p].used) pcount++;
	}
	printf("piles: %li\n",pcount);
	
	/*nearest_ratg = -1;
	for (g = 0 ; g < GROUPS ; g++) {
		if (!groups[g].used) continue;
		if (groups[g].persons[persontype_rat] == 0) continue;
		if (nearest_ratg == -1) {
			nearest_ratg = g;
		} else {
			if (get_dist(yourx,youry,groups[g].x,groups[g].y) < get_dist(yourx,youry,groups[nearest_ratg].x,groups[nearest_ratg].y)) {
				nearest_ratg = g;
			}
		}
	}
	if (nearest_ratg == -1) printf("there are no rat groups\n");
	else printf("nearest rat group is %hi in (%hi,%hi), at dist %hi\n",
		nearest_ratg,groups[nearest_ratg].x,groups[nearest_ratg].y,
		get_dist(yourx,youry,groups[nearest_ratg].x,groups[nearest_ratg].y));*/
		
	printf("your group's combatvalue: %lu\n",get_combatvalue_of_group(yourgroup));
	
	calc_city_vals(&yourcities,&totalcities,&yourpercent);
	printf("cities: %lu / %lu  = %hu%%  (need %hu%%)\n",yourcities,totalcities,(u16int)(yourpercent*100),(u16int)(CITY_OWN_PER_TO_WIN*100));
	
	foundbrief = FALSE;
	p = -1;
	for (g = 0 ; g < GROUPS ; g++) {
		if (!groups[g].used) continue;
		if (groups[g].items[itemtype_briefcase_of_power] > 0) {
			foundbrief = TRUE;
			break;
		}
	}
	if (!foundbrief) {
		g = -1;
		for (p = 0 ; p < PILES ; p++) {
			if (!piles[p].used) continue;
			if (piles[p].items[itemtype_briefcase_of_power] > 0) {
				foundbrief = TRUE;
				break;
			}
		}
		if (!foundbrief) p = -1;
	}
	if (foundbrief) {
		if (g == yourgroup) {
			printf("your group has the briefcase of power!\n");
		} else {
			printf("briefcase of power in (%hi,%hi)\n",
				(g==-1?piles[p].x:groups[g].x),(g==-1?piles[p].y:groups[g].y));
		}
	} else printf("briefcase of power not found!\n");
}

void handle_rp() {
	int pt;
	
	for (pt = 1 ; pt < PERSONTYPES ; pt++) {
		if (persontypes[pt].known) printf("%20s - %s\n",persontypes[pt].name,(allowtojoin[pt]?"yes":"no"));
	}	
}

void handle_allowall() {
	int pt;
	
	printf("you will now allow all persons types when recruiting\n");
	for (pt = 1 ; pt < PERSONTYPES ; pt++) {
		allowtojoin[pt] = TRUE;
	}	
}

void handle_refuseall() {
	int pt;
	
	printf("you will now refuse all persons types when recruiting\n");
	for (pt = 1 ; pt < PERSONTYPES ; pt++) {
		allowtojoin[pt] = FALSE;
	}	
}

void handle_pt() {
	int pt;
	
	for (pt = 0 ; pt < PERSONTYPES ; pt++) {
		if (persontypes[pt].known) printf("%2i - %s\n",pt,persontypes[pt].name);
	}
}

void handle_Dpt() {
	int pt;

	printf("id name           nameplur        desc     freq un ip pg an dn kn cv sy\n");	
	for (pt = 0 ; pt < PERSONTYPES ; pt++) {
		printf("%2i %-14s %-15s %-5.5s... %2lu   %1u  %1u  %1u  %1u  %1u  %1u  %2u %c\n", pt, persontypes[pt].name,
			persontypes[pt].nameplural, persontypes[pt].desc,
			persontypes[pt].basefrequency, persontypes[pt].unique,
			persontypes[pt].unique_inplay, persontypes[pt].unique_permgone,
			persontypes[pt].is_animal, persontypes[pt].is_dangerous,
			persontypes[pt].known, persontypes[pt].combatvalue, persontypes[pt].symbol);
	}
}

void handle_people() {
	int pt;
	
	for (pt = 0 ; pt < PERSONTYPES ; pt++) {
		if (groups[yourgroup].persons[pt] < 1) continue;
		if (pt == persontype_you) printf("%s\n",persontypes[pt].name);
		else printf("%4hi %s\n",groups[yourgroup].persons[pt],(groups[yourgroup].persons[pt]>1?persontypes[pt].nameplural:persontypes[pt].name));
	}
}

void handle_items() {
	int it;
	
	for (it = 0 ; it < ITEMTYPES ; it++) {
		if (groups[yourgroup].items[it] < 1) continue;
		printf("%5hi %s\n",groups[yourgroup].items[it],(groups[yourgroup].items[it]>1?itemtypes[it].nameplural:itemtypes[it].name));
	}
}

void handle_it() {
	int it;
	
	for (it = 0 ; it < ITEMTYPES ; it++) {
		if (itemtypes[it].known) printf("%2i - %s\n",it,itemtypes[it].name);
	}
}

void handle_Dit() {
	int it;

	printf("id name               nameplur            desc     freq un ip pg kn cv\n");
	for (it = 0 ; it < ITEMTYPES ; it++) {
		printf("%2i %-18s %-19s %-5.5s... %2lu   %1u  %1u  %1u  %1u  %2u\n", it, itemtypes[it].name,
			itemtypes[it].nameplural, itemtypes[it].desc, itemtypes[it].basefrequency,
			itemtypes[it].unique, itemtypes[it].unique_inplay,
			itemtypes[it].unique_permgone, itemtypes[it].known,
			itemtypes[it].combatvalue);
	}
}

void handle_intro() {
	printf("They said it could never happen. World-wide thermonuclear war. "\
		"That mankind had finally reached a point in history where the unthinkable was also impossible.\n"\
		"But they were wrong. The missiles flew and devastated the face of the Earth.\n"\
		"Civilization as we knew it had ended.\n"\
		"Years later a young road warrior emerges from a ragged refugee camp near Danveer on the " CONTINENT_NAME_PLURAL " land mass. "\
		"He is the only hope of his people. And he is you.\n\n"\
		"Welcome to Apocalypse Dawn!\nYour goal is to survive and perhaps rebuild civilization. "\
		"You start with a gun and some food, and an itch to explore this dangerous new world...\n");
}

void handle_w() {
	printf("you wait for a day, doing nothing...\n");
	tick();
}

/*this isn't handling unique's completely correctly yet*/
void handle_r() {
	s16int max_num_to_find;
	u16int num_found, type;
	int i, pt, last_viable_pt;
	u32int total_freqs, freq;
	u32int min_for_match[PERSONTYPES], max_for_match[PERSONTYPES], freqroll;
	s32int foo;
	bool this_cant_match[PERSONTYPES];
	
	foo = get_your_people_count();
	if (foo > 32000) foo = 32000;
	max_num_to_find = foo;
	max_num_to_find /= 2;
	if (max_num_to_find < 1) max_num_to_find = 1;
	num_found = get_rnd_u16int_with_notinclusive_max(max_num_to_find+1);
	if (num_found == 0) printf("you can't find anybody!\n");
	
	for (pt = 1 ; pt < PERSONTYPES ; pt++) {
		min_for_match[pt] = 0;
		max_for_match[pt] = 0;
		this_cant_match[pt] = FALSE;
	}
	last_viable_pt = -1;	
	for (pt = 1 ; pt < PERSONTYPES ; pt++) {
		if (persontypes[pt].is_dangerous) continue;
		if (persontypes[pt].is_animal) continue;
		if (last_viable_pt == -1) {
			min_for_match[pt] = 0;
		} else {
			min_for_match[pt] = max_for_match[last_viable_pt] + 1;
		}
		last_viable_pt = pt;
		freq = (u32int)((float)persontypes[pt].basefrequency * persontypes[pt].terrfreqmultiplier[areas[yourx][youry].terraintype]);
		if (freq == 0) this_cant_match[pt] = TRUE;
		max_for_match[pt] = min_for_match[pt] + freq;
		/*remember to further modify the frequency for "nature of your party" effects, etc.*/
		/*remember to make same mods to "total_pt_freqs" calc below*/
	}
	
	total_freqs = 0;
	for (pt = 1 ; pt < PERSONTYPES ; pt++) {
		if (persontypes[pt].is_dangerous) continue;
		if (persontypes[pt].is_animal) continue;
		freq = (u32int)((float)persontypes[pt].basefrequency * persontypes[pt].terrfreqmultiplier[areas[yourx][youry].terraintype]);
		total_freqs += freq;
	}
	
	for (i = 0 ; i < num_found ; i++) {
		freqroll = get_rnd_u32int_with_notinclusive_max(total_freqs);
		for (pt = 1 ; pt < PERSONTYPES ; pt++) {
			if (persontypes[pt].is_dangerous) continue;
			if (persontypes[pt].is_animal) continue;
			if (this_cant_match[pt]) continue;
			if ((freqroll >= min_for_match[pt]) && (freqroll <= max_for_match[pt])) {
				type = pt;
				break;
			}
		}
		
		if (!allowtojoin[type]) {
			printf("you turn away a %s\n",persontypes[type].name);
			continue;
		}

		g_persons_add(yourgroup,type,1);
		printf("%s (%hu) joins!\n",persontypes[type].name,type);
	}

	tick();
}

void handle_l() {
	u16int itemtype, qty, maxqty;
	s32int total_people;
	int it;	
	u32int total_freqs, freq;
	u32int min_for_match[ITEMTYPES], max_for_match[ITEMTYPES], freqroll;
	bool this_cant_match[ITEMTYPES];
	
	
	if (rollforsuccess(1,3)) {
	
		for (it = 0 ; it < ITEMTYPES ; it++) {
			min_for_match[it] = 0;
			max_for_match[it] = 0;
			this_cant_match[it] = FALSE;
		}
	
		for (it = 0 ; it < ITEMTYPES ; it++) {
			if (it == 0) {
				min_for_match[it] = 0;
			} else {
				min_for_match[it] = max_for_match[it-1] + 1;
			}
			freq = (u32int)((float)itemtypes[it].basefrequency * itemtypes[it].terrfreqmultiplier[areas[yourx][youry].terraintype]);
			if (freq == 0) this_cant_match[it] = TRUE;
			max_for_match[it] = min_for_match[it] + freq;
			/*remember to further modify the frequency for "nature of your party" effects, etc.*/
			/*remember to make same mods to "total_pt_freqs" calc below*/
		}
	
		total_freqs = 0;
		for (it = 0 ; it < ITEMTYPES ; it++) {
			freq = (u32int)((float)itemtypes[it].basefrequency * itemtypes[it].terrfreqmultiplier[areas[yourx][youry].terraintype]);
			total_freqs += freq;
		}
	
		do {
			freqroll = get_rnd_u32int_with_notinclusive_max(total_freqs);
			for (it = 0 ; it < ITEMTYPES ; it++) {
				if (this_cant_match[it]) continue;
				if ((freqroll >= min_for_match[it]) && (freqroll <= max_for_match[it])) {
					itemtype = it;
					break;
				}
			}
		} while (itemtypes[itemtype].unique && (itemtypes[itemtype].unique_inplay || itemtypes[itemtype].unique_permgone));
		
		maxqty = 10;
		total_people = get_your_people_count();
		if (total_people >= 2)   maxqty *= 2;
		if (total_people >= 5)   maxqty *= 2;
		if (total_people >= 10)  maxqty *= 2;
		if (total_people >= 25)  maxqty *= 2;
		if (total_people >= 50)  maxqty *= 2;
		if (total_people >= 100) maxqty *= 2;
		if (total_people >= 250) maxqty *= 2;
		if (areas[yourx][youry].terraintype == terraintype_urban) maxqty *= 2;
		if (itemtype == itemtype_gun) maxqty /= 2;
		qty = 1 + get_rnd_u16int_with_notinclusive_max(maxqty);
		g_items_add(yourgroup,itemtype,qty);
		printf("you found %hu %s!\n",qty,(qty>1?itemtypes[itemtype].nameplural:itemtypes[itemtype].name));
	} else printf("you can't find anything!\n");
	tick();
}

void handle_t() {
	if (areas[yourx][youry].terraintype != terraintype_urban) {
		printf("there is no city here for you to takeover!\n");
		return;
	}
	
	printf("attempting to takeover city...\n");
	
	if (areas[yourx][youry].youown) {
		printf("according to reliable intelligence reports, this is in fact already your city!\n");
		return;
	}
	
	if (rollforsuccess(1,2)) {
		printf("you successfully takeover the city!\n");
		areas[yourx][youry].youown = TRUE;
	} else {
		printf("your attempt to takeover the city has failed!\n");
	}
	tick();
}

void handle_c() {
	u32int totalcities, yourcities;
	float yourpercent;
	
	calc_city_vals(&yourcities,&totalcities,&yourpercent);
	printf("you control %lu cit%s in " CONTINENT_NAME "\n",yourcities,(yourcities==1?"y":"ies"));
	if (yourpercent < (CITY_OWN_PER_TO_WIN*6)/10) {
		printf("this is probably not enough to win the game\n");
	} else printf("you have a feeling this might be getting close to what you need to the win the game, but you're not sure\n");
}

void handle_e() {
	int it;
	s16int p;
	
	printf("you examine the area...\n");
	if (areas[yourx][youry].pileid != -1) {
		printf("there's a pile of stuff here! namely:\n");
		p = areas[yourx][youry].pileid;
		for (it = 0 ; it < ITEMTYPES ; it++) {
			if (piles[p].items[it] > 0) {
				itemtypes[it].known = TRUE;
				printf("%5hi %s\n",piles[p].items[it],(piles[p].items[it]>1?itemtypes[it].nameplural:itemtypes[it].name));
			}
		}
	}
}

void handle_p() {
	int it;
	s16int p;
	
	if (areas[yourx][youry].pileid == -1) {
		printf("there's nothing here to pickup!\n");
		return;
	}
	
	p = areas[yourx][youry].pileid;
	
	for (it = 0 ; it < ITEMTYPES ; it++) {
		if (piles[p].items[it] < 1) continue;
		printf("picking up %hu %s...\n",piles[p].items[it],(piles[p].items[it]>1?itemtypes[it].nameplural:itemtypes[it].name));
		g_items_add(yourgroup,it,piles[p].items[it]);
		piles[p].items[it] = 0;
	}

	destroy_pile_if_empty(p);
	
	if (groups[yourgroup].items[itemtype_briefcase_of_power] > 0) {
		printf("holy cow! you now possess the legendary Briefcase of Power!\n");
		printf("you win the game!\n");
		printf("you may now retire at any time, or, continue playing...\n");
		gamewon = TRUE;
	}
}

bool is_valid_persontype(s16int pt) {
	if ((pt >= 0) && (pt < PERSONTYPES)) return TRUE;
	else return FALSE;
}

bool is_valid_itemtype(s16int it) {
	if ((it >= 0) && (it < ITEMTYPES)) return TRUE;
	else return FALSE;
}

void describe_area(s16int x, s16int y) {
	int pt, it;
	s16int g, p;
	
	printf("the terrain is %s\n",terrains[areas[x][y].terraintype].name);
	if (areas[x][y].groupid != -1) {
		g = areas[x][y].groupid;
		if (groups[g].owner == org_you) {
			printf("there's a group of your people there!\n");
		} else if (groups[g].owner == org_independent) {
			printf("there's a group of strangers there!\n");
		}	else if (groups[g].owner == org_enemy) {
			printf("there's a group of enemies there!\n");
		}	else {
			printf("there's a group of unknown people there!\n");
		}
		for (pt = 0 ; pt < PERSONTYPES ; pt++) {
			if (groups[g].persons[pt] > 0) {
				persontypes[pt].known = TRUE;
				printf("%5hi %s\n",groups[g].persons[pt],(groups[g].persons[pt]>1?persontypes[pt].nameplural:persontypes[pt].name));
			}
		}
		for (it = 0 ; it < ITEMTYPES ; it++) {
			if (groups[g].items[it] > 0) {
				itemtypes[it].known = TRUE;
				printf("%5hi %s\n",groups[g].items[it],(groups[g].items[it]>1?itemtypes[it].nameplural:itemtypes[it].name));
			}
		}
	}
	
	if (areas[x][y].pileid != -1) {
		printf("there's a pile of stuff there!\n");
		p = areas[x][y].pileid;
		for (it = 0 ; it < ITEMTYPES ; it++) {
			if (piles[p].items[it] > 0) {
				itemtypes[it].known = TRUE;
				printf("%5hi %s\n",piles[p].items[it],(piles[p].items[it]>1?itemtypes[it].nameplural:itemtypes[it].name));
			}
		}
	}
}

void handle_g(char *inputline) {
	int dirnum, xrel, yrel;
	
	dirnum = get_nth_token_as_int(inputline,1);
	if ((dirnum < 1) || (dirnum > 9) || (dirnum == 5)) {
		printf("%i is not a valid direction number, try 1-4 or 6-9!\n",dirnum);
		return;
	}
	
	xrel = dir2xrel[dirnum];
	yrel = dir2yrel[dirnum];
	
	if (!is_valid_xy(yourx+xrel,youry+yrel)) {
		printf("the place you want to look doesn't exist in the real universe,"\
			" only in an imaginary undefined one, therefore I don't know how to"\
			" tell you what is there.\n");
		return;
	}
	
	printf("glancing to the %s:\n",dirnames[dirnum]);
	describe_area(yourx+xrel,youry+yrel);
}

void attack(s16int attgroup, s16int defgroup) {
	int pt;
	u32int att_cv, def_cv;
	s16int ammoshot, g;
	s32int max_weaponusers, actual_weaponusers, unassigned_weaponusers;
	s16int wingroup, losegroup;
	bool youdie;
	
	if (defgroup == yourgroup) printf("your group is attacked!\n");
	att_cv = get_combatvalue_of_group(attgroup);
	def_cv = get_combatvalue_of_group(defgroup);
	
	g = attgroup;
	do {
		max_weaponusers = get_people_count(g);
		actual_weaponusers = 0;
		unassigned_weaponusers = max_weaponusers;
	
		/* Guns & Bullets */
		if (groups[g].items[itemtype_bullet] < groups[g].items[itemtype_gun])
			ammoshot = groups[g].items[itemtype_bullet];
		else ammoshot = groups[g].items[itemtype_gun];
		ammoshot = ((ammoshot<unassigned_weaponusers)?ammoshot:unassigned_weaponusers);
		actual_weaponusers += ammoshot;
		unassigned_weaponusers = max_weaponusers - actual_weaponusers;
		g_items_lose(g,itemtype_bullet,ammoshot);
		printf("%s expends %hi bullets\n",(g==attgroup?"attacker":"defender"),ammoshot);
	
		/* Bows & Arrows */
		if (groups[g].items[itemtype_arrow] < groups[g].items[itemtype_bow])
			ammoshot = groups[g].items[itemtype_arrow];
		else ammoshot = groups[g].items[itemtype_bow];
		ammoshot = ((ammoshot<unassigned_weaponusers)?ammoshot:unassigned_weaponusers);
		actual_weaponusers += ammoshot;
		unassigned_weaponusers = max_weaponusers - actual_weaponusers;
		g_items_lose(g,itemtype_arrow,ammoshot);
		printf("%s expends %hi arrows\n",(g==attgroup?"attacker":"defender"),ammoshot);
		
		if (g == attgroup) g = defgroup;
		else break;
	} while (TRUE);
	
	if (att_cv > def_cv) {
		wingroup = attgroup;
		losegroup = defgroup;
	} else if (att_cv < def_cv) {
		wingroup = defgroup;
		losegroup = attgroup;
	} else {
		if ((attgroup == yourgroup) || (defgroup == yourgroup)) printf("the attack was a stand-off, neither side had losses\n");
		return;
	}

	for (pt = 0 ; pt < PERSONTYPES ; pt++) {
		if ((groups[losegroup].persons[pt] > 0) && persontypes[pt].unique) {
			persontypes[pt].unique_inplay = FALSE;
			persontypes[pt].unique_permgone = TRUE;
		}
		groups[losegroup].persons[pt] = 0;
	}
	youdie = FALSE;
	if (wingroup == yourgroup) printf("your groups wins! other guys destroyed!\n");
	else if (losegroup == yourgroup) {
		printf("your group lost! it was destroyed!\n");
		printf("you died in the battle!!!\n");
		youdie = TRUE;
	}
	destroy_group_if_nobody(losegroup);
	if (youdie) {
		gameover();
	}
}

void handle_a(char *inputline) {
	int dirnum, xrel, yrel, x, y;
	
	dirnum = get_nth_token_as_int(inputline,1);
	if ((dirnum < 1) || (dirnum > 9) || (dirnum == 5)) {
		printf("%i is not a valid direction number, try 1-4 or 6-9!\n",dirnum);
		return;
	}
	
	xrel = dir2xrel[dirnum];
	yrel = dir2yrel[dirnum];
	
	x = yourx + xrel;
	y = youry + yrel;
	
	if (!is_valid_xy(x,y)) {
		printf("the place you want to look doesn't exist in the real universe!\n");
		return;
	}
	
	if (areas[x][y].groupid == -1) {
		printf("there's nobody there to attack!\n");
		return;
	}
	
	if (groups[areas[x][y].groupid].owner == org_you) {
		printf("you can't attack one of your groups!\n");
		return;
	}
	
	printf("attacking to the %s:\n",dirnames[dirnum]);
	attack(yourgroup,areas[x][y].groupid);
}

void handle_Dcv(char *inputline) {
	int dirnum, xrel, yrel, x, y;
	
	dirnum = get_nth_token_as_int(inputline,1);
	if ((dirnum < 1) || (dirnum > 9) || (dirnum == 5)) {
		printf("%i is not a valid direction number, try 1-4 or 6-9!\n",dirnum);
		return;
	}
	
	xrel = dir2xrel[dirnum];
	yrel = dir2yrel[dirnum];
	
	x = yourx + xrel;
	y = youry + yrel;
	
	if (!is_valid_xy(x,y)) {
		printf("the place you want to look doesn't exist in the real universe!\n");
		return;
	}
	
	if (areas[x][y].groupid == -1) {
		printf("there's nobody there to attack!\n");
		return;
	}
	
	printf("combatvalue of group to %s is %lu\n",dirnames[dirnum],get_combatvalue_of_group(areas[x][y].groupid));
}

void handle_dp(char *inputline) {
	int type;
	
	type = get_nth_token_as_int(inputline,1);
	if (!is_valid_persontype(type)) {
		printf("%i is not a valid persontype!\n",type);
		return;
	}
	
	if (!persontypes[type].known) {
		printf("you have no knowledge of that persontype!\n");
		return;
	}
	
	printf("%s\n",persontypes[type].desc);
}

void handle_di(char *inputline) {
	int type;
	
	type = get_nth_token_as_int(inputline,1);
	if (!is_valid_itemtype(type)) {
		printf("%i is not a valid itemtype!\n",type);
		return;
	}
	
	if (!itemtypes[type].known) {
		printf("you have no knowledge of that itemtype!\n");
		return;
	}
	
	printf("%s\n",itemtypes[type].desc);
}

void handle_d(char *inputline) {
	int type, qty;
	
	type = get_nth_token_as_int(inputline,1);
	if (!is_valid_persontype(type)) {
		printf("%i is not a valid persontype!\n",type);
		return;
	}
	
	if (type == persontype_you) {
		printf("although %i is a valid persontype, it refers to you -- and you cannot dismiss yourself from your own party! because you're the host of the party! if the host leaves, the party won't be as good! please, have sympathy for the party!\n",type);
		return;
	}
	
	qty = get_nth_token_as_int(inputline,2);
	if (qty <= 0) {
		printf("%i is not a valid qty to dismiss!\n",qty);
		return;
	}
	
	if (qty > groups[yourgroup].persons[type]) {
		printf("you don't have %i persons of that type, only %hi!\n",qty,groups[yourgroup].persons[type]);
		return;
	}
	
	g_persons_lose(yourgroup,type,qty);
	printf("%i %s dismissed from your party!\n",qty,(qty>1?persontypes[type].nameplural:persontypes[type].name));
	if (persontypes[type].unique) {
		persontypes[type].unique_inplay = FALSE;
		persontypes[type].unique_permgone = FALSE;
		printf("you have a funny feeling you'll never see that person again...\n");
	}
}

void handle_th(char *inputline) {
	int type, qty;
	
	type = get_nth_token_as_int(inputline,1);
	if (!is_valid_itemtype(type)) {
		printf("%i is not a valid itemtype!\n",type);
		return;
	}
	
	qty = get_nth_token_as_int(inputline,2);
	if (qty <= 0) {
		printf("%i is not a valid qty!\n",qty);
		return;
	}
	
	if (qty > groups[yourgroup].items[type]) {
		printf("you don't have %i items of that type, only %hi!\n",qty,groups[yourgroup].items[type]);
		return;
	}
	
	g_items_lose(yourgroup,type,qty);
	printf("your party throws away %i %s!\n",qty,(qty>1?itemtypes[type].nameplural:itemtypes[type].name));
	if (itemtypes[type].unique) {
		itemtypes[type].unique_inplay = FALSE;
		itemtypes[type].unique_permgone = FALSE;
		printf("you have a funny feeling it is now lost forever...\n");
	}
}

void handle_De() {
	if (enforce_eat) {
		enforce_eat = FALSE;
	} else {
		enforce_eat = TRUE;
	}
	printf("enforce_eat now %s\n",(enforce_eat?"ON":"OFF"));
}

void handle_Dm() {
	if (enforce_rndmvmt) {
		enforce_rndmvmt = FALSE;
	} else {
		enforce_rndmvmt = TRUE;
	}
	printf("enforce_rndmvmt now %s\n",(enforce_rndmvmt?"ON":"OFF"));
}

void handle_Dxy(char *inputline) {
	int x, y;
	
	x = get_nth_token_as_int(inputline,1);
	if (!is_valid_x(x)) {
		printf("%i is not a valid x!\n",x);
		return;
	}
	
	y = get_nth_token_as_int(inputline,2);
	if (!is_valid_y(y)) {
		printf("%i is not a valid y!\n",y);
		return;
	}
	
	if (areas[x][y].groupid != -1) {
		printf("you can't teleport there because there's a group there!\n");
		return;
	}
	
	areas[groups[yourgroup].x][groups[yourgroup].y].groupid = -1;
	yourx = x;
	youry = y;
	groups[yourgroup].x = x;
	groups[yourgroup].y = y;
	areas[x][y].groupid = yourgroup;	

	printf("your party has teleported to (%i,%i)!\n",x,y);
}

void handle_givep(char *inputline) {
	int type, qty, dir, xrel, yrel, recv_groupid;
	
	type = get_nth_token_as_int(inputline,1);
	if (!is_valid_persontype(type)) {
		printf("%i is not a valid persontype!\n",type);
		return;
	}
	
	qty = get_nth_token_as_int(inputline,2);
	if (qty <= 0) {
		printf("%i is not a valid qty!\n",qty);
		return;
	}
	
	if (qty > groups[yourgroup].persons[type]) {
		printf("you don't have %i persons of that type, only %hi!\n",qty,groups[yourgroup].persons[type]);
		return;
	}
	
    dir = get_nth_token_as_int(inputline,3);
	if ((dir < 1) || (dir > 9) || (dir == 5)) {
		printf("%i is not a valid dir number, must be 1-4 or 6-9!\n",dir);
		return;
	}
	
	xrel = dir2xrel[dir];
	yrel = dir2yrel[dir];
	
	if (!is_valid_xy(yourx+xrel,youry+yrel)) {
		printf("the place you want to transfer to doesn't exist in the real universe\n");
		return;
	}
	
	recv_groupid = get_or_create_group_at(yourx+xrel, youry+yrel,org_you);
	if (recv_groupid < 0) {
		printf("couldn't create new group so transfer failed!\n");
		return;
	} else printf("groupid is %i\n",recv_groupid);
	
	if ((type == persontype_you) && (groups[recv_groupid].owner != org_you)) {
		printf("you cannot transfer youself to a group that isn't yours!\n");
		return;
	}
	
	g_persons_lose(yourgroup, type, qty);
	g_persons_add(recv_groupid, type, qty);
	
	printf("%i %s transferred to group to the %s!\n",
		qty, (qty>1?persontypes[type].nameplural:persontypes[type].name),
		dirnames[dir]);
		
	if (type == persontype_you) {
		yourgroup = recv_groupid;
		yourx = groups[yourgroup].x;
		youry = groups[yourgroup].y;
		tick();
	}
}

void handle_getp(char *inputline) {
	int type, qty, dir, xrel, yrel, groupid;
	
	type = get_nth_token_as_int(inputline,1);
	if (!is_valid_persontype(type)) {
		printf("%i is not a valid persontype!\n",type);
		return;
	}
	
	qty = get_nth_token_as_int(inputline,2);
	if (qty <= 0) {
		printf("%i is not a valid qty!\n",qty);
		return;
	}
	
    dir = get_nth_token_as_int(inputline,3);
	if ((dir < 1) || (dir > 9) || (dir == 5)) {
		printf("%i is not a valid dir number, must be 1-4 or 6-9!\n",dir);
		return;
	}
	
	xrel = dir2xrel[dir];
	yrel = dir2yrel[dir];
	
	if (!is_valid_xy(yourx+xrel,youry+yrel)) {
		printf("the place you want to transfer to doesn't exist in the real universe\n");
		return;
	}
	
	groupid = areas[yourx+xrel][youry+yrel].groupid;
	
	if (groupid < 0) {
		printf("there's no group there!\n");
		return;
	}
	
	if (qty > groups[groupid].persons[type]) {
		printf("they don't have %i persons of that type, only %hi!\n",qty,groups[groupid].persons[type]);
		return;
	}
	
	if (groups[groupid].owner != org_you) {
		printf("you can't take people from a group you don't control!\n");
		return;
	}
	
	g_persons_lose(groupid, type, qty);
	g_persons_add(yourgroup, type, qty);
	
	destroy_group_if_nobody(groupid);
	
	printf("%i %s transferred from group to the %s!\n",
		qty, (qty>1?persontypes[type].nameplural:persontypes[type].name),
		dirnames[dir]);
}

void handle_givei(char *inputline) {
	int type, qty, dir, xrel, yrel, recv_groupid;
	
	type = get_nth_token_as_int(inputline,1);
	if (!is_valid_itemtype(type)) {
		printf("%i is not a valid itemtype!\n",type);
		return;
	}
	
	qty = get_nth_token_as_int(inputline,2);
	if (qty <= 0) {
		printf("%i is not a valid qty!\n",qty);
		return;
	}
	
	if (qty > groups[yourgroup].items[type]) {
		printf("you don't have %i items of that type, only %hi!\n",qty,groups[yourgroup].items[type]);
		return;
	}
	
	dir = get_nth_token_as_int(inputline,3);
	if ((dir < 1) || (dir > 9) || (dir == 5)) {
		printf("%i is not a valid dir number, must be 1-4 or 6-9!\n",dir);
		return;
	}
	
	xrel = dir2xrel[dir];
	yrel = dir2yrel[dir];
	
	if (!is_valid_xy(yourx+xrel,youry+yrel)) {
		printf("the place you want to transfer to doesn't exist in the real universe\n");
		return;
	}
	
	recv_groupid = get_or_create_group_at(yourx+xrel, youry+yrel,org_you);
	if (recv_groupid < 0) {
		printf("couldn't create new group so transfer failed!\n");
		return;
	} else printf("groupid is %i\n",recv_groupid);
	
	g_items_lose(yourgroup, type, qty);
	g_items_add(recv_groupid, type, qty);
	
	printf("%i %s transferred to group to the %s!\n",
		qty, (qty>1?itemtypes[type].nameplural:itemtypes[type].name),
		dirnames[dir]);
}

void handle_geti(char *inputline) {
	int type, qty, dir, xrel, yrel, groupid;
	
	type = get_nth_token_as_int(inputline,1);
	if (!is_valid_itemtype(type)) {
		printf("%i is not a valid itemtype!\n",type);
		return;
	}
	
	qty = get_nth_token_as_int(inputline,2);
	if (qty <= 0) {
		printf("%i is not a valid qty!\n",qty);
		return;
	}
	
	dir = get_nth_token_as_int(inputline,3);
	if ((dir < 1) || (dir > 9) || (dir == 5)) {
		printf("%i is not a valid dir number, must be 1-4 or 6-9!\n",dir);
		return;
	}
	
	xrel = dir2xrel[dir];
	yrel = dir2yrel[dir];
	
	if (!is_valid_xy(yourx+xrel,youry+yrel)) {
		printf("the place you want to transfer to doesn't exist in the real universe\n");
		return;
	}
	
	groupid = areas[yourx+xrel][youry+yrel].groupid;
	
	if (groupid < 0) {
		printf("there's no group there!\n");
		return;
	}
	
	if (qty > groups[groupid].items[type]) {
		printf("they don't have %i items of that type, only %hi!\n",qty,groups[groupid].items[type]);
		return;
	}
	
	if (groups[groupid].owner != org_you) {
		printf("you can't take items from a group you don't control!\n");
		return;
	}
	
	g_items_lose(groupid, type, qty);
	g_items_add(yourgroup, type, qty);
	
	destroy_group_if_nobody(groupid);
	
	printf("%i %s transferred from group to the %s!\n",
		qty, (qty>1?itemtypes[type].nameplural:itemtypes[type].name),
		dirnames[dir]);
}

void handle_transi(char *inputline) {
	int type, qty, dir, xrel, yrel, recv_pileid;
	
	type = get_nth_token_as_int(inputline,1);
	if (!is_valid_itemtype(type)) {
		printf("%i is not a valid itemtype!\n",type);
		return;
	}
	
	qty = get_nth_token_as_int(inputline,2);
	if (qty <= 0) {
		printf("%i is not a valid qty!\n",qty);
		return;
	}
	
	if (qty > groups[yourgroup].items[type]) {
		printf("you don't have %i items of that type, only %hi!\n",qty,groups[yourgroup].items[type]);
		return;
	}
	
	dir = get_nth_token_as_int(inputline,3);
	if ((dir < 1) || (dir > 9) || (dir == 5)) {
		printf("%i is not a valid dir number, must be 1-4 or 6-9!\n",dir);
		return;
	}
	
	xrel = dir2xrel[dir];
	yrel = dir2yrel[dir];
	
	if (!is_valid_xy(yourx+xrel,youry+yrel)) {
		printf("the place you want to transfer to doesn't exist in the real universe\n");
		return;
	}
	
	recv_pileid = get_or_create_pile_at(yourx+xrel, youry+yrel);
	if (recv_pileid < 0) {
		printf("couldn't create new pile so transfer failed!\n");
		return;
	} else printf("pileid is %i\n",recv_pileid);
	
	g_items_lose(yourgroup, type, qty);
	p_items_add(recv_pileid, type, qty);
	
	printf("%i %s transferred to pile to the %s!\n",
		qty, (qty>1?itemtypes[type].nameplural:itemtypes[type].name),
		dirnames[dir]);
}

void handle_grabi(char *inputline) {
	int type, qty, dir, xrel, yrel, pileid;
	
	type = get_nth_token_as_int(inputline,1);
	if (!is_valid_itemtype(type)) {
		printf("%i is not a valid itemtype!\n",type);
		return;
	}
	
	qty = get_nth_token_as_int(inputline,2);
	if (qty <= 0) {
		printf("%i is not a valid qty!\n",qty);
		return;
	}
	
	dir = get_nth_token_as_int(inputline,3);
	if ((dir < 1) || (dir > 9) || (dir == 5)) {
		printf("%i is not a valid dir number, must be 1-4 or 6-9!\n",dir);
		return;
	}
	
	xrel = dir2xrel[dir];
	yrel = dir2yrel[dir];
	
	if (!is_valid_xy(yourx+xrel,youry+yrel)) {
		printf("the place you want to transfer to doesn't exist in the real universe\n");
		return;
	}
	
	pileid = areas[yourx+xrel][youry+yrel].pileid;
	
	if (pileid < 0) {
		printf("there's no pile there!\n");
		return;
	}
	
	if (qty > piles[pileid].items[type]) {
		printf("that pile doesn't have %i items of that type, only %hi!\n",qty,piles[pileid].items[type]);
		return;
	}
	
	if ((areas[yourx+xrel][youry+yrel].groupid != -1)
		&&
		(groups[areas[yourx+xrel][youry+yrel].groupid].owner == org_enemy)) {
		printf("you can't take items out of a pile that's in the same area as an enemy group!\n");
		return;
	}
	
	p_items_lose(pileid, type, qty);
	g_items_add(yourgroup, type, qty);
	
	destroy_pile_if_empty(pileid);
	
	printf("%i %s transferred from pile to the %s!\n",
		qty, (qty>1?itemtypes[type].nameplural:itemtypes[type].name),
		dirnames[dir]);
}

void handle_drop(char *inputline) {
	int type, qty, recv_pileid;
	
	type = get_nth_token_as_int(inputline,1);
	if (!is_valid_itemtype(type)) {
		printf("%i is not a valid itemtype!\n",type);
		return;
	}
	
	qty = get_nth_token_as_int(inputline,2);
	if (qty <= 0) {
		printf("%i is not a valid qty!\n",qty);
		return;
	}
	
	if (qty > groups[yourgroup].items[type]) {
		printf("you don't have %i items of that type, only %hi!\n",qty,groups[yourgroup].items[type]);
		return;
	}
	
	recv_pileid = get_or_create_pile_at(yourx, youry);
	if (recv_pileid < 0) {
		printf("couldn't create new pile so transfer failed!\n");
		return;
	} else printf("pileid is %i\n",recv_pileid);
	
	g_items_lose(yourgroup, type, qty);
	p_items_add(recv_pileid, type, qty);
	
	printf("%i %s dropped in the pile in your area!\n",
		qty, (qty>1?itemtypes[type].nameplural:itemtypes[type].name));
}

void handle_pickup(char *inputline) {
	int type, qty;
	s16int pileid;
	
	type = get_nth_token_as_int(inputline,1);
	if (!is_valid_itemtype(type)) {
		printf("%i is not a valid itemtype!\n",type);
		return;
	}
	
	qty = get_nth_token_as_int(inputline,2);
	if (qty <= 0) {
		printf("%i is not a valid qty!\n",qty);
		return;
	}
	
	if (areas[yourx][youry].pileid < 0) {
		printf("there's nothing here to pickup!\n");
		return;
	}
	
	pileid = areas[yourx][youry].pileid;
	
	if (qty > piles[pileid].items[type]) {
		printf("there aren't %i items of that type here, only %hi!\n",qty,piles[pileid].items[type]);
		return;
	}
	
	p_items_lose(pileid, type, qty);
	g_items_add(yourgroup, type, qty);
	
	destroy_pile_if_empty(pileid);
	
	printf("%i %s picked up from the pile in your area!\n",
		qty, (qty>1?itemtypes[type].nameplural:itemtypes[type].name));
}

void handle_allow(char *inputline) {
	int type;
	
	type = get_nth_token_as_int(inputline,1);
	if (!is_valid_persontype(type)) {
		printf("%i is not a valid persontype!\n",type);
		return;
	}
	
	if (type == persontype_you) {
		printf("although %i is a valid persontype, it refers to you -- and you cannot specifically allow or refuse yourself to be recruited, because you can never recruit yourself, because you're you, and you don't need to be recruited, and couldn't possibly be anyway, at least not in the free crippled version available outside of Sweden.\n",type);
		return;
	}
	
	if (!persontypes[type].known) {
		printf("you have no knowledge of that persnotype!\n");
		return;
	}
	
	allowtojoin[type] = TRUE;
	
	printf("%s are now allowed when recruiting\n",persontypes[type].nameplural);
}

void handle_refuse(char *inputline) {
	int type;
	
	type = get_nth_token_as_int(inputline,1);
	if (!is_valid_persontype(type)) {
		printf("%i is not a valid persontype!\n",type);
		return;
	}
	
	if (type == persontype_you) {
		printf("although %i is a valid persontype, it refers to you -- and you cannot specifically allow or refuse yourself to be recruited, because you can never recruit yourself, because you're you, and you don't need to be recruited, and couldn't possibly be anyway, at least not in the free crippled version available outside of Sweden.\n",type);
		return;
	}
	
	if (!persontypes[type].known) {
		printf("you have no knowledge of that persnotype!\n");
		return;
	}
	
	allowtojoin[type] = FALSE;
	
	printf("%s are now refused when recruiting\n",persontypes[type].nameplural);
}

void handle_k() {
	int t, pt;
	
	printf("Key to Map Symbols:\n");
	printf(YOU_SYMBOL " - your party\n");
	printf(GROUP_SYMBOL " - group of people\n");
	for (pt = 0 ; pt < PERSONTYPES ; pt++) {
		if (pt == persontype_you) continue;
		if (persontypes[pt].symbol == 'P') continue;
		printf("%c - %s\n",persontypes[pt].symbol, persontypes[pt].nameplural);
	}
	printf(PILE_SYMBOL " - pile of items\n");
	for (t = 0 ; t < TERRAINS ; t++) {
		printf("%c - %s\n",terrains[t].symbol, terrains[t].name);
	}
}

void status() {
	s16int r, x, y, i, j, pt, it;
	int oldlen;
	char tmp[200]="", tmp2[200]="";
	s32int total_people;
	char txtfield[21][49]; /*21 rows, each 48 chars long (plus 1 for end-of-string null)*/
	int btg;
		
	total_people = get_your_people_count();
	
	for (r = 0 ; r < 21 ; r++) {
		strcpy(txtfield[r],"");
	}
	
	sprintf(txtfield[0]," Apocalypse Dawn                      %-9s",(gamewon?"GAME WON!":""));
	
	r = 2;
	for (pt = 0 ; pt < PERSONTYPES ; pt++) {
		if (groups[yourgroup].persons[pt] > 0) {
			if (r == 18) {sprintf(txtfield[r],"    (more)");break;}
			if (pt == persontype_you) sprintf(txtfield[r],"       %s",persontypes[pt].name);
			else sprintf(txtfield[r]," %5hi %.15s",groups[yourgroup].persons[pt],(groups[yourgroup].persons[pt]>1?persontypes[pt].nameplural:persontypes[pt].name));
			r++;
		}
	}
	
	r = 2;
	for (it = 0 ; it < ITEMTYPES ; it++) {
		if (groups[yourgroup].items[it] > 0) {
			if (strlen(txtfield[r]) < 22) {
				oldlen = strlen(txtfield[r]);
				for (i = 0 ; i < (22 - oldlen) ; i++) {
					strcat(txtfield[r]," ");
				}
			}
			if (r == 18) {sprintf(tmp,"    (more)"); strcat(txtfield[r],tmp); break;}
			sprintf(tmp,"%5hi %.19s",groups[yourgroup].items[it],(groups[yourgroup].items[it]>1?itemtypes[it].nameplural:itemtypes[it].name));
			if ((it == itemtype_food) && (total_people > 0)) {
				sprintf(tmp2," (~%hi days)",(s16int)(groups[yourgroup].items[it] / total_people));
				strcat(tmp,tmp2);
			}
			strcat(txtfield[r],tmp);
			r++;
		}
	}
	
	
	/*now 29 chars long! keep track of this...*/	
	sprintf(txtfield[20]," (%3hi,%3hi)   %-9s %-s",yourx-STARTX,youry-STARTY,terrains[areas[yourx][youry].terraintype].name,(areas[yourx][youry].youown?"yours":""));
	if (total_people > 1) {
		sprintf(tmp,"    %4li people",total_people);
		strcat(txtfield[20],tmp);
	}
	
	printf("/---------------------------\\------------------------------------------------\\\n");
	
	r = 0;
	for (y = youry - YMARGIN ; y <= youry + YMARGIN ; y++) {
		for (i=0;i<3;i++) {
			printf("|");
			for (x = yourx - XMARGIN ; x <= yourx + XMARGIN ; x++) {
				printf("%c",terrains[areas[x][y].terraintype].symbol);
				if (i == 1) {
					if (get_dist(yourx,youry,x,y) <= 2) {
						if ((yourx == x) && (youry == y)) {
							printf(YOU_SYMBOL);
						} else if (areas[x][y].groupid != -1) {
							btg = get_biggest_persontype_in_group(areas[x][y].groupid);
							persontypes[btg].known = TRUE;
							printf("%c",persontypes[btg].symbol);
						} else if ((areas[x][y].pileid != -1) && (get_dist(yourx,youry,x,y) <= 1)) {
							printf(PILE_SYMBOL);
						} else {
							printf("%c",terrains[areas[x][y].terraintype].symbol);
						}
					} else {
						printf("%c",terrains[areas[x][y].terraintype].symbol);
					}
				} else {
					printf("%c",terrains[areas[x][y].terraintype].symbol);
				}
				if ((i == 1) && (areas[x][y].pileid != -1) &&
					(areas[x][y].groupid != -1) && (get_dist(yourx,youry,x,y) <= 1)) {
					printf("%%");
				} else printf("%c",terrains[areas[x][y].terraintype].symbol);
			}
			printf("|");
			printf("%.48s",txtfield[r]);
			if (strlen(txtfield[r]) < 48) {
				for (j = 0 ; j < 48 - strlen(txtfield[r]) ; j++) {
					printf(" ");
				}
			}
			printf("|\n");
			r++;
		}
	}
	printf("\\---------------------------/------------------------------------------------/\n");
	if (showlastmsg) {
		printf("%.78s\n",lastmsg);
		showlastmsg = FALSE;
	} else {
		printf("\n");
	}
}

void handle_quit() {
	printf("Quitting...\n");
	show_final_score();
	my_exit();
}

void rules() {
	printf("These are not the rules of the game but this is where the rules would be if they were anywhere in the game.\n[Under construction]\n");
}

void credits() {
	printf("design & programming by Mike Kramlich\n");
}

void print_version_line() {
	printf("Apocalypse Dawn %i.%i built " __DATE__ " at " __TIME__"\n", VERSION_MAJOR, VERSION_MINOR);
}

void handle_ver() {
	print_version_line();
    printf("Apocalypse Dawn: Adventure in Post-Apocalyptic America\n");
	printf("design & programming by " AUTHOR "\n");
	printf("email: " EMAIL "\n");
	printf("web: " WEBSITE "\n");
	printf("Copyright 2002 by Michael A. Kramlich\n");
	printf("All Rights Reserved Worldwide\n");
}

void clearlastmsg() {
	strcpy(lastmsg,"");
}

bool is_adjacent_to_terrain(s16int x, s16int y, s16int ter) {
	s16int xrel, yrel;
	
	for (xrel=-1 ; xrel<2 ; xrel++) {
		for (yrel=-1 ; yrel<2 ; yrel++) {
			if ((xrel==0) && (yrel==0)) continue;
			if (is_valid_xy(x+xrel,y+yrel)) {
				if (areas[x+xrel][y+yrel].terraintype == ter) return TRUE;
			}
		}
	}
	return FALSE;
}

void wipe_data() {
	int x, y, t, pt, it, g, p, o;
	
	/*printf("wipe_data()\n");*/
	turn = 0;
	yourx = -1;
	youry = -1;
	
	clearlastmsg();
	showlastmsg = FALSE;
	moving = FALSE;
	gamewon = FALSE;
	enforce_eat = FALSE;
	enforce_rndmvmt = FALSE;
	
	for (o = 0 ; o < ORGS ; o++) {
		strcpy(orgs[o].name,"");
	}
	
	for (t = 0 ; t < TERRAINS ; t++) {
		strcpy(terrains[t].name,"");
		terrains[t].symbol = '?';
		terrains[t].impassable = FALSE;
		terrains[t].movetickcost = 1;
	}
	
	for (x = 0 ; x < MAPX ; x++) {
		for (y = 0 ; y < MAPY ; y++) {
			areas[x][y].terraintype = 0;
			areas[x][y].groupid = -1;
			areas[x][y].pileid = -1;
			areas[x][y].youown = FALSE;
		}
	}
	
	for (pt = 0 ; pt < PERSONTYPES ; pt++) {
		strcpy(persontypes[pt].name,"");
		strcpy(persontypes[pt].nameplural,"");
		strcpy(persontypes[pt].desc,"");
		persontypes[pt].basefrequency = 1;
		for (t = 0 ; t < TERRAINS ; t++) {
			persontypes[pt].terrfreqmultiplier[t] = 1.0;
		}
		persontypes[pt].unique = FALSE;
		persontypes[pt].unique_inplay = FALSE;
		persontypes[pt].unique_permgone = FALSE;
		persontypes[pt].is_animal = FALSE;
		persontypes[pt].is_dangerous = FALSE;
		persontypes[pt].known = FALSE;
		persontypes[pt].combatvalue = 0;
		persontypes[pt].symbol = ' ';
	}
	
	for (it = 0 ; it < ITEMTYPES ; it++) {
		strcpy(itemtypes[it].name,"");
		strcpy(itemtypes[it].nameplural,"");
		strcpy(itemtypes[it].desc,"");
		itemtypes[it].basefrequency = 1;
		for (t = 0 ; t < TERRAINS ; t++) {
			itemtypes[it].terrfreqmultiplier[t] = 1.0;
		}
		itemtypes[it].unique = FALSE;
		itemtypes[it].unique_inplay = FALSE;
		itemtypes[it].unique_permgone = FALSE;
		itemtypes[it].known = FALSE;
		itemtypes[it].combatvalue = 0;
	}
	
	for (g = 0 ; g < GROUPS ; g++) {
		wipe_group(g);
	}
	
	for (p = 0 ; p < PILES ; p++) {
		wipe_pile(p);
	}
	
	for (pt = 0 ; pt < PERSONTYPES ; pt++) allowtojoin[pt] = TRUE;
}

s16int get_rnd_notimpassable_notcity_land_terrain() {
	s16int terr;
	do {
		terr = get_rnd_u16int_with_notinclusive_max(TERRAINS);
	} while ((terr == terraintype_sea) || (terr == terraintype_mountains) || (terr == terraintype_urban));
	return terr;
}

void genesis_init_misc_global_vars() {
	turn = 0;
	yourx = STARTX;
	youry = STARTY;
}

void genesis_init_orgs() {
	int o;
	
	o = 0;
	strcpy(orgs[o].name,"Yours");
	org_you = o;
	
	o++;
	strcpy(orgs[o].name,"Indep.");
	org_independent = o;
	
	o++;
	strcpy(orgs[o].name,"Enemy");
	org_enemy = o;	
}

void genesis_init_terrains() {
	int t;
	
	t = 0;	
	strcpy(terrains[t].name,"plains");
	terrains[t].symbol = '\'';
	terraintype_plains = t;
		
	t++;	
	strcpy(terrains[t].name,"grassland");
	terrains[t].symbol = '`';
	terraintype_grassland = t;
	
	t++;	
	strcpy(terrains[t].name,"urban");
	terrains[t].symbol = '#';
	terraintype_urban = t;
	
	t++;	
	strcpy(terrains[t].name,"sea");
	terrains[t].symbol = '~';
	terrains[t].impassable = TRUE;
	terraintype_sea = t;
	
	t++;	
	strcpy(terrains[t].name,"forest");
	terrains[t].symbol = '"';
	terraintype_forest = t;
	
	t++;	
	strcpy(terrains[t].name,"desert");
	terrains[t].symbol = '.';
	terrains[t].movetickcost = 2;
	terraintype_desert = t;
	
	t++;	
	strcpy(terrains[t].name,"mountains");
	terrains[t].symbol = '^';
	terrains[t].movetickcost = 2;
	terraintype_mountains = t;	
}

void genesis_init_map_terrain() {
	int x, y, n;
	
	for (x = 0 ; x < MAPX ; x++) {
		for (y = 0 ; y < MAPY ; y++) {
			areas[x][y].terraintype = get_rnd_u16int_with_notinclusive_max(TERRAINS);
			if ((areas[x][y].terraintype == terraintype_urban) && (rollforsuccess(11,12))) {
				areas[x][y].terraintype = terraintype_plains;
			}
			if ((areas[x][y].terraintype == terraintype_mountains) && (rollforsuccess(7,12))) {
				areas[x][y].terraintype = terraintype_plains;
			}
			if ((areas[x][y].terraintype == terraintype_desert) && (rollforsuccess(10,12))) {
				areas[x][y].terraintype = terraintype_plains;
			}
			if ((areas[x][y].terraintype == terraintype_forest) && (rollforsuccess(3,12))) {
				areas[x][y].terraintype = terraintype_grassland;
			}
			if (areas[x][y].terraintype == terraintype_sea) {
				areas[x][y].terraintype = terraintype_grassland;
			}
		}
	}
	
	/*put a strip of sea all around the outer edge of the map*/
	/*left edge*/  for (x=0;x<XMARGIN;x++)         for (y=0;y<MAPY;y++) areas[x][y].terraintype = terraintype_sea;
	/*right edge*/ for (x=MAPX-XMARGIN;x<MAPX;x++) for (y=0;y<MAPY;y++) areas[x][y].terraintype = terraintype_sea;
	/*top edge*/   for (y=0;y<YMARGIN;y++)         for (x=0;x<MAPX;x++) areas[x][y].terraintype = terraintype_sea;
	/*bottom edge*/for (y=MAPY-YMARGIN;y<MAPY;y++) for (x=0;x<MAPX;x++) areas[x][y].terraintype = terraintype_sea;
	
	/* make the sea "grow" organically from outer edge in towards the continent, in fingers/threads/rivers*/
	for (n=0;n<2;n++) {	
		for (x = 0 ; x < MAPX ; x++) {
			for (y = 0 ; y < MAPY ; y++) {
				if ((areas[x][y].terraintype != terraintype_sea) && is_adjacent_to_terrain(x, y, terraintype_sea) && rollforsuccess(1,3)) {
					areas[x][y].terraintype = terraintype_sea;
				}
			}
		}
	}
	
	/* Make sure Colorado area is mostly plains as the "base" terrain type,*/
	/* before we add mountains and stuff */
	
	/*   50% plains in general 15x15 region*/
	for (x = yourx - 15 ; x < yourx + 15 ; x++) {
		for (y = youry - 15 ; y < youry + 15 ; y++) {
			if (rollforsuccess(5,10)) areas[x][y].terraintype = terraintype_plains;
		}
	}
	/*   mostly plains in general 8x8 region*/
	for (x = yourx - 8 ; x < yourx + 8 ; x++) {
		for (y = youry - 8 ; y < youry + 8 ; y++) {
			if (rollforsuccess(8,10)) areas[x][y].terraintype = terraintype_plains;
		}
	}
	/*  plains only in core 5x5 region*/
	for (x = yourx - 5 ; x < yourx + 5 ; x++) {
		for (y = youry - 5 ; y < youry + 5 ; y++) {
			areas[x][y].terraintype = terraintype_plains;
		}
	}
	
	/* Put a city (Danveer/Denver) to the south of where you start*/
	areas[yourx][youry+2].terraintype = terraintype_urban;
	
	/* Rocky Mountains */
	areas[yourx-2][youry].terraintype = terraintype_mountains;
	areas[yourx-2][youry+1].terraintype = terraintype_mountains;	
	areas[yourx-3][youry-1].terraintype = terraintype_mountains;	
	areas[yourx-3][youry-2].terraintype = terraintype_mountains;	
	areas[yourx-3][youry].terraintype = terraintype_mountains;	
	areas[yourx-3][youry+1].terraintype = terraintype_mountains;		
	areas[yourx-3][youry+2].terraintype = terraintype_mountains;	
	areas[yourx-3][youry+3].terraintype = terraintype_mountains;	
	areas[yourx-4][youry-2].terraintype = terraintype_mountains;	
	areas[yourx-4][youry-3].terraintype = terraintype_mountains;	
	areas[yourx-4][youry].terraintype = terraintype_mountains;		
}

void genesis_init_persontypes() {
	int pt;
	
	pt = 0;
	strcpy(persontypes[pt].name,"you");
	strcpy(persontypes[pt].nameplural,"you");
	strcpy(persontypes[pt].desc,"Looks suspiciously like you. Almost like "\
		"you're related or something. Hey, wait a minute... it is you!");
	persontypes[pt].basefrequency = 30;
	persontypes[pt].unique = TRUE;
	persontypes[pt].combatvalue = 5;
	persontypes[pt].symbol = '@';
	persontype_you = pt;
	
	pt++;
	strcpy(persontypes[pt].name,"refugee");
	strcpy(persontypes[pt].nameplural,"refugees");
	strcpy(persontypes[pt].desc,"Give me your tired, your poor, your weak, your"\
		" completely lacking in skills or tradeable goods. These people look like refugees.");
	persontypes[pt].basefrequency = 90;
	persontypes[pt].terrfreqmultiplier[terraintype_urban] = 1.5;
	persontypes[pt].terrfreqmultiplier[terraintype_desert] = 0.25;
	persontypes[pt].terrfreqmultiplier[terraintype_forest] = 0.75;
	persontypes[pt].terrfreqmultiplier[terraintype_mountains] = 0.5;
	persontypes[pt].combatvalue = 2;
	persontypes[pt].symbol = 'P';
	persontype_refugee = pt;
	
	pt++;
	strcpy(persontypes[pt].name,"farmer");
	strcpy(persontypes[pt].nameplural,"farmers");
	strcpy(persontypes[pt].desc,"Looks like a farmer. Ayup. A farmer. He be"\
		" farmin. Got no time for varmints.");
	persontypes[pt].basefrequency = 25;
	persontypes[pt].terrfreqmultiplier[terraintype_plains] = 2.0;
	persontypes[pt].terrfreqmultiplier[terraintype_urban] = 0.5;
	persontypes[pt].terrfreqmultiplier[terraintype_desert] = 0.25;
	persontypes[pt].terrfreqmultiplier[terraintype_mountains] = 0.5;
	persontypes[pt].combatvalue = 2;
	persontypes[pt].symbol = 'P';
	persontype_farmer = pt;
	
	pt++;
	strcpy(persontypes[pt].name,"thug");
	strcpy(persontypes[pt].nameplural,"thugs");
	strcpy(persontypes[pt].desc,"Looks like thugs. Grrr! Ook! Arf. Huh? *slap* *runs away*");
	persontypes[pt].basefrequency = 50;
	persontypes[pt].terrfreqmultiplier[terraintype_urban] = 2.0;
	persontypes[pt].terrfreqmultiplier[terraintype_desert] = 0.25;
	persontypes[pt].terrfreqmultiplier[terraintype_forest] = 0.75;
	persontypes[pt].terrfreqmultiplier[terraintype_mountains] = 0.5;
	persontypes[pt].combatvalue = 4;
	persontypes[pt].symbol = 'P';
	persontype_thug = pt;
	
	pt++;
	strcpy(persontypes[pt].name,"drill sergeant");
	strcpy(persontypes[pt].nameplural,"drill sergeants");
	strcpy(persontypes[pt].desc,"Looks like a drill sergeant.");
	persontypes[pt].basefrequency = 10;
	persontypes[pt].terrfreqmultiplier[terraintype_desert] = 0.25;
	persontypes[pt].terrfreqmultiplier[terraintype_mountains] = 0.5;
	persontypes[pt].combatvalue = 6;
	persontypes[pt].symbol = 'P';
	persontype_drillsergeant = pt;
	
	pt++;
	strcpy(persontypes[pt].name,"ex-Green Beret");
	strcpy(persontypes[pt].nameplural,"ex-Green Berets");
	strcpy(persontypes[pt].desc,"Looks like a former Green Beret. Must be the"\
		" Green Beret uniform he's wearing plus the fact that the United States of"\
		" America, as such, no longer exists, and therefore has no armed forces "\
		"anymore. Yeah, I'm pretty sure it's those two factors.");
	persontypes[pt].basefrequency = 5;
	persontypes[pt].terrfreqmultiplier[terraintype_desert] = 0.25;
	persontypes[pt].terrfreqmultiplier[terraintype_mountains] = 0.5;
	persontypes[pt].combatvalue = 15;
	persontypes[pt].symbol = 'P';
	persontype_greenberet = pt;
	
	pt++;
	strcpy(persontypes[pt].name,"hunter");
	strcpy(persontypes[pt].nameplural,"hunters");
	strcpy(persontypes[pt].desc,"Looks like a hunter. The obnoxious day-glow"\
		" orange outfit he thinks is camouflage is the biggest clue.");
	persontypes[pt].basefrequency = 25;
	persontypes[pt].terrfreqmultiplier[terraintype_grassland] = 2.0;
	persontypes[pt].terrfreqmultiplier[terraintype_desert] = 0.25;
	persontypes[pt].terrfreqmultiplier[terraintype_forest] = 3.0;
	persontypes[pt].terrfreqmultiplier[terraintype_mountains] = 3.0;
	persontypes[pt].combatvalue = 4;
	persontypes[pt].symbol = 'P';
	persontype_hunter = pt;
	
	pt++;
	strcpy(persontypes[pt].name,"hippie");
	strcpy(persontypes[pt].nameplural,"hippies");
	strcpy(persontypes[pt].desc,"Looks like a hippie. Someone who likes the "\
		"Grateful Dead for some reason nobody else understands, and, of course,"\
		" feels very strongly about the many fine uses of hemp. Such as rope, and"\
		" making tee-shirts, and...Hey, do you have any snacks?");
	persontypes[pt].basefrequency = 30;
	persontypes[pt].terrfreqmultiplier[terraintype_desert] = 0.25;
	persontypes[pt].terrfreqmultiplier[terraintype_mountains] = 0.5;
	persontypes[pt].combatvalue = 2;
	persontypes[pt].symbol = 'P';
	persontype_hippie = pt;
	
	pt++;
	strcpy(persontypes[pt].name,"engineer");
	strcpy(persontypes[pt].nameplural,"engineers");
	strcpy(persontypes[pt].desc,"Looks like an engineer.");
	persontypes[pt].basefrequency = 10;
	persontypes[pt].terrfreqmultiplier[terraintype_urban] = 2.0;
	persontypes[pt].terrfreqmultiplier[terraintype_desert] = 0.25;
	persontypes[pt].terrfreqmultiplier[terraintype_forest] = 0.5;
	persontypes[pt].terrfreqmultiplier[terraintype_mountains] = 0.5;
	persontypes[pt].combatvalue = 2;
	persontypes[pt].symbol = 'P';
	persontype_engineer = pt;
	
	pt++;
	strcpy(persontypes[pt].name,"doctor");
	strcpy(persontypes[pt].nameplural,"doctors");
	strcpy(persontypes[pt].desc,"Looks like a doctor.");
	persontypes[pt].basefrequency = 10;
	persontypes[pt].terrfreqmultiplier[terraintype_urban] = 2.0;
	persontypes[pt].terrfreqmultiplier[terraintype_desert] = 0.25;
	persontypes[pt].terrfreqmultiplier[terraintype_forest] = 0.5;
	persontypes[pt].terrfreqmultiplier[terraintype_mountains] = 0.5;
	persontypes[pt].combatvalue = 2;
	persontypes[pt].symbol = 'P';
	persontype_doctor = pt;
	
	pt++;
	strcpy(persontypes[pt].name,"lawyer");
	strcpy(persontypes[pt].nameplural,"lawyers");
	strcpy(persontypes[pt].desc,"Looks like a lawyer. You realize you feel a "\
	"little nauseous in it's presence and hope it doesn't notice you. Then a "\
	"wonderful idea occurs to you, and you yell, \"Look, an ambulance!\" which"\
	" distracts it just long enough for you to disappear behind a small shrub"\
	" of decency that was located conveniently nearby.");
	persontypes[pt].basefrequency = 10;
	persontypes[pt].terrfreqmultiplier[terraintype_urban] = 2.0;
	persontypes[pt].terrfreqmultiplier[terraintype_desert] = 0.25;
	persontypes[pt].terrfreqmultiplier[terraintype_forest] = 0.5;
	persontypes[pt].terrfreqmultiplier[terraintype_mountains] = 0.5;
	persontypes[pt].combatvalue = 2;
	persontypes[pt].symbol = 'P';
	persontype_lawyer = pt;

	pt++;
	strcpy(persontypes[pt].name,"beggar");
	strcpy(persontypes[pt].nameplural,"beggars");
	strcpy(persontypes[pt].desc,"Looks like a beggar.");
	persontypes[pt].basefrequency = 30;
	persontypes[pt].terrfreqmultiplier[terraintype_urban] = 1.0;
	persontypes[pt].terrfreqmultiplier[terraintype_desert] = 1.0;
	persontypes[pt].terrfreqmultiplier[terraintype_forest] = 1.0;
	persontypes[pt].terrfreqmultiplier[terraintype_mountains] = 1.0;
	persontypes[pt].combatvalue = 1;
	persontypes[pt].symbol = 'P';
	persontype_beggar = pt;
	
	pt++;
	strcpy(persontypes[pt].name,"leper");
	strcpy(persontypes[pt].nameplural,"lepers");
	strcpy(persontypes[pt].desc,"Looks like a leper.");
	persontypes[pt].basefrequency = 30;
	persontypes[pt].terrfreqmultiplier[terraintype_urban] = 1.0;
	persontypes[pt].terrfreqmultiplier[terraintype_desert] = 1.0;
	persontypes[pt].terrfreqmultiplier[terraintype_forest] = 1.0;
	persontypes[pt].terrfreqmultiplier[terraintype_mountains] = 1.0;
	persontypes[pt].combatvalue = 1;
	persontypes[pt].symbol = 'P';
	persontype_leper = pt;
	
	pt++;
	strcpy(persontypes[pt].name,"thief");
	strcpy(persontypes[pt].nameplural,"thieves");
	strcpy(persontypes[pt].desc,"Looks like a thief.");
	persontypes[pt].is_dangerous = TRUE;
	persontypes[pt].basefrequency = 30;
	persontypes[pt].terrfreqmultiplier[terraintype_urban] = 1.0;
	persontypes[pt].terrfreqmultiplier[terraintype_desert] = 1.0;
	persontypes[pt].terrfreqmultiplier[terraintype_forest] = 1.0;
	persontypes[pt].terrfreqmultiplier[terraintype_mountains] = 1.0;
	persontypes[pt].combatvalue = 2;
	persontypes[pt].symbol = 'P';
	persontype_thief = pt;

	pt++;
	strcpy(persontypes[pt].name,"zombie");
	strcpy(persontypes[pt].nameplural,"zombies");
	strcpy(persontypes[pt].desc,"Looks like a zombie.");
	persontypes[pt].is_dangerous = TRUE;
	persontypes[pt].basefrequency = 30;
	persontypes[pt].terrfreqmultiplier[terraintype_urban] = 1.0;
	persontypes[pt].terrfreqmultiplier[terraintype_desert] = 1.0;
	persontypes[pt].terrfreqmultiplier[terraintype_forest] = 1.0;
	persontypes[pt].terrfreqmultiplier[terraintype_mountains] = 1.0;
	persontypes[pt].combatvalue = 1;
	persontypes[pt].symbol = 'Z';
	persontype_zombie = pt;
	
	pt++;
	strcpy(persontypes[pt].name,"mutant");
	strcpy(persontypes[pt].nameplural,"mutants");
	strcpy(persontypes[pt].desc,"Looks like a mutant.");
	persontypes[pt].is_dangerous = TRUE;
	persontypes[pt].basefrequency = 30;
	persontypes[pt].terrfreqmultiplier[terraintype_urban] = 1.0;
	persontypes[pt].terrfreqmultiplier[terraintype_desert] = 1.0;
	persontypes[pt].terrfreqmultiplier[terraintype_forest] = 1.0;
	persontypes[pt].terrfreqmultiplier[terraintype_mountains] = 1.0;
	persontypes[pt].combatvalue = 10;
	persontypes[pt].symbol = 'M';
	persontype_mutant = pt;
	
	pt++;
	strcpy(persontypes[pt].name,"psycho");
	strcpy(persontypes[pt].nameplural,"psycho's");
	strcpy(persontypes[pt].desc,"Looks like a psycho.");
	persontypes[pt].is_dangerous = TRUE;
	persontypes[pt].basefrequency = 30;
	persontypes[pt].terrfreqmultiplier[terraintype_urban] = 1.0;
	persontypes[pt].terrfreqmultiplier[terraintype_desert] = 1.0;
	persontypes[pt].terrfreqmultiplier[terraintype_forest] = 1.0;
	persontypes[pt].terrfreqmultiplier[terraintype_mountains] = 1.0;
	persontypes[pt].combatvalue = 4;
	persontypes[pt].symbol = 'P';
	persontype_psycho = pt;

	pt++;
	strcpy(persontypes[pt].name,"biker");
	strcpy(persontypes[pt].nameplural,"bikers");
	strcpy(persontypes[pt].desc,"Looks like a biker. Leather...Wheels...Beard... Yep, it's either a biker or a member of ZZ Top. And you don't see any guitars.");
	persontypes[pt].is_dangerous = TRUE;
	persontypes[pt].basefrequency = 30;
	persontypes[pt].terrfreqmultiplier[terraintype_urban] = 1.0;
	persontypes[pt].terrfreqmultiplier[terraintype_desert] = 1.0;
	persontypes[pt].terrfreqmultiplier[terraintype_forest] = 1.0;
	persontypes[pt].terrfreqmultiplier[terraintype_mountains] = 1.0;
	persontypes[pt].combatvalue = 4;
	persontypes[pt].symbol = 'P';
	persontype_biker = pt;
	
	pt++;
	strcpy(persontypes[pt].name,"rat");
	strcpy(persontypes[pt].nameplural,"rats");
	strcpy(persontypes[pt].desc,"Looks like a rat.");
	persontypes[pt].is_animal = TRUE;
	persontypes[pt].basefrequency = 30;
	persontypes[pt].terrfreqmultiplier[terraintype_urban] = 1.0;
	persontypes[pt].terrfreqmultiplier[terraintype_desert] = 1.0;
	persontypes[pt].terrfreqmultiplier[terraintype_forest] = 1.0;
	persontypes[pt].terrfreqmultiplier[terraintype_mountains] = 1.0;
	persontypes[pt].combatvalue = 1;
	persontypes[pt].symbol = 'r';
	persontype_rat = pt;
	
	pt++;
	strcpy(persontypes[pt].name,"wolf");
	strcpy(persontypes[pt].nameplural,"wolves");
	strcpy(persontypes[pt].desc,"Looks like a wolf.");
	persontypes[pt].is_animal = TRUE;
	persontypes[pt].basefrequency = 30;
	persontypes[pt].terrfreqmultiplier[terraintype_urban] = 1.0;
	persontypes[pt].terrfreqmultiplier[terraintype_desert] = 1.0;
	persontypes[pt].terrfreqmultiplier[terraintype_forest] = 1.0;
	persontypes[pt].terrfreqmultiplier[terraintype_mountains] = 1.0;
	persontypes[pt].combatvalue = 6;
	persontypes[pt].symbol = 'w';
	persontype_wolf = pt;
	
	pt++;
	strcpy(persontypes[pt].name,"bear");
	strcpy(persontypes[pt].nameplural,"bears");
	strcpy(persontypes[pt].desc,"Looks like a bear.");
	persontypes[pt].is_animal = TRUE;
	persontypes[pt].basefrequency = 30;
	persontypes[pt].terrfreqmultiplier[terraintype_urban] = 1.0;
	persontypes[pt].terrfreqmultiplier[terraintype_desert] = 1.0;
	persontypes[pt].terrfreqmultiplier[terraintype_forest] = 1.0;
	persontypes[pt].terrfreqmultiplier[terraintype_mountains] = 1.0;
	persontypes[pt].combatvalue = 8;
	persontypes[pt].symbol = 'b';
	persontype_bear = pt;	
}

void genesis_init_itemtypes() {
	int it;
	
	it = 0;
	strcpy(itemtypes[it].name,"food");
	strcpy(itemtypes[it].nameplural,"food");
	strcpy(itemtypes[it].desc,"A gobby paste containing random but delicious and"\
		" nutrious food from every food group, all mixed together in a delightful"\
		" panoply that gently assaults the senses like a symphony on the palate of"\
		" the finest European connosiuer. In other words, peanut butter.");
	itemtypes[it].basefrequency = 30;
	itemtypes[it].terrfreqmultiplier[terraintype_plains] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_grassland] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_urban] = 2.0;
	itemtypes[it].terrfreqmultiplier[terraintype_sea] = 0.0;
	itemtypes[it].terrfreqmultiplier[terraintype_desert] = 0.1;
	itemtypes[it].terrfreqmultiplier[terraintype_forest] = 1.5;
	itemtypes[it].terrfreqmultiplier[terraintype_mountains] = 0.5;
	itemtypes[it].combatvalue = 0;
	itemtype_food = it;
	
	it++;
	strcpy(itemtypes[it].name,"gun");
	strcpy(itemtypes[it].nameplural,"guns");
	strcpy(itemtypes[it].desc,"This type of gun looks remarkably like a Beretta"\
		" 9mm pistol.");
	itemtypes[it].basefrequency = 30;
	itemtypes[it].terrfreqmultiplier[terraintype_plains] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_grassland] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_urban] = 1.5;
	itemtypes[it].terrfreqmultiplier[terraintype_sea] = 0.0;
	itemtypes[it].terrfreqmultiplier[terraintype_desert] = 0.1;
	itemtypes[it].terrfreqmultiplier[terraintype_forest] = 0.25;
	itemtypes[it].terrfreqmultiplier[terraintype_mountains] = 0.2;
	itemtypes[it].combatvalue = 10;
	itemtype_gun = it;
	
	it++;
	strcpy(itemtypes[it].name,"crude map");
	strcpy(itemtypes[it].nameplural,"crude maps");
	strcpy(itemtypes[it].desc,"A parchment with what appears to be a crude map "\
		"of the area surrounding your old refugee camp. Here is what the drawing "\
		"looks like:\n"\
		"...................................\n"\
		"..^....our.camp........gubmnt.--->.\n"\
		"..^.......#........................\n"\
		"...^...............................\n"\
		"..^.............#.........BAD!.....\n"\
		"...^^........Danveer.......|.......\n"\
		"..^^.^.....................V.......\n"\
		"..^.^..............................");
	itemtypes[it].basefrequency = 30;
	itemtypes[it].terrfreqmultiplier[terraintype_plains] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_grassland] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_urban] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_sea] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_desert] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_forest] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_mountains] = 1.0;
	itemtypes[it].combatvalue = 0;
	itemtypes[it].unique = TRUE;
	itemtype_crudemap = it;
			
	it++;
	strcpy(itemtypes[it].name,"knife");
	strcpy(itemtypes[it].nameplural,"knives");
	strcpy(itemtypes[it].desc,"Looks like a knife.");
	itemtypes[it].basefrequency = 30;
	itemtypes[it].terrfreqmultiplier[terraintype_plains] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_grassland] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_urban] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_sea] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_desert] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_forest] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_mountains] = 1.0;
	itemtypes[it].combatvalue = 2;
	itemtype_knife = it;
	
	it++;
	strcpy(itemtypes[it].name,"pair of boots");
	strcpy(itemtypes[it].nameplural,"pairs of boots");
	strcpy(itemtypes[it].desc,"Looks like a pair of boots.");
	itemtypes[it].basefrequency = 30;
	itemtypes[it].terrfreqmultiplier[terraintype_plains] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_grassland] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_urban] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_sea] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_desert] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_forest] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_mountains] = 1.0;
	itemtypes[it].combatvalue = 0;
	itemtype_bootpair = it;
	
	it++;
	strcpy(itemtypes[it].name,"blanket");
	strcpy(itemtypes[it].nameplural,"blankets");
	strcpy(itemtypes[it].desc,"Looks like a blanket.");
	itemtypes[it].basefrequency = 30;
	itemtypes[it].terrfreqmultiplier[terraintype_plains] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_grassland] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_urban] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_sea] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_desert] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_forest] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_mountains] = 1.0;
	itemtypes[it].combatvalue = 0;
	itemtype_blanket = it;

	it++;
	strcpy(itemtypes[it].name,"coat");
	strcpy(itemtypes[it].nameplural,"coats");
	strcpy(itemtypes[it].desc,"Looks like a coat.");
	itemtypes[it].basefrequency = 30;
	itemtypes[it].terrfreqmultiplier[terraintype_plains] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_grassland] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_urban] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_sea] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_desert] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_forest] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_mountains] = 1.0;
	itemtypes[it].combatvalue = 0;
	itemtype_coat = it;
	
	it++;
	strcpy(itemtypes[it].name,"bottle of medicine");
	strcpy(itemtypes[it].nameplural,"bottles of medicine");
	strcpy(itemtypes[it].desc,"Looks like a bottle of medicine.");
	itemtypes[it].basefrequency = 30;
	itemtypes[it].terrfreqmultiplier[terraintype_plains] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_grassland] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_urban] = 2.0;
	itemtypes[it].terrfreqmultiplier[terraintype_sea] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_desert] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_forest] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_mountains] = 1.0;
	itemtypes[it].combatvalue = 0;
	itemtype_medicine = it;
	
	it++;
	strcpy(itemtypes[it].name,"hand tool");
	strcpy(itemtypes[it].nameplural,"hand tools");
	strcpy(itemtypes[it].desc,"Looks like a hand tool.");
	itemtypes[it].basefrequency = 30;
	itemtypes[it].terrfreqmultiplier[terraintype_plains] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_grassland] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_urban] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_sea] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_desert] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_forest] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_mountains] = 1.0;
	itemtypes[it].combatvalue = 1;
	itemtype_handtool = it;
	
	it++;
	strcpy(itemtypes[it].name,"bullet");
	strcpy(itemtypes[it].nameplural,"bullets");
	strcpy(itemtypes[it].desc,"Looks like a bullet.");
	itemtypes[it].basefrequency = 30;
	itemtypes[it].terrfreqmultiplier[terraintype_plains] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_grassland] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_urban] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_sea] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_desert] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_forest] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_mountains] = 1.0;
	itemtypes[it].combatvalue = 0;
	itemtype_bullet = it;

	it++;
	strcpy(itemtypes[it].name,"club");
	strcpy(itemtypes[it].nameplural,"clubs");
	strcpy(itemtypes[it].desc,"Looks like a club.");
	itemtypes[it].basefrequency = 30;
	itemtypes[it].terrfreqmultiplier[terraintype_plains] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_grassland] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_urban] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_sea] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_desert] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_forest] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_mountains] = 1.0;
	itemtypes[it].combatvalue = 3;
	itemtype_club = it;
	
	it++;
	strcpy(itemtypes[it].name,"spear");
	strcpy(itemtypes[it].nameplural,"spears");
	strcpy(itemtypes[it].desc,"Looks like a spear.");
	itemtypes[it].basefrequency = 30;
	itemtypes[it].terrfreqmultiplier[terraintype_plains] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_grassland] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_urban] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_sea] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_desert] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_forest] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_mountains] = 1.0;
	itemtypes[it].combatvalue = 4;
	itemtype_spear = it;
	
	it++;
	strcpy(itemtypes[it].name,"bow");
	strcpy(itemtypes[it].nameplural,"bows");
	strcpy(itemtypes[it].desc,"Looks like a bow.");
	itemtypes[it].basefrequency = 30;
	itemtypes[it].terrfreqmultiplier[terraintype_plains] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_grassland] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_urban] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_sea] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_desert] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_forest] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_mountains] = 1.0;
	itemtypes[it].combatvalue = 6;
	itemtype_bow = it;
	
	it++;
	strcpy(itemtypes[it].name,"arrow");
	strcpy(itemtypes[it].nameplural,"arrows");
	strcpy(itemtypes[it].desc,"Looks like an arrow.");
	itemtypes[it].basefrequency = 30;
	itemtypes[it].terrfreqmultiplier[terraintype_plains] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_grassland] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_urban] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_sea] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_desert] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_forest] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_mountains] = 1.0;
	itemtypes[it].combatvalue = 0;
	itemtype_arrow = it;
	
	it++;
	strcpy(itemtypes[it].name,"axe");
	strcpy(itemtypes[it].nameplural,"axes");
	strcpy(itemtypes[it].desc,"Looks like an axe.");
	itemtypes[it].basefrequency = 30;
	itemtypes[it].terrfreqmultiplier[terraintype_plains] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_grassland] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_urban] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_sea] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_desert] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_forest] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_mountains] = 1.0;
	itemtypes[it].combatvalue = 3;
	itemtype_axe = it;
	
	it++;
	strcpy(itemtypes[it].name,"nail");
	strcpy(itemtypes[it].nameplural,"nails");
	strcpy(itemtypes[it].desc,"Looks like a nail.");
	itemtypes[it].basefrequency = 30;
	itemtypes[it].terrfreqmultiplier[terraintype_plains] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_grassland] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_urban] = 2.0;
	itemtypes[it].terrfreqmultiplier[terraintype_sea] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_desert] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_forest] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_mountains] = 1.0;
	itemtypes[it].combatvalue = 0;
	itemtype_nail = it;
	
	it++;
	strcpy(itemtypes[it].name,"rock");
	strcpy(itemtypes[it].nameplural,"rocks");
	strcpy(itemtypes[it].desc,"Looks like a rock.");
	itemtypes[it].basefrequency = 30;
	itemtypes[it].terrfreqmultiplier[terraintype_plains] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_grassland] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_urban] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_sea] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_desert] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_forest] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_mountains] = 2.0;
	itemtypes[it].combatvalue = 1;
	itemtype_rock = it;
	
	it++;
	strcpy(itemtypes[it].name,"bag");
	strcpy(itemtypes[it].nameplural,"bags");
	strcpy(itemtypes[it].desc,"Looks like a bag.");
	itemtypes[it].basefrequency = 30;
	itemtypes[it].terrfreqmultiplier[terraintype_plains] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_grassland] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_urban] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_sea] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_desert] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_forest] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_mountains] = 1.0;
	itemtypes[it].combatvalue = 0;
	itemtype_bag = it;
	
	it++;
	strcpy(itemtypes[it].name,"backpack");
	strcpy(itemtypes[it].nameplural,"backpacks");
	strcpy(itemtypes[it].desc,"Looks like a backpack.");
	itemtypes[it].basefrequency = 30;
	itemtypes[it].terrfreqmultiplier[terraintype_plains] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_grassland] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_urban] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_sea] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_desert] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_forest] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_mountains] = 1.0;
	itemtypes[it].combatvalue = 0;
	itemtype_backpack = it;
	
	it++;
	strcpy(itemtypes[it].name,"twig");
	strcpy(itemtypes[it].nameplural,"twigs");
	strcpy(itemtypes[it].desc,"Looks like a twig.");
	itemtypes[it].basefrequency = 30;
	itemtypes[it].terrfreqmultiplier[terraintype_plains] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_grassland] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_urban] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_sea] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_desert] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_forest] = 3.0;
	itemtypes[it].terrfreqmultiplier[terraintype_mountains] = 1.0;
	itemtypes[it].combatvalue = 0;
	itemtype_twig = it;
	
	it++;
	strcpy(itemtypes[it].name,"branch");
	strcpy(itemtypes[it].nameplural,"branches");
	strcpy(itemtypes[it].desc,"Looks like a branch.");
	itemtypes[it].basefrequency = 30;
	itemtypes[it].terrfreqmultiplier[terraintype_plains] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_grassland] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_urban] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_sea] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_desert] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_forest] = 2.5;
	itemtypes[it].terrfreqmultiplier[terraintype_mountains] = 1.0;
	itemtypes[it].combatvalue = 0;
	itemtype_branch = it;
	
	it++;
	strcpy(itemtypes[it].name,"lumber");
	strcpy(itemtypes[it].nameplural,"lumber");
	strcpy(itemtypes[it].desc,"Looks like lumber.");
	itemtypes[it].basefrequency = 30;
	itemtypes[it].terrfreqmultiplier[terraintype_plains] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_grassland] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_urban] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_sea] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_desert] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_forest] = 2.0;
	itemtypes[it].terrfreqmultiplier[terraintype_mountains] = 1.0;
	itemtypes[it].combatvalue = 0;
	itemtype_lumber = it;
	
	it++;
	strcpy(itemtypes[it].name,"briefcase of power");
	strcpy(itemtypes[it].nameplural,"briefcases of power");
	strcpy(itemtypes[it].desc,"This black metal briefcase looks government issue"\
		" and has a handle tag with the letters USBAMF. It is noticeably warm to"\
		" the touch. And it vibrates softly and emits a thrumming sound when "\
		"anyone gets near it.");
	itemtypes[it].basefrequency = 30;
	itemtypes[it].terrfreqmultiplier[terraintype_plains] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_grassland] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_urban] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_sea] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_desert] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_forest] = 1.0;
	itemtypes[it].terrfreqmultiplier[terraintype_mountains] = 1.0;
	itemtypes[it].combatvalue = 0;
	itemtypes[it].unique = TRUE;
	itemtype_briefcase_of_power = it;	
}

void genesis_init_you_and_neighbors() {
	int g, p;
	s16int briefx, briefy, dist;
	
	g = 0;
	groups[g].persons[persontype_you] = 1;
	persontypes[persontype_you].unique_inplay = TRUE;
	persontypes[persontype_you].unique_permgone = FALSE;
	groups[g].used = TRUE;
	groups[g].x = yourx;
	groups[g].y = youry;
	groups[g].owner = org_you;
	areas[yourx][youry].groupid = g;
	areas[yourx][youry].terraintype = terraintype_plains;
	groups[g].items[itemtype_food] = YOUR_STARTING_FOOD;
	groups[g].items[itemtype_gun] = 1;
	groups[g].items[itemtype_bullet] = 6;
	groups[g].items[itemtype_crudemap] = 1;
	itemtypes[itemtype_crudemap].unique_inplay = TRUE;
	itemtypes[itemtype_crudemap].unique_permgone = FALSE;
	yourgroup = g;
	
	g++;
	groups[g].used = TRUE;
	groups[g].x = yourx - 1;
	groups[g].y = youry - 1;
	groups[g].owner = org_independent;
	areas[groups[g].x][groups[g].y].groupid = g;
	areas[groups[g].x][groups[g].y].terraintype = terraintype_plains;
	groups[g].persons[persontype_refugee] = FORMER_REFUGEE_COUNT;
	groups[g].items[itemtype_food] = YOUR_STARTING_FOOD * FORMER_REFUGEE_COUNT;
	groups[g].items[itemtype_knife] = 3;
	groups[g].items[itemtype_club] = 2;
	groups[g].items[itemtype_rock] = 6;
	groups[g].persons[persontype_farmer] = 1;
	
	g++;
	groups[g].used = TRUE;
	groups[g].x = yourx + 2;
	groups[g].y = youry;
	groups[g].owner = org_enemy;
	areas[groups[g].x][groups[g].y].groupid = g;
	groups[g].persons[persontype_rat] = 3;
	groups[g].items[itemtype_food] = 5;

	
	persontypes[persontype_you].known = TRUE;	
	persontypes[persontype_refugee].known = TRUE;
	persontypes[persontype_farmer].known = TRUE;		
	persontypes[persontype_rat].known = TRUE;
	
	itemtypes[itemtype_food].known = TRUE;
	itemtypes[itemtype_gun].known = TRUE;
	itemtypes[itemtype_crudemap].known = TRUE;	
	itemtypes[itemtype_bullet].known = TRUE;
	itemtypes[itemtype_rock].known = TRUE;
	itemtypes[itemtype_knife].known = TRUE;
	itemtypes[itemtype_club].known = TRUE;

	
	p = 0;
	piles[p].used = TRUE;
	while (TRUE) {
		briefx = get_rnd_u16int_with_notinclusive_max(MAPX);
		briefy = get_rnd_u16int_with_notinclusive_max(MAPY);
		dist = get_dist(yourx,youry,briefx,briefy);
		if ((dist < 50) || (dist > 200)) continue;
		if (areas[briefx][briefy].terraintype == terraintype_sea) continue;
		break;
	}
	printf("placing briefcase in (%hi,%hi)\n",briefx,briefy);
	piles[p].x = briefx;
	piles[p].y = briefy;
	areas[piles[p].x][piles[p].y].pileid = p;
	piles[p].items[itemtype_briefcase_of_power] = 1;
	itemtypes[itemtype_briefcase_of_power].unique_inplay = TRUE;
	itemtypes[itemtype_briefcase_of_power].unique_permgone = FALSE;	
}

int get_rnd_animal_type() {
	int type;
	
	do {
		type = get_rnd_u16int_with_notinclusive_max(PERSONTYPES);
	} while (!persontypes[type].is_animal);
	return type;
}

void genesis_put_some_rnd_groups_everywhere() {	
	int x, y, newgroup, newgroupcount, qty;
	
	newgroupcount = 0;
	for (x = yourx-50 ; x < yourx+50 ; x++) {
		for (y = youry-50 ; y < youry+50 ; y++) {
			if (!is_valid_xy(x,y)) continue;
			if (is_tooclosetoedge(x,y)) continue;
			if (get_dist(x,y,yourx,youry) < 3) continue;
			if (areas[x][y].groupid != -1) continue;
			if (areas[x][y].terraintype == terraintype_sea) continue;
			if (rollforsuccess(9,10)) continue;
			/*printf("newgroup in (%i,%i)\n",x,y);*/
			newgroup = create_group(x, y, org_independent);
			/*printf("groupid %i\n",newgroup);*/
			if (newgroup != -1) {
				qty = 1;
				while (rollforsuccess(1,2) && (qty < 20)) {
					qty += 1;
				}
				groups[newgroup].persons[get_rnd_animal_type()] = qty;
				newgroupcount++;
			}
		}
	}
	printf("newgroupcount = %i\n",newgroupcount);
}

void new_random_genesis_universe() {
	genesis_init_misc_global_vars();
	genesis_init_orgs();
	genesis_init_terrains();
	genesis_init_map_terrain();
	genesis_init_persontypes();
	genesis_init_itemtypes();
	genesis_init_you_and_neighbors();	
	genesis_put_some_rnd_groups_everywhere();
}

void replace_current_universe_with_new_one() {
	/*printf("replacing current universe with new one\n");*/
	wipe_data();
	new_random_genesis_universe();
}

void program_startup_only() {
	/*printf("program_startup_only()\n");*/
	srand( (unsigned) time(NULL) );
	strcpy(dirnames[0],"0 IS BAD DIRNUM");
	strcpy(dirnames[1],"south-west");
	strcpy(dirnames[2],"south");
	strcpy(dirnames[3],"south-east");
	strcpy(dirnames[4],"west");
	strcpy(dirnames[5],"5 IS BAD DIRNUM");
	strcpy(dirnames[6],"east");
	strcpy(dirnames[7],"north-west");
	strcpy(dirnames[8],"north");
	strcpy(dirnames[9],"north-east");
	
	dir2xrel[0] =  0; dir2yrel[0] =  0;
	dir2xrel[1] = -1; dir2yrel[1] =  1;
	dir2xrel[2] =  0; dir2yrel[2] =  1;
	dir2xrel[3] =  1; dir2yrel[3] =  1;
	dir2xrel[4] = -1; dir2yrel[4] =  0;
	dir2xrel[5] =  0; dir2yrel[5] =  0;
	dir2xrel[6] =  1; dir2yrel[6] =  0;
	dir2xrel[7] = -1; dir2yrel[7] = -1;
	dir2xrel[8] =  0; dir2yrel[8] = -1;
	dir2xrel[9] =  1; dir2yrel[9] = -1;
}

void game_loop() {
	int inputlen;
	char inputline[MAX_SAFE_STRLEN+1];
	
	printf("crudemap desc strlen() = %i\n",strlen(itemtypes[itemtype_crudemap].desc));
	
	
	/*printf("play_game()\n");*/
	status();
	
	do {
		printf("]> ");
		inputlen = getlinechomped(inputline,MAX_SAFE_STRLEN);
		if (strlen(inputline) >= MAX_SAFE_STRLEN) {
			printf("ignored because line too long!\n");
			continue;
		}
		if (streql(inputline,"h") || streql(inputline,"H") ||
				streql(inputline,"help") || streql(inputline,"HELP") ||
			streql(inputline,"?")) {
			handle_help();
		} else if (streql(inputline,"rules")) {
			rules();
		} else if (streql(inputline,"credits")) {
			credits();
		} else if (streql(inputline,"q") || streql(inputline,"Q") ||
				streql(inputline,"quit") || streql(inputline,"QUIT") ||
				streql(inputline,"exit") || streql(inputline,"EXIT")) {
			handle_quit();
		} else if (streql(inputline,"ver")) {
			handle_ver();
		} else if (streql(inputline,"")) {
			clearlastmsg();
			status();
		} else if (streql(inputline,"k")) {
			handle_k();
		} else if (streql(inputline,"1")) {
			moverel(-1,1); status();
		} else if (streql(inputline,"2")) {
			moverel(0,1); status();
		} else if (streql(inputline,"3")) {
			moverel(1,1); status();
		} else if (streql(inputline,"4")) {
			moverel(-1,0); status();
		} else if (streql(inputline,"6")) {
			moverel(1,0); status();
		} else if (streql(inputline,"7")) {
			moverel(-1,-1); status();
		} else if (streql(inputline,"8")) {
			moverel(0,-1); status();
		} else if (streql(inputline,"9")) {
			moverel(1,-1); status();
		} else if (streql(inputline,"r")) {
			handle_r();
		} else if (streql(inputline,"w")) {
			handle_w();
		} else if (streql(inputline,"l")) {
			handle_l();
		} else if (streql(inputline,"t")) {
			handle_t();
		} else if (streql(inputline,"c")) {
			handle_c();
		} else if (streql(inputline,"e")) {
			handle_e();
		} else if (streql(inputline,"p")) {
			handle_p();
		} else if (streql(inputline,"pt")) {
			handle_pt();
		} else if (streql(inputline,"it")) {
			handle_it();
		} else if (streql(inputline,"rp")) {
			handle_rp();
		} else if (streql(inputline,"intro")) {
			handle_intro();
		} else if (streql(inputline,"people")) {
			handle_people();
		} else if (streql(inputline,"items")) {
			handle_items();
		} else if (streql(inputline,"allowall")) {
			handle_allowall();
		} else if (streql(inputline,"refuseall")) {
			handle_refuseall();
		} else if (strbegin(inputline,"dp ")) {
			handle_dp(inputline);
		} else if (strbegin(inputline,"di ")) {
			handle_di(inputline);
		} else if (strbegin(inputline,"d ")) {
			handle_d(inputline);
		} else if (strbegin(inputline,"th ")) {
			handle_th(inputline);
		} else if (strbegin(inputline,"givep ")) {
			handle_givep(inputline);
		} else if (strbegin(inputline,"givei ")) {
			handle_givei(inputline);
		} else if (strbegin(inputline,"transi ")) {
			handle_transi(inputline);
		} else if (strbegin(inputline,"getp ")) {
			handle_getp(inputline);
		} else if (strbegin(inputline,"geti ")) {
			handle_geti(inputline);
		} else if (strbegin(inputline,"grabi ")) {
			handle_grabi(inputline);
		} else if (strbegin(inputline,"drop ")) {
			handle_drop(inputline);
		} else if (strbegin(inputline,"pickup ")) {
			handle_pickup(inputline);
		} else if (strbegin(inputline,"g ")) {
			handle_g(inputline);
		} else if (strbegin(inputline,"a ")) {
			handle_a(inputline);
		} else if (strbegin(inputline,"allow ")) {
			handle_allow(inputline);
		} else if (strbegin(inputline,"refuse ")) {
			handle_refuse(inputline);
		} else if (streql(inputline,"F")) {
			handle_F();
		} else if (streql(inputline,"D")) {
			handle_D();
		} else if (strbegin(inputline,"Dxy ")) {
			handle_Dxy(inputline);
		} else if (streql(inputline,"De")) {
			handle_De();
		} else if (streql(inputline,"Dm")) {
			handle_Dm();
		} else if (streql(inputline,"Dpt")) {
			handle_Dpt();
		} else if (streql(inputline,"Dit")) {
			handle_Dit();
		} else if (strbegin(inputline,"Dcv ")) {
			handle_Dcv(inputline);
		} else if (streql(inputline,"Ddai")) {
			handle_Ddai();
		} else if (streql(inputline,"Ddap")) {
			handle_Ddap();
		} else if (streql(inputline,"foo")) {
			handle_foo();
		} else {
			printf("What?\n");
		}
		
	} while (TRUE);
}

int main(int argc, char *argv[]) {
	program_startup_only();
	replace_current_universe_with_new_one();
	game_loop();
	exit(EXIT_FAILURE);
}
