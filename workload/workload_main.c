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

static void load_saved_log_file( FILE **fpp, int sinario )
{
	switch( sinario )
	{
		case S_NORMAL_DRIVING 		: *fpp = fopen( SAVE_LOG_NORMAL, 	    "a+" );     break;
		case S_NORMAL_DRIVING_SHOCK : *fpp = fopen( SAVE_LOG_NORMAL_SHOCK,  "a+" );     break;
		case S_PARKING        		: *fpp = fopen( SAVE_LOG_PARKING,       "a+" );     break;
		case S_PARKING_SHOCK 		: *fpp = fopen( SAVE_LOG_PARKING_SHOCK, "a+" );     break;
		case S_HANDWORK        	    : *fpp = fopen( SAVE_LOG_HANDWORK,      "a+" );     break;

		case S_AUTOMATION    	    : *fpp = fopen( SAVE_LOG_AUTOMATION,    "a+" );     break;

		default : printf("save_track_to_file \n");                         		        break;
	}
}

static void init( int sinario, int *selected_dir, char *argv3 , int *load_flag )
{
	if( !strcmp( argv3, "on" ) || !strcmp( argv3, "On" ) || !strcmp( argv3, "ON" ))
	{
		*load_flag = ON;
		load_saved_log_file( &g_fp, sinario );

		printf("Loading saved log file");
	}
	else 
		*load_flag = OFF;

	switch( sinario )
	{
		//AUTOMATION의 의미는 상시, 상시이벤트, 주차, 주차이벤트를 랜덤으로 섞어서 파일을 만들어 내는거임
		//AUTOMATION을 제외한 나머지 실험은 아직 제대로 정의 안되어 있음 
		case S_NORMAL_DRIVING 		: printf("sinario : NORMAL_DRIVING \n");  		  *selected_dir = S_NORMAL_DRIVING; 	       break;
		case S_NORMAL_DRIVING_SHOCK : printf("sinario : NORMAL_DRIVING_SHOCK \n");    *selected_dir = S_NORMAL_DRIVING_SHOCK;      break;
		case S_PARKING     		    : printf("sinario : PARKING \n");           	  *selected_dir = S_PARKING;    	     	   break;
		case S_PARKING_SHOCK  		: printf("sinario : PARKING_SHOCK \n");     	  *selected_dir = S_PARKING_SHOCK;     		   break;
		case S_HANDWORK        	    : printf("sinario : SHOCK \n");             	  *selected_dir = S_HANDWORK;      	 		   break;

		case S_AUTOMATION    	    : printf("sinario : AUTOMATION \n");        	  *selected_dir = S_AUTOMATION;		 	 	   break;

		default           	  	 	: printf("sinario is ?? \n");                          		 	    					 	 	   break;
	}

//	g_time.start_point = clock();
	g_line_to_read = atoi( argv3 + strlen( argv3 ) + 1 );
	printf("%d \n", g_line_to_read );


	//////
	g_total.file_counter = 0;

	printf("\n");
}

static void auto_select( int *selected_dir )
{
	static int before_num = 0;
	int invoked;

//retry:
	invoked = ( rand( ) % 8 ) ; // 4 : 2 : 1 : 1

	//현재 auto_select는 normal, normal_event, parking, parking_event 
	//4가지 경우를 위의 비율로 파일을 생성시킴


	if( 0 <= invoked && invoked <= 1 )
		*selected_dir = NORMAL;
	else if( 2 <= invoked && invoked <= 3 )
	{   
		//주차 다음에 바로 상시 이벤트가 올 수 없다는 가정
//		if( before_num == PARKING || before_num == PARKING_EVENT )
//			goto retry;

		*selected_dir = NORMAL_EVENT;
	}
	else if( 4 <= invoked && invoked <= 5 )
		*selected_dir = PARKING;
	else if( 6 <= invoked && invoked <= 7 )
	{
		//상시 다음에 바로 주차 이벤트가 올 수 없다.
//		if( before_num == NORMAL || before_num == NORMAL_EVENT )
//			goto retry;

		*selected_dir = PARKING_EVENT;
	}
	else; 


	before_num = invoked;
}   

void run_workload( char **dirs, int sinario, int selected_dir, int load_flag )
{
	if( sinario != S_AUTOMATION )
	{
		file_create( dirs, &selected_dir, sinario, load_flag );
	}		
	else // AUTOMATION
	{
		auto_select( &selected_dir );
		file_create( dirs, &selected_dir, sinario, load_flag );
	}
}

int main( int argc, char *argv[] )
{
	char **buf;
	char *dirs[NAME_SIZE];
	int dir_cnt,
		sinario,
		selected_dir,
		load_flag, // load  flag on 되면 
		line_to_read;

	srand( time( NULL ) );
	if( argc < 4 )
	{
		//printf("%d \n", argc );
		
		printf("USAGE : ./a.out (sinario_num) (target_list dir) (load_flag) (line_to_read)  \n");

		printf("sinario 1 : normal driving \n");
		printf("sinario 2 : normal driving shock \n");
		printf("sinario 3 : parking \n");
		printf("sinario 4 : parking shock\n");
		printf("sinario 5 : handwork\n");
		printf("sinario 6 : automation \n");
		//printf("Select sinario_num flag_mode( origina || opel ) :");

		return 0;
	}
	sinario = atoi( argv[1] );
	dir_cnt = open_files( argv[2], dirs, &buf );
	view_dirs( dirs, dir_cnt );
	init( sinario, &selected_dir, argv[3], &load_flag );


//	printf("test\n");
	while(1)
	{
		if( load_flag == ON )
			sleep(10);

//		usleep(10000);
		
		run_workload( dirs, sinario, selected_dir, load_flag );
	}

	return 0;
}

