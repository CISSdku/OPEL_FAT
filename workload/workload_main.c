#include"workload.h"

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

static void save_load_track_to_file( FILE **fpp, int sinario )
{
	switch( sinario )
	{
		case S_NORMAL_DRIVING : *fpp = fopen( SAVE_LOG_NORMAL,  "a+" );           break;
		case S_PARKING        : *fpp = fopen( SAVE_LOG_PARKING, "a+" );           break;
		case S_PARKING_SHOCK  : *fpp = fopen( SAVE_LOG_PARKING_SHOCK, "a+" );     break;
		case S_SHOCK          : *fpp = fopen( SAVE_LOG_SHOCK,    "a+" );          break;
		case S_SESSION_3      : *fpp = fopen( SAVE_LOG_SESSION3, "a+" );          break;
		case S_SESSION_4      : *fpp = fopen( SAVE_LOG_SESSION4, "a+" );          break;
		case S_AUTOMATION     : *fpp = fopen( SAVE_LOG_AUTOMATION, "a+" );        break;

		default : printf("save_track_to_file \n");                              break;
	}
}

static void init( int sinario, int *selected_dir, char *argv2, int *mode_flag  )
{
	int i = 0;
	
	if( !strcmp( argv2, "original" ) )
		*mode_flag = ORIGINAL;
	else if( !strcmp( argv2, "opel" ) )//opel //opel 테스트에서는 original에서 적용했던 동일한 파일 들을 적용시키기 위해서 저장했던 로그를 가져오게한다.
	{
		*mode_flag = OPEL;
		save_load_track_to_file( &g_fp, sinario );
	}
	else
	{
		printf("mode error \n");
		exit(-1);
	}

	switch( sinario )
	{
		//AUTOMATION의 의미는 상시, 상시이벤트, 주차, 주차이벤트를 랜덤으로 섞어서 파일을 만들어 내는거임
		//AUTOMATION을 제외한 나머지 실험은 아직 제대로 정의 안되어 있음 
		//ETC에서 다른걸로 수정해야 함
		case S_NORMAL_DRIVING : printf("sinario : NORMAL_DRIVING \n");    *selected_dir = ETC;      break;
		case S_PARKING        : printf("sinario : PARKING \n");           *selected_dir = ETC;      break;
		case S_PARKING_SHOCK  : printf("sinario : PARKING_SHOCK \n");     *selected_dir = ETC;      break;
		case S_SHOCK          : printf("sinario : SHOCK \n");             *selected_dir = ETC;      break;
		case S_SESSION_3      : printf("sinario : SESSION_3 \n");         *selected_dir = ETC;      break;
		case S_SESSION_4      : printf("sinario : SESSION_4 \n");         *selected_dir = ETC;      break;

		case S_AUTOMATION     : printf("sinario : AUTOMATION \n");        *selected_dir = S_AUTOMATION; break;

		default             : printf("sinario is ?? \n");                              break;
	}

	printf("\n");
}

static void auto_select( int *selected_dir )
{
	static int before_num = 0;
	int invoked;

retry:
	srand( time( NULL ) );
	invoked = ( rand( ) % 8 ) ; // 4 : 2 : 1 : 1


	if( 0 <= invoked && invoked <= 3 )
		*selected_dir = NORMAL;
	else if( 4 <= invoked && invoked <= 5 )
	{   
		//주차 다음에 바로 상시 이벤트가 올 수 없다는 가정
		if( before_num == PARKING || before_num == PARKING_EVENT )
			goto retry;

		*selected_dir = NORMAL_EVENT;
	}
	else if( invoked == 6 )
		*selected_dir = PARKING;
	else if( invoked == 7 )
	{
		//상시 다음에 바로 주차 이벤트가 올 수 없다.
		if( before_num == NORMAL || before_num == NORMAL_EVENT )
			goto retry;

		*selected_dir = PARKING_EVENT;
	}
	else; 


	before_num = invoked;
}   

int main( int argc, char *argv[] )
{
	char **buf;
	char *dirs[NAME_SIZE];
	int dir_cnt,
		sinario,
		selected_dir,
		mode_flag;

	if( argc < 3 )
	{
		//일단 로그 파일 받은거 기반으로
		printf("USAGE : ./a.out sinario_num (original or opel) writen_target_dir.txt \n");

		printf("sinario 1 : normal driving \n");
		printf("sinario 2 : parking \n");
		printf("sinario 3 : parking shock\n");
		printf("sinario 4 : shock \n");
		printf("sinario 5 : session 3 \n");
		printf("sinario 6 : session 4 \n");
		printf("sinario 7 : automation \n");

		//printf("Select sinario_num flag_mode( origina || opel ) :");

		return 0;
	}
	sinario = atoi( argv[1] );

	dir_cnt = open_files( argv[3], dirs, &buf );
	view_dirs( dirs, dir_cnt );
	init( sinario, &selected_dir, argv[2], &mode_flag );

	//printf("mode_flag : %d \n", mode_flag );
	
	while(1)
	{
	//	sleep(1);
		usleep(10000);

		if( sinario != S_AUTOMATION )
			file_create( dirs, &selected_dir, sinario, mode_flag );
		else //AUTOMATION
		{
			if(  mode_flag == ORIGINAL )
			{
				auto_select( &selected_dir );
		
				//printf("selected_dir : %d \n", selected_dir );

				file_create( dirs, &selected_dir, sinario, mode_flag );
			}
			else
			{
				file_create( dirs, &selected_dir, sinario, mode_flag ); // dn이 log에서 읽어온거에 따라 바껴야 함
			}
		}
	}

	return 0;
}

