#include "workload.h"

#define STN_SIZE 50
/*
 98 * 1024 * 1024 = 102760448
 99 * 1024 * 1024 = 103809024
 */

static void save_load_track_to_file( FILE **fpp, int sinario )
{
	switch( sinario )
	{
		case S_NORMAL_DRIVING : *fpp = fopen( SAVE_LOG_NORMAL,  "a+" ); 			break; 
		case S_PARKING 		: *fpp = fopen( SAVE_LOG_PARKING, "a+" ); 			break;
		case S_PARKING_SHOCK  : *fpp = fopen( SAVE_LOG_PARKING_SHOCK, "a+" ); 	break;
		case S_SHOCK			: *fpp = fopen( SAVE_LOG_SHOCK,    "a+" ); 			break;
		case S_SESSION_3 		: *fpp = fopen( SAVE_LOG_SESSION3, "a+" ); 			break;
		case S_SESSION_4		: *fpp = fopen( SAVE_LOG_SESSION4, "a+" ); 			break;
		case S_AUTOMATION 	: *fpp = fopen( SAVE_LOG_AUTOMATION, "a+" ); 		break;

		default : printf("save_track_to_file \n"); 								break;
	}
}
static unsigned long random_range( unsigned int n1, unsigned int n2 )
{
	srand( time( NULL ) );
	return ( unsigned long )( rand() % ( n2 - n1 ) + n1 );
}
static unsigned long dir_size( char *dn)
{
	FILE *fp = NULL;
//	char *stn = "du -scb /mnt/normal/* | grep 합계";
	char stn[ STN_SIZE ];
	char *ch[2]  = {  "du -scb ", "* | grep total" };

	char buf[50];
	char *size = NULL;
	unsigned long l_size = 0;

	//printf( "This is dir_size( ) : %s \n", dn );
	sprintf( stn,  "%s%s%s",ch[0],dn,ch[1] ); 

//	printf("stn : %s \n", stn );

	fp = popen(stn,"r");
	if(fp == NULL) {
		printf("Error opening : %s\n", strerror( errno ));
		printf("stn : %s \n", stn );
		printf("total file counter : %lu \n", g_total.file_counter );
		
//		g_time.end_point = clock();

//		printf("Execution time : %f sec \n", (double)( g_time.end_point - g_time.start_point ) / CLOCKS_PER_SEC );

		fclose( g_fp );
		exit(-1);
	}

	fgets( buf,sizeof(buf),fp);

	size = strtok(buf,"\t");

	pclose( fp );

//	l_size = atol( size );
	l_size = strtoul( size, NULL, 10 );

//	printf("dir_size : %luK \n", l_size / 1024  );

	return l_size;
}

/*
 17.01.23
 mode_flag을 통해 opel_fat인지 original_fat인지 구분을 통해 
 저장된 로그를 로드 할 것인지 결정 했었는데

 load_flag를 통해 이를 대신 함 ( 두 경우 모드 load_flag를 사용할 수 도록 )
 
 */
static unsigned long f_rand_size( int *selected_dir, int sinario, int load_flag )
{
	int dir_num = 0;
	char buf[ STN_SIZE ];
	char *token = NULL;
	unsigned long result = 0;

	if( load_flag == OFF ) 
	{
		save_load_track_to_file( &g_fp, sinario ); //original에 테스트를 opel 테스트에서 동일하게 적용하기 위해서 저장시킴

		if( sinario != S_AUTOMATION )
		{
			result = random_range( 98 * 1024 * 1024, 99 * 1024 * 1024 ); 
		}
		else  //AUTOMATION
		{
			switch( *selected_dir )
			{
				//  범위
				case NORMAL : result = random_range( 98 * 1024 * 1024, 99 * 1024 * 1024 ); 	break;//상시
				case NORMAL_EVENT : result = random_range( 53 * 1024 * 1024, 73 * 1024 * 1024 );	break;//상시 이벤트
				case PARKING : result = random_range( 52 * 1024 * 1024, 58 * 1024 * 1024 ); 	break;//주차
				case PARKING_EVENT : result = random_range( 8 * 1024 * 1024, 87 * 1024 * 1024 ); 	break; //주차 이벤트
				default : break;
			}
		}	
		
		fprintf( g_fp, "%d\t%lu\n",*selected_dir, result );
		fclose( g_fp );

		printf("result : %lu, selected_dir : %d \n ", result, *selected_dir );
	}	
	else if( load_flag == ON )
	{
		if( fgets( buf, sizeof( buf ), g_fp ) == NULL )
		{
			printf("Log End \n");
			printf("%d\n", g_line_to_read ); 
			
			fclose( g_fp );

			//g_time.end_point = clock();
			//printf("Execution time : %f sec \n", (double)( g_time.end_point - g_time.start_point ) / CLOCKS_PER_SEC );

			exit(1);
			
		}
	//	printf("buf : %s \n", buf );
		
		token = strtok( buf, "\t" );
		dir_num = atoi( token );
		token = strtok( NULL, "\t" );
		
		result = strtoul( token, NULL, 10 );
	
		*selected_dir = dir_num;

//		printf(" dir_num : %d result : %lu \n",dir_num, result );
	}
	else 
	{
		fclose( g_fp );
		exit(-1);
	}
	return result;
}
static int detect_file_counter( int file_counter )
{
	if( file_counter >= g_line_to_read )
	{
		printf("Detect_file_counter is operating \n");
		printf("%d\n", g_line_to_read ); 

		fclose( g_fp );	
		exit(1);
	}
	else; 
//		printf("detect_file_counter error \n");


	return 0;
}
void file_create(char **dirs, int *selected_dir, int sinario, int load_flag )
{
	FILE *fd;
	char fn[10];//, in_name[10];
	char temp_full_file[ NAME_SIZE ];
	int k;
	
	static int file_creator[ DIR_NUM ] = { 0, };
	unsigned long f_size = 0;

	f_size = f_rand_size( selected_dir, sinario, load_flag );

	//printf("f_size : %luM \t", f_size/1024/1024 );

	switch( *selected_dir )
	{
	//	case ETC 			: file_creator[ ETC ]++; break;
		case NORMAL 		: file_creator[ NORMAL ]++; break;
		case NORMAL_EVENT 	: file_creator[ NORMAL_EVENT ]++; break;
		case PARKING		: file_creator[ PARKING ]++; break;
		case PARKING_EVENT 	: file_creator[ PARKING_EVENT ]++; break;
		case HANDWORK		: file_creator[ HANDWORK ]++; break;
		default : printf("*selected_dir error \n"); break;
	}

	if( load_flag == ON )
	{	
		printf("File name : %d \t", file_creator[ *selected_dir ] );
		sprintf(fn,"%s%d", dirs[ *selected_dir ], file_creator[ *selected_dir ] );

		//실제 타겟 파일에 설정된 크기 만큼, 파일을 생성하고 씀
		if((fd = fopen(fn,"w")) == NULL) {
			printf("File create error\n");
			exit(-1);
		}
		for( k=0 ; k < f_size ; k++)
			fputs("k",fd);
	
		fclose(fd);
	
		g_total.file_counter++;
		printf("File create: %s \t\t %8luM \t dir_size(): %10luM \n",fn, f_size/1024/1024, dir_size( dirs[ *selected_dir ] )/1024/1024 );
		
		detect_file_counter( g_total.file_counter );
	}
	else //if load_flag is off, this program make logs
	{
	
	
	}
}

