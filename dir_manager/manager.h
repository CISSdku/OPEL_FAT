#ifndef __MANAGER_H__
#define __MANAGER_H__

#define NAME_SIZE 50
#define STRING_SIZE 50
#define DIR_COUNT 20

#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>
#include <limits.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <dirent.h>


#define UNIT_CON ( 1024 ) 

//단취 K 단위로 맞출려고 1024로 나눈거임
#if 0
//#define ETC_SIZE 			( ( 512 * pow( 1024, 2 ) ) / UNIT_CON )
#define NORMAL_SIZE 		( ( 2 * pow( 1024, 3 ) )   / UNIT_CON )
#define NORMAL_EVENT_SIZE 	( ( 1 * pow( 1024, 3 ) )   / UNIT_CON )
#define PARKING_SIZE 		( ( 1 * pow( 1024, 3) )  / UNIT_CON )
#define PARKING_EVENT_SIZE  ( ( 512 * pow( 1024, 2) )  / UNIT_CON ) 
#define HANDWORK_SIZE 		( ( 512 * pow( 1024, 2) )  / UNIT_CON )
#endif
//#define ETC_SIZE 			( ( 512 * pow( 1024, 2 ) ) / UNIT_CON )
#define NORMAL_SIZE 		( ( 40 * pow( 1024, 2 ) )   / UNIT_CON )
#define NORMAL_EVENT_SIZE 	( ( 20 * pow( 1024, 2 ) )   / UNIT_CON )
#define PARKING_SIZE 		( ( 20 * pow( 1024, 2) )  / UNIT_CON )
#define PARKING_EVENT_SIZE  ( ( 10 * pow( 1024, 2) )  / UNIT_CON ) 
#define HANDWORK_SIZE 		( ( 10 * pow( 1024, 2) )  / UNIT_CON )
#if 0
typedef enum
{
	DIR_1 = 0,
	DIR_2,      //normal
	DIR_3,      //normal event
	DIR_4,      //parking
	DIR_5,      //parking event
	DIR_6
}dir_counter_e;
#endif

typedef enum 
{
//	ETC  = 0,           //etc
	NORMAL = 0,            //normal
	NORMAL_EVENT,       //normal event
	PARKING,            //parking
	PARKING_EVENT,      //parking event
	HANDWORK            //handwork
}dir_counter_e;



typedef struct dir_data_t
{
	unsigned long dir_size;
	unsigned long check_portion;
	int full_count;

}dir_info;

dir_info *g_dir;


void detect_and_control( char **dirs, int dir_cnt );


#endif
