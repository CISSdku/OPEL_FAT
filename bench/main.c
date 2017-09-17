#include <stdio.h>

char buf[1024];
struct services{
	int limit;
	int max_len;
	int write_len;
	char file_name[100];
	int fd;
	int fsync;
};

long fsize[5] = {1,2,4,8,16};
struct services service[5];
int main(int argc, char* argv[])
{
	int dir;	
	int file_create;
	int target_number = atoi( argv[1] );
	for( i = 0; i < 5; i++){
		service[i].limit =  fsize[i];
		service[i].max_len = 0;
		service[i].write_len = 0;
		strcpy( service[i].filename,  );

	}
	while(file_create < target_number){
		dir = select_dir();// random 0~4
		file_create += write_file( service[dir] );
	}
}

int write_file(struct services* service ) {
	
	if( service->fd == 0 ){
			// New open
		open(ervice->file_name , ... );
		service->max_len = random_size(services->limit)
		// change file name
	}
	
	service->write_len += write( service->fd, buf, 1024;);

	if( service->max_len <= service->write_len ){
		close( service.fd );
		service.fd = 0;
		return 1;
	}else
		return 0;
}


