#include "workload.h"

#define STN_SIZE 100
/*
 98 * 1024 * 1024 = 102760448
 99 * 1024 * 1024 = 103809024
 */

char err_msg[50] = {"Success"};

static void save_load_track_to_file( FILE **fpp, int sinario )
{
	switch( sinario )
	{
		case S_NORMAL_DRIVING 		: *fpp = fopen( SAVE_LOG_NORMAL,  		"a+" ); 			break; 
		case S_NORMAL_DRIVING_SHOCK : *fpp = fopen( SAVE_LOG_NORMAL_SHOCK,  "a+" ); 			break; 
		case S_PARKING 				: *fpp = fopen( SAVE_LOG_PARKING, 		"a+" ); 			break;
		case S_PARKING_SHOCK  		: *fpp = fopen( SAVE_LOG_PARKING_SHOCK, "a+" ); 			break;
		case S_HANDWORK				: *fpp = fopen( SAVE_LOG_HANDWORK,   	"a+" ); 			break;
		
		case S_AUTOMATION 			: *fpp = fopen( SAVE_LOG_AUTOMATION, "a+" ); 				break;

		default : printf("save_track_to_file \n"); 												break;
	}
}
static unsigned long random_range( unsigned int n1, unsigned int n2 )
{
//	srand( time( NULL ) );
	return ( unsigned long )( rand() % ( n2 - n1 ) + n1 );
}
static unsigned long dir_size( char *dn)
{
	FILE *fp = NULL;
//	char *stn = "du -scb /mnt/normal/* | grep 합계";
	char stn[ STN_SIZE ];
	char *ch[2]  = {  "du -scb ", "* | grep 합계" };
//	char *ch[2]  = {  "du -scb ", "* | grep total" };

	char buf[50];
	char *size = NULL;
	unsigned long l_size = 0;

//	printf( "This is dir_size( ) : %s \n", dn );
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
		//	result = random_range( 98 * 1024 * 1024, 99 * 1024 * 1024 ); 
		//	result = random_range( 7 * 1024 * 1024, 8 * 1024 * 1024 ); 
		//	result = random_range( 512 * 1024, 1 * 1024 * 1024 ); 
//			result = 30 * 1024 * 1024;
	//		result = random_range( 10 * 1024 * 1024, 32 * 1024 * 1024 );
			result = 	32 * 1024 * 1024; 

		}
		else  //AUTOMATION
		{
			switch( *selected_dir )
			{
				//  범위
#if 1
				case NORMAL 		: result = random_range( 10 * 1024 * 1024, 32 * 1024 * 1024 );    break;//상시
				case NORMAL_EVENT 	: result = random_range( 10 * 1024 * 1024, 32 * 1024 * 1024 );  break;//상시 이벤트
				case PARKING 		: result = random_range( 10 * 1024 * 1024, 32 * 1024 * 1024 );   break;//주차
				case PARKING_EVENT  : result = random_range( 10 * 1024 * 1024, 32 * 1024 * 1024 );     break; //주차 이벤트
#endif
#if 0
				case NORMAL 		: result = 	28 * 1024 * 1024;   break;//상시
				case NORMAL_EVENT 	: result =  28 * 1024 * 1024; break;//상시 이벤트
				case PARKING 		: result =  28 * 1024 * 1024;  break;//주차
				case PARKING_EVENT  : result =  28 * 1024 * 1024;    break; //주차 이벤트
#endif

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

/*
	프로그램 수행 시에 설정한 파일 갯수 만큼 수행되면,
	단편화 체크를 위해 파일 일부를 지우고 
	단편화 체크 모듈을 삽입할 준비를 함
 */

struct file_info {
		char fn[50];
		unsigned long time;
		long point;
};

int compare_time(const void *node1, const void *node2)
{
	unsigned long time1,time2;
	long point1,point2;

	time1 = ((struct file_info*)node1)->time;
	point1 = ((struct file_info*)node1)->point;
	time2 = ((struct file_info*)node2)->time;
	point2 = ((struct file_info*)node2)->point;

	if(time1 < time2)
		return -1;
	else if(time1 > time2)
		return 1;
	else {
		if(point1 < point2)
			return -1;
		else if(point1 > point2)
			return 1;
		else
			return 0;
	}
}
void fdel(char *dn, int gap)
{
	DIR *dir;
	FILE *po;
	struct dirent *de;
	struct stat st;
	struct file_info *farr = NULL;
	char fn[50];
	char numch[50];
	char openpath[50];
	int fs_err=0;
	int filenum=0, filecnt=0;

	sprintf(openpath,"%s%s%s","find ",dn," -type f | wc -l");
	po = popen(openpath,"r");
	if(po != NULL) {
		fgets(numch,sizeof(numch),po);
	}
	filenum = atoi(numch);

	dir = opendir(dn);
	if(dir != NULL)
	{
		if(!farr) {
			if(!(farr = (struct file_info *)calloc(filenum,sizeof(struct file_info)))) {
				printf("farr calloc error\n");
				exit(-1);
			}
		}

		while(de = readdir(dir))
		{
			if(!strcmp(de->d_name,".") || !strcmp(de->d_name,".."))
				continue;

			sprintf(fn,"%s%s",dn,de->d_name);

			if((fs_err = stat(fn,&st)) == -1) {
				printf("stat error\n");
				exit(-1);
			}

			strcpy(farr[filecnt].fn,fn);
			farr[filecnt].time = st.st_mtime;
			farr[filecnt].point = st.st_mtim.tv_nsec;

			filecnt++;
		} 
	}

	/*quick sort*/
	qsort(farr,filenum,sizeof(struct file_info),compare_time);

	/*sort check print*/
	/*	filecnt=0;
		while(filecnt < filenum) {
		printf("%s %lu %ld\n",farr[filecnt].fn,farr[filecnt].time,farr[filecnt].point);
		filecnt++;
		}
	 */
	filecnt=0;
	/*file gap delete*/
	while(filecnt < filenum) {
		remove(farr[filecnt].fn);
		printf("%s\n",farr[filecnt].fn);
		filecnt += gap;
	}

	free(farr);
	closedir( dir );
}

#if 0
static void remove_file_for_calculate_fragmentation( char *dn )
{
	DIR *dir;
	struct dirent *de;
	struct stat st, 
				temp_st;
	char fn[40], old_name[40], time_sequence[11];
	int cnt    = 0,
		fs_err = 0;
		

	unsigned long old_time = ULONG_MAX;
	long decimal_point = LONG_MAX;

	dir = opendir(dn);
	if(dir != NULL)
	{
		while(de = readdir(dir))
		{
			if(!strcmp(de->d_name,".") || !strcmp(de->d_name,".."))
				continue;
			
			cnt++; //total file cnt in dir
		}
	}
	closedir( dir );

	dir = opendir(dn);
	if(dir != NULL)
	{
		while(de = readdir(dir))
		{
			if(!strcmp(de->d_name,".") || !strcmp(de->d_name,".."))
				continue;

			sprintf(fn,"%s%s",dn,de->d_name);

			if((fs_err = stat(fn,&st)) == -1) {
				printf("stat error\n");
				exit(-1);
			}

			if( old_time > st.st_mtime )
			{
				old_time = st.st_mtime;
				
				decimal_point = st.st_mtim.tv_nsec;
				strcpy(old_name,fn);

				temp_st = st;
			}
			else if( old_time == st.st_mtime )
			{
				if( decimal_point >=  st.st_mtim.tv_nsec )
				{
					decimal_point = st.st_mtim.tv_nsec;
					strcpy(old_name,fn);
					temp_st = st;
				}
			}
			else;

			
		}
	}


	remove( old_name );
	closedir( dir );
}
#endif


static int detect_file_counter( int file_counter, int load_flag )
{
	if( file_counter >= g_line_to_read )
	{
		printf("Detect_file_counter is operating \n");
		printf("%d\n", g_line_to_read ); 
	
	//	remove_file_for_calculate_fragmentation( "/mnt/normal/" );
	//	remove_file_for_calculate_fragmentation( "/mnt/normal_event/" );
	//	remove_file_for_calculate_fragmentation( "/mnt/parking/" );
	//	remove_file_for_calculate_fragmentation( "/mnt/parking_event" );

		fdel( "./mnt/normal/", 20 );
		fdel( "./mnt/normal_event/", 20 );
		fdel( "./mnt/parking/", 20 );
		fdel( "./mnt/parking_event/", 20 );

		
		if( load_flag == ON )
			fclose( g_fp );	

		exit(1);
	}
	else; 

	return 0;
}

char dummy_data [ 1024 ];

void thread_file_create(char **dirs, int selected_dir, int load_flag )
{
	FILE *fd1;
	char fn1[ NAME_SIZE ] = {0,};//, in_name[10];
	char temp_full_file[ NAME_SIZE ];
	int k;
	
	static int file_creator[ DIR_NUM ] = { 0, };
	unsigned long f_size = 0;
	static unsigned long sleep_cnt = 1;
	int retval = 0;
	int setcond = 0;
	//f_size = f_rand_size( &selected_dir, S_AUTOMATION, load_flag );
	
	f_size = 16500000 + rand()%10000000;
//	printf("f_size : %luM \t", f_size/1024/1024 );

	switch( selected_dir )
	{
		//	case ETC 			: file_creator[ ETC ]++; break;
		case NORMAL 		: file_creator[ NORMAL ]++; 		break;
		case NORMAL_EVENT 	: file_creator[ NORMAL_EVENT ]++; 	break;
		case PARKING		: file_creator[ PARKING ]++; 		break;
		case PARKING_EVENT 	: file_creator[ PARKING_EVENT ]++; 	break;
		case HANDWORK		: file_creator[ HANDWORK ]++; 		break;

		default : printf("*selected_dir error \n"); 			break;
	}

	if( load_flag == ON )
	{	
//		printf("File name : %d \t", file_creator[ selected_dir ] );
		sprintf(fn1,"%s%d_%d%s", dirs[ selected_dir -1 ], file_creator[ selected_dir ], pthread_self(),".avi"  );
		printf("File : %s\n", fn1);

#if 1
		while(1)
		{
			//if( (fd1 = fopen(fn1,"w")) ){
			if( (fd1 = fopen(fn1,"w"))<0 ){
				printf("File create error\n");
				exit(-1);
			}

			for( k=0 ; k < f_size ; k++)
			{
				retval = fwrite( "k",1, 1, fd1);

				if( retval <= 0 ) {
					printf("retval : %d \n", retval );
					setcond = 1;
					printf("%s file \n", fn1 );
					perror("fwrite");

					fclose( fd1 );
					fd1 = NULL;
					sleep(10);
					break;
				}

				if( setcond ) {
					printf("no write\n");
					sleep(10);
				}
			}
			setcond = 0;
			if( fd1 )
			{
				fclose( fd1 );	
				break;
			}
		}
#endif
	}
}

#if 1
void file_create(char **dirs, int *selected_dir, int sinario, int load_flag )
{
	FILE *fd1;
	FILE *fd2;
//	int fd;
	char fn1[ NAME_SIZE ];//, in_name[10];
	char fn2[ NAME_SIZE ];//, in_name[10];
	char temp_full_file[ NAME_SIZE ];
	int k;
	int sleep_cnt = 1;
	
	static int file_creator[ DIR_NUM ] = { 0, };
	unsigned long f_size = 0;


	f_size = f_rand_size( selected_dir, sinario, load_flag );

	//printf("f_size : %luM \t", f_size/1024/1024 );
	switch( *selected_dir )
	{
	//	case ETC 			: file_creator[ ETC ]++; break;
		case NORMAL 		: file_creator[ NORMAL ]++; 		break;
		case NORMAL_EVENT 	: file_creator[ NORMAL_EVENT ]++; 	break;
		case PARKING		: file_creator[ PARKING ]++; 		break;
		case PARKING_EVENT 	: file_creator[ PARKING_EVENT ]++; 	break;
		case HANDWORK		: file_creator[ HANDWORK ]++; 		break;

		default : printf("*selected_dir error \n"); 			break;
	}

	if( load_flag == ON )
	{	
		printf("File name : %d \t", file_creator[ *selected_dir ] );
		sprintf(fn1,"%s%d%s", dirs[ *selected_dir -1 ], file_creator[ *selected_dir ], ".avi"  );

		printf("File name : %d \t\n", ++file_creator[ *selected_dir ]   );
		sprintf(fn2,"%s%d%s", dirs[ *selected_dir -1 ], file_creator[ *selected_dir ], ".avi"  );

		//sprintf(fn,"%s%d", dirs[ *selected_dir -1 ], file_creator[ *selected_dir ] );
		//printf("%s \n", fn );
		//실제 타겟 파일에 설정된 크기 만큼, 파일을 생성하고 씀
#if 1
		if( (fd1 = fopen(fn1,"w")) == NULL || (fd2 = fopen(fn2,"w")) == NULL ) {

			printf("g_total.file_counter : %lu \n", g_total.file_counter );	
			printf("File create error\n");
			exit(-1);
		}

		for( k=0 ; k < f_size ; k++)
		{
			
			fputs("k",fd1);
			fputs("k",fd2);
	//		fprintf( stderr, "%s\n", strerror(errno));

			
		}

		fclose( fd1 );
		fclose( fd2 );
		//	fflush(stdout);
#endif
#if 0
		fd = open( fn, O_WRONLY | O_CREAT, 0644);
		if( !fd )
		{
			printf("Fail workload file creatation\n");			
			exit(-1);
		}
		for( k = 0 ; k < f_size ; k++ )
			write( fd, "k", strlen( "k" ) );		

		close( fd );
#endif
		g_total.file_counter++;
		//printf("File create: %-20s \t %10luM \t dir_size(): %10luM \n",fn, f_size/1024/1024, dir_size( dirs[ *selected_dir ] )/1024/1024 );
		printf("File create: %-20s %-20s\t %10luK \t dir_size(): %10luK \n", fn1, fn2, f_size/1024, dir_size( dirs[ *selected_dir - 1 ] )/1024 );

		//		detect_file_counter( g_total.file_counter, load_flag );
	}
	else //if load_flag is off, this program make logs
	{
		g_total.file_counter++;
		//		detect_file_counter( g_total.file_counter, load_flag );
	}
}
#endif

#if 0
void file_create(char **dirs, int *selected_dir, int sinario, int load_flag )
{
	FILE *fd;
//	int fd;
	char fn[ NAME_SIZE ];//, in_name[10];
	char temp_full_file[ NAME_SIZE ];
	int k;
	
	static int file_creator[ DIR_NUM ] = { 0, };
	unsigned long f_size = 0;

	f_size = f_rand_size( selected_dir, sinario, load_flag );

	//printf("f_size : %luM \t", f_size/1024/1024 );
	switch( *selected_dir )
	{
	//	case ETC 			: file_creator[ ETC ]++; break;
		case NORMAL 		: file_creator[ NORMAL ]++; 		break;
		case NORMAL_EVENT 	: file_creator[ NORMAL_EVENT ]++; 	break;
		case PARKING		: file_creator[ PARKING ]++; 		break;
		case PARKING_EVENT 	: file_creator[ PARKING_EVENT ]++; 	break;
		case HANDWORK		: file_creator[ HANDWORK ]++; 		break;

		default : printf("*selected_dir error \n"); 			break;
	}

	if( load_flag == ON )
	{	
		printf("File name : %d \t", file_creator[ *selected_dir ] );
		sprintf(fn,"%s%d%s", dirs[ *selected_dir -1 ], file_creator[ *selected_dir ], ".avi"  );
		//sprintf(fn,"%s%d", dirs[ *selected_dir -1 ], file_creator[ *selected_dir ] );
		//printf("%s \n", fn );
		//실제 타겟 파일에 설정된 크기 만큼, 파일을 생성하고 씀
#if 1
		if((fd = fopen(fn,"w")) == NULL) {
			printf("g_total.file_counter : %lu \n", g_total.file_counter );	
			printf("File create error\n");
			exit(-1);
		}

		for( k=0 ; k < f_size ; k++)
			fputs("k",fd);
	
		fclose( fd );
	//	fflush(stdout);
#endif
#if 0
		fd = open( fn, O_WRONLY | O_CREAT, 0644);
		if( !fd )
		{
			printf("Fail workload file creatation\n");			
			exit(-1);
		}
		for( k = 0 ; k < f_size ; k++ )
			write( fd, "k", strlen( "k" ) );		

		close( fd );
#endif
		g_total.file_counter++;
		//printf("File create: %-20s \t %10luM \t dir_size(): %10luM \n",fn, f_size/1024/1024, dir_size( dirs[ *selected_dir ] )/1024/1024 );
		printf("File create: %-20s \t %10luK \t dir_size(): %10luK \n",fn, f_size/1024, dir_size( dirs[ *selected_dir - 1 ] )/1024 );
		
//		detect_file_counter( g_total.file_counter, load_flag );
	}
	else //if load_flag is off, this program make logs
	{
		g_total.file_counter++;
//		detect_file_counter( g_total.file_counter, load_flag );
	}
}

#endif
