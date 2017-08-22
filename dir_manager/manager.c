#include"manager.h"

#define STN_SIZE 100

static unsigned long dir_size( char *dn)
{
	FILE *fp = NULL;
	char stn[ STN_SIZE ];
//	char *ch[2]  = {  "du -scb ", "* | grep 합계" };
//	char *ch[2]  = {  "ls --size ", " | grep total" }; //M단위로 나옴
	char *ch[2]  = {  "ls --size ", " | grep 합계" };

	char buf[50];
	char *size = NULL;
	unsigned long l_size = 0;

	//printf("%s \t", dn );
	sprintf( stn,  "%s%s%s",ch[0],dn,ch[1] );

//	printf("\ntest : %s \n ", stn);
	
		
	fp = popen(stn,"r");
	if(fp == NULL) {
		printf("Error opening : %s\n", strerror( errno ));
		printf("stn : %s \n", stn );

		exit(-1);
	}

	fgets( buf,sizeof(buf),fp);

	strtok(buf," ");
	size = strtok( NULL, " ");

	pclose( fp );

//	while(1);
	l_size = strtoul( size, NULL, 10 );

//	printf("dir_size : %luM \n", l_size / 1024  );

	return l_size;
}

static void file_old_remove( char *dn, int selected_dir )
{
	DIR *dir;
	struct dirent *de;
	struct stat st, temp_st;
	char fn[ 100 ], old_name[ 100 ];
	int fs_err=0;
	unsigned long old_time = ULONG_MAX;
	unsigned long old_inum = ULONG_MAX;
	long decimal_point = LONG_MAX;

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
				old_inum = st.st_ino;
				decimal_point = st.st_mtim.tv_nsec;
				strcpy(old_name,fn);

				temp_st = st;
			}
			else if( old_time == st.st_mtime )
			{
				if( old_inum > st.st_ino )
				{
					strcpy( old_name, fn );	
				}
#if 0
				if( decimal_point >=  st.st_mtim.tv_nsec )
				{
					decimal_point = st.st_mtim.tv_nsec;
					strcpy(old_name,fn);
					temp_st = st;
				}
#endif
			}
			else;
		} 
	}

	remove(old_name);
	closedir( dir );
	//printf("Old file remove : %s \t  %lu \t  %lu \t %lu M \t %d\n",old_name, temp_st.st_size /1024, dir_size( dn ) /1024, g_dir[ selected_dir ].check_portion/1024, g_dir[ selected_dir ].full_count );
//	printf("Old file remove %15s\tmtime %12lu\tnsec %12lu\tdir_size %8lu\tcheck_portion %lu full_count %5d \n",old_name, temp_st.st_mtime, temp_st.st_mtim.tv_nsec, dir_size( dn ) /1024, g_dir[ selected_dir ].check_portion/1024, g_dir[ selected_dir ].full_count );
	printf("Old file remove %15s\tdir_size %8lu\tcheck_portion %lu full_count %5d \n",old_name, dir_size( dn ) /1024, g_dir[ selected_dir ].check_portion/1024, g_dir[ selected_dir ].full_count );
}

void detect_and_control( char **dirs, int dir_cnt )
{
	int num = 0;
	unsigned long size = 0;

#if 1

	while(1)
	{

		while( ( size = dir_size( dirs[ num ] ) ) > g_dir[ num ].dir_size ) // 꽉찬 경우
		{

			printf("Full---------------------------------------------------------------------------------------------- \n");
			printf("%s	%lu \n", dirs[ num ], size/1024 );

			g_dir[ num ].full_count++;
//			printf("full_count : %d \n", g_dir[ num ].full_count );

			//꽉 차면 전체에서 일정 비율을 삭제할 거임
			while( dir_size( dirs[ num ] )  > g_dir[ num ].check_portion ) 
			{
				printf("DELETE\n");	
				
				file_old_remove( dirs[ num ], num );
			}

//			printf("check_portion : %lu \n", g_dir[ num ].check_portion / 1024 );
	
//			while(1);
		}
	
		num++;
		if( num > ( dir_cnt - 1 ) )
			num = 0;

//		usleep(100000);
	//	sleep(1);
	}
	
#endif
}









