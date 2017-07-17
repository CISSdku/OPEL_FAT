#include"manager.h"

/*
   1. 관리할 디렉터리 설정
		a. 기존에는 workload 안에서 위치가 미리 셋팅되고 만들면서 크기를 관리 하였음
		b. manager만 따로 수행하기 때문에 관리 대상이 되는 전체 디렉터리를 체크하고 관리해야 함
   2. 관리 대상 사이즈 확인
*/

static int extrat_setting_value( char **config_line, int *config_ratio )
{
	int i = 0, j =0; 
	char *str;

	while( 1 )
	{
		str = strstr( *( config_line + i ), "=" );	
		
		if( str )
		{
		//	printf("config ratio : %s\n", str + 1 );	

			*(config_ratio + j) = atoi( str + 1 );

			j++;
		}

		if( strstr( *( config_line + i ), "Preallocation" ) )
		{
			return j;	
		}
		
		i++;

	}
}

static int open_files( char *target_direcotry, char **dirs, char ***pbuf )
{
	FILE *fp = fopen( target_direcotry, "r" );	
	int cnt = 0,
		i = 0;
	char buf[ STRING_SIZE ]; 


	while(1)
	if( fgets( buf, sizeof( buf ), fp ) != NULL )
		cnt++;
	else 
	{
		fclose( fp );		
//		printf("%d \n", cnt );
		break;
	}

	*pbuf = ( char ** )malloc( sizeof(char *) * STRING_SIZE );	
	for( i = 0 ; i < cnt ; i++ )
		*( *pbuf + i ) = ( char * )malloc( sizeof( char ) * STRING_SIZE );

	cnt = 0;
	fp = fopen( target_direcotry, "r" ); 
	

	while( 1 )
	if( fgets( buf, sizeof( buf ), fp ) == NULL )
	{
	//	printf("total line num : %d \n", cnt );
		goto out;	
	}
	else
	{
//		printf("buf : %s", buf );

		buf[ strlen( buf ) - 1 ] = '\0';
		*( dirs + cnt ) = strcpy( *( *pbuf + cnt ), buf );	 
		//printf("%s, %s", *( *pbuf + cnt ), *( dirs + cnt ) );
		cnt++;
	}

out:

#if 0
	if( !(strcmp( target_direcotry, "/sys/fs/OPEL_FAT/control" ) ) )
	{
		if( strstr( *dirs, "START" ) == NULL )
		{
//			printf("아직 START 아니여서 FREE");
			free( *pbuf );
		}	
	}
#endif

	return cnt;;
}
static void view_dirs( char **dirs, int dir_cnt )
{
	int i;

	for( i = 0 ; i < dir_cnt ; i++ ) 
		printf("%s\n", *( dirs + i ) );

}

static void total_size_SD_card( unsigned long *total_size )
{
	int n;
	FILE *fp = NULL;
	char buf[50];
	char *size = NULL;

	fp = popen( "df -h | grep /dev/mmcblk1" ,"r" ); // 수정

	if( fp == NULL )
		printf("Error opening : %s\n", strerror( errno ) );	

	fgets( buf, sizeof( buf ), fp );
	size = strtok( buf, " " );
//	printf("SDcard Size : %s \n", size );
	size = strtok( NULL, " " );
	size = strtok( size, "G" );

//	printf("SDcard total sd size :%s \n", size );

	//*total_size = strtoul( size, NULL, 10 ) * pow( 1024, 3 );

//	printf("test : %lf \n", atof( size ) * pow( 1024, 2 ) ); //GB단위인데 기본이 MB로 처리해서

	*total_size = atof( size ) * pow( 1024, 2 );

	pclose( fp );
}

//일단 대충 짜자
static void current_size_SD_card( unsigned long *current_size )
{
	int n;
	FILE *fp = NULL;
	char buf[50];
	char *size = NULL;
	int tmp_len;

	fp = popen( "du -sh /home/odroid/mount/" ,"r" ); // 수정

	if( fp == NULL )
		printf("Error opening : %s\n", strerror( errno ) );	

	fgets( buf, sizeof( buf ), fp );

	//tmp_len = strlen( size)
	//printf("-------------size : %ld\n", strlen( buf ));
	tmp_len = strlen( buf );

	size = strtok( buf, "G" );
//	printf("[G] current_size_SD_card size : %s	 %u \n", size, strlen( size ) );
	if( tmp_len != strlen( size ) )
	{
		*current_size = (unsigned long )( (double)(atof( size ) * pow( 1024, 2 )) ); //GB단위인데 기본이 MB로 처리해서
		pclose( fp );
		return;	
	}
	size = strtok( buf, "M" );
//	printf("[M] current_size_SD_card size : %s	 %u \n", size, strlen( size ) );
	if( tmp_len != strlen( size ) )
	{
		*current_size = (unsigned long )((double)atof( size ) * pow( 1024, 1 ));
		pclose( fp );
		return;	
	}
	size = strtok( buf, "K" );
//	printf("[K] current_size_SD_card size : %s \n", size );
	if( tmp_len != strlen( size ) )
	{
		*current_size = (unsigned long)atof( size );
		//printf("[K] current_size : %lu \n", *current_size );

		pclose( fp );
		return;	
	}

}

void init( int dir_cnt, char **dirs, char **config_line, char *sysfs_line )
{
	int i = 0;
	int config_ratio[ 10 ] = { 0, };
	int config_cnt;
	unsigned long SDcard_total_size;
	unsigned long SDcard_current_size;
	unsigned long control_size;

	
	g_dir = ( dir_info * )malloc( sizeof( dir_info ) * dir_cnt ); 
	memset( ( void *)g_dir, 0x0, sizeof( g_dir ) * dir_cnt );
	/////
	config_cnt = extrat_setting_value( config_line, config_ratio );
	total_size_SD_card( &SDcard_total_size );
	//특정 파티션 FULL 상태일 때 남은 공간을 파악해야 함

	current_size_SD_card( &SDcard_current_size );
	printf("SD Total Size : %lu, Current SD Size : %lu \n", SDcard_total_size, SDcard_current_size );

//	printf("test : %s \n", sysfs_line );

	//if( !strcmp( "START_ORIGINAL", sysfs_line ) )
	if( strstr( sysfs_line, "ORIGINAL" ) )
	{
		//OPEL FAT의 특정 파티션이 사용자에 의해 FULL 상태로 되어 기존 Default FAT으로 동작하고 
		//그에 따라 메니저가 관리하는 디렉터리의 크기도 수정되어야 한다.
		printf("START_ORIGINAL\n");
		control_size = SDcard_total_size - SDcard_current_size;
	}
	else //START_OPEL
	{
		//파일 시스템 OPEL_FAT 으로 동작
		printf("START_OPEL\n");
		control_size = SDcard_total_size;
	}
	printf("control_size : %lu \n\n", control_size );

	for( i = 0 ; i < config_cnt ; i++ )
		printf("config_ratio : %d \n", config_ratio[ i ] );

#if 1
	g_dir[ NORMAL ].dir_size 		= ( control_size * ( (double)config_ratio[0] / 100 ) ) * 0.95;
	g_dir[ NORMAL_EVENT ].dir_size  = ( control_size * ( (double)config_ratio[1] / 100 ) ) * 0.95;
	g_dir[ PARKING ].dir_size 		= ( control_size * ( (double)config_ratio[2] / 100 ) ) * 0.95;
	g_dir[ PARKING_EVENT ].dir_size = ( control_size * ( (double)config_ratio[3] / 100 ) ) * 0.95;
	g_dir[ HANDWORK ].dir_size 		= ( control_size * ( (double)config_ratio[4] / 100 ) ) * 0.95;
//	g_dir[ ETC ].dir_size 			= control_size * ( (double)config_ratio[5] / 100 ); 				 //etc는 그냥 dummy

	printf("%lu %lu %lu %lu %lu\n", g_dir[ NORMAL ].dir_size, g_dir[ NORMAL_EVENT ].dir_size, g_dir[ PARKING ].dir_size, g_dir[ PARKING_EVENT ].dir_size, g_dir[ HANDWORK ].dir_size );
#endif

	//while(1);
#if 0

	//전체 크기에서 1% 뺀 크기를 기준으로 디렉터리가 꽉 찼는지 판단
//	g_dir[ ETC ].dir_size 			= ETC_SIZE;  //etc는 그냥 dummy
	g_dir[ NORMAL ].dir_size 		= NORMAL_SIZE;
	g_dir[ NORMAL_EVENT ].dir_size  = NORMAL_EVENT_SIZE;
	g_dir[ PARKING ].dir_size 		= PARKING_SIZE;
	g_dir[ PARKING_EVENT ].dir_size = PARKING_EVENT_SIZE;
	g_dir[ HANDWORK ].dir_size 		= HANDWORK_SIZE;

	for( i = 0 ; i <= dir_cnt ; i++ )
		printf("%luM ", g_dir[ i ].dir_size / 1024  ); // M 단위

//	g_dir[ ETC ].dir_size 			= ETC_SIZE 		     - ( ETC_SIZE * 0.01 );
	g_dir[ NORMAL ].dir_size 		= NORMAL_SIZE	  	 - ( NORMAL_SIZE * 0.6 );
	g_dir[ NORMAL_EVENT ].dir_size  = NORMAL_EVENT_SIZE	 - ( NORMAL_EVENT_SIZE * 0.6 );
	g_dir[ PARKING ].dir_size 		= PARKING_SIZE 		 - ( PARKING_SIZE * 0.6 );
	g_dir[ PARKING_EVENT ].dir_size = PARKING_EVENT_SIZE - ( PARKING_EVENT_SIZE * 0.6 );
	g_dir[ HANDWORK ].dir_size 		= HANDWORK_SIZE 	 - ( HANDWORK_SIZE * 0.6 );
	printf("\n");
#endif

	for( i = 0 ; i < dir_cnt ; i++ )
	{
		g_dir[ i ].check_portion = g_dir[ i ].dir_size - ( g_dir[ i ].dir_size  / 4 );

		printf("%-35s Check Full Status [-5%] %luM\t After Remove Size %luM\n",dirs[ i ], g_dir[ i ].dir_size/1024 , g_dir[ i ].check_portion/1024 );
	}

	
	printf("\n");
}

int main( int argc, char *argv[] )
{
	char **buf;
	char *dirs[NAME_SIZE];
	int dir_cnt;
	//
	char **config_buf;
	char *config_line[ CONFIG_LINE ];
	int cnt = 0;
	//
	char **sysfs_buf; 
	char *sysfs_line;

	if( argc < 2 )
	{
		printf("USAGE : ./a.out target_direcotry.txt \n");
		return 0;
	}
		
	dir_cnt = open_files( argv[1], dirs, &buf ); //target_direcotry
	//view_dirs( dirs, dir_cnt );
	cnt = open_files( "/home/odroid/mount/BXFS_CON", config_line, &config_buf ); //read config file  //mnt/ 위치는 수정되어야 함
	//view_dirs( config_line, cnt );
	cnt = open_files( "/sys/fs/OPEL_FAT/control", &sysfs_line, &sysfs_buf ); //FAT Policy
	view_dirs( &sysfs_line, cnt );

	init( dir_cnt, dirs, config_line, sysfs_line );
	printf("dir_cnt : %d \n", dir_cnt );

	detect_and_control( dirs, dir_cnt );		

	return 0;
}







