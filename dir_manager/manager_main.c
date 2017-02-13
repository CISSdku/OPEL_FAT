#include"manager.h"

/*
   1. 관리할 디렉터리 설정
		a. 기존에는 workload 안에서 위치가 미리 셋팅되고 만들면서 크기를 관리 하였음
		b. manager만 따로 수행하기 때문에 관리 대상이 되는 전체 디렉터리를 체크하고 관리해야 함
   2. 관리 대상 사이즈 확인
*/

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
		printf("total dir num : %d \n", cnt );
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
	return cnt;;
}
static void view_dirs( char **dirs, int dir_cnt )
{
	int i;

	for( i = 0 ; i < dir_cnt ; i++ ) 
		printf("%s\n", *( dirs + i ) );

}

void init( int dir_cnt, char **dirs )
{
	int i = 0;
	
	g_dir = ( dir_info * )malloc( sizeof( dir_info ) * dir_cnt ); 

	memset( ( void *)g_dir, 0x0, sizeof( g_dir ) * dir_cnt );

	//전체 크기에서 1% 뺀 크기를 기준으로 디렉터리가 꽉 찼는지 판단
//	g_dir[ ETC ].dir_size 			= ETC_SIZE; 
	g_dir[ NORMAL ].dir_size 		= NORMAL_SIZE;
	g_dir[ NORMAL_EVENT ].dir_size  = NORMAL_EVENT_SIZE;
	g_dir[ PARKING ].dir_size 		= PARKING_SIZE;
	g_dir[ PARKING_EVENT ].dir_size = PARKING_EVENT_SIZE;
	g_dir[ HANDWORK ].dir_size 		= HANDWORK_SIZE;

	for( i = 0 ; i < dir_cnt ; i++ )
		printf("%luM ", g_dir[ i ].dir_size / 1024  ); // M 단위

//	g_dir[ ETC ].dir_size 			= ETC_SIZE 		     - ( ETC_SIZE * 0.01 );
	g_dir[ NORMAL ].dir_size 		= NORMAL_SIZE	  	 - ( NORMAL_SIZE * 0.02 );
	g_dir[ NORMAL_EVENT ].dir_size  = NORMAL_EVENT_SIZE	 - ( NORMAL_EVENT_SIZE * 0.02 );
	g_dir[ PARKING ].dir_size 		= PARKING_SIZE 		 - ( PARKING_SIZE * 0.02 );
	g_dir[ PARKING_EVENT ].dir_size = PARKING_EVENT_SIZE - ( PARKING_EVENT_SIZE * 0.02 );
	g_dir[ HANDWORK ].dir_size 		= HANDWORK_SIZE 	 - ( HANDWORK_SIZE * 0.02 );

	printf("\n");

	for( i = 0 ; i < dir_cnt ; i++ )
	{
		g_dir[ i ].check_portion = g_dir[ i ].dir_size - ( g_dir[ i ].dir_size  / 4 );

		printf("%-35s Check Full Status [-2%] %luM\t After Remove Size %luM\n",dirs[ i ], g_dir[ i ].dir_size/1024 , g_dir[ i ].check_portion/1024 );
	}

	
	printf("\n");
}

int main( int argc, char *argv[] )
{
	char **buf;
	char *dirs[NAME_SIZE];
	int dir_cnt;

	if( argc < 2 )
	{
		printf("USAGE : ./a.out target_direcotry.txt \n");
		return 0;
	}

	dir_cnt = open_files( argv[1], dirs, &buf );
	//view_dirs( dirs, dir_cnt );
	init( dir_cnt, dirs );
	//printf("%d \n", dir_cnt );

	detect_and_control( dirs, dir_cnt );		

	return 0;
}







