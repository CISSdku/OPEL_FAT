#ifndef __CONTROL_FUNCTION_H__
#define __CONTROL_FUNCTION_H__

#define SAVE_LOG_NORMAL 		"./written_log/normal_driving.txt"
#define SAVE_LOG_PARKING 		"./written_log/parking.txt"
#define SAVE_LOG_PARKING_SHOCK  "./written_log/parking_shock.txt"
#define SAVE_LOG_SHOCK 			"./written_log/shock.txt"
#define SAVE_LOG_SESSION3 		"./written_log/session3.txt"
#define SAVE_LOG_SESSION4 		"./written_log/session4.txt"
#define SAVE_LOG_AUTOMATION		"./written_log/automation.txt"

#define ORIGINAL 0
#define OPEL 1

#define NAME_SIZE 100
#define STRING_SIZE 100

#define F_SIZE 8192
#define F_NUM 5
#define FD_SIZE (( F_SIZE ) * ( F_NUM ))

#define ON 1
#define OFF 0
#define DIR_NUM 5
/*
#define DIR1_SIZE ( 512 * pow( 1024, 2 ) )
#define DIR2_SIZE ( 2 * pow( 1024, 3 ) )
#define DIR3_SIZE ( 1 * pow( 1024, 3 ) )
#define DIR4_SIZE ( 512 * pow( 1024, 2) )
#define DIR5_SIZE ( 512 * pow( 1024, 2) )
#define DIR6_SIZE ( 512 * pow( 1024, 2) )
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <errno.h>

#include<fcntl.h>
#include<unistd.h>

/*
typedef enum
{
	DIR_1 = 0, 	//etc
	DIR_2, 		//normal
	DIR_3, 		//normal event
	DIR_4,		//parking
	DIR_5,		//parking event
	DIR_6 		//handwork
}dir_counter_e; 
*/
typedef enum //selected_dir
{
//	ETC  = 0, 			//etc
	NORMAL = 0,			//normal
	NORMAL_EVENT, 		//normal event
	PARKING,			//parking
	PARKING_EVENT,		//parking event
	HANDWORK 			//handwork
}dir_counter_e; 
typedef enum 
{
	S_NONE = 0,
	S_NORMAL_DRIVING,
	S_PARKING,
	S_PARKING_SHOCK,
	S_SHOCK,	
	S_SESSION_3,
	S_SESSION_4,

	S_AUTOMATION
}sinario_e;

typedef struct each_data_t
{
	unsigned long dir[ DIR_NUM ];	
	int file[ DIR_NUM ];	
	int file_num[ DIR_NUM ];
	long long sum_each_files[ DIR_NUM ];
}data;

typedef struct directory_full_t
{
	int flag;
	int count;
	char last_name[ NAME_SIZE ];
	unsigned int check_portion;
}dir_full;

typedef struct total_count_t
{
	unsigned long file_counter;

}total;

typedef struct time_check_t
{
	clock_t start_point,
			end_point;
}time_check;

data g_data;
dir_full g_dirfull[ DIR_NUM ];
total g_total;
FILE *g_fp;

time_check g_time;

int g_line_to_read;

void file_create( char **dirs, int *selected_dir, int sinario, int load_flag );
#endif
