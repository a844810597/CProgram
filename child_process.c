#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<sys/stat.h>
#include<dirent.h>
#include<malloc.h>
#include<string.h>
#include<pwd.h>
#include<time.h>
#include<grp.h>
//#include<errno.h>

void myls(char* work_path);
void print_type(struct stat* buf);  //打印文件类型
void print_perm(struct stat* buf);  //打印文件权限
void print_link(struct stat* buf);
void print_usrname(uid_t uid);
void print_grname(gid_t gid);
void print_time(time_t* mtime);
void print_filename(char* d_name);

void print_type(struct stat* buf){
	char* ptr;
	if(S_ISREG(buf->st_mode)) ptr = "-";
	else if(S_ISDIR(buf->st_mode)) ptr ="d";
	else if(S_ISCHR(buf->st_mode)) ptr = "c";
	else if(S_ISBLK(buf->st_mode)) ptr = "b";
	else if(S_ISFIFO(buf->st_mode)) ptr = "p";
	else if(S_ISLNK(buf->st_mode)) ptr = "l";
	else if(S_ISSOCK(buf->st_mode)) ptr = "s";
	printf("%s", ptr);
}

void print_perm(struct stat* buf){
	char usr_r = buf->st_mode & S_IRUSR ? 'r' : '-';
	char usr_w = buf->st_mode & S_IWUSR ? 'w' : '-';
	char usr_x = buf->st_mode & S_IXUSR ? 'x' : '-';
	char grp_r = buf->st_mode & S_IRGRP ? 'r' : '-';
	char grp_w = buf->st_mode & S_IWGRP ? 'w' : '-';
	char grp_x = buf->st_mode & S_IXGRP ? 'x' : '-';
	char oth_r = buf->st_mode & S_IROTH ? 'r' : '-';
	char oth_w = buf->st_mode & S_IWOTH ? 'w' : '-';
	char oth_x = buf->st_mode & S_IXOTH ? 'x' : '-';
	printf("%c%c%c%c%c%c%c%c%c ", usr_r, usr_w, usr_x, grp_r, grp_w, grp_x, oth_r, oth_w, oth_x);
}

void print_link(struct stat* buf){
	printf("%d ", (int)buf->st_nlink);
}

void print_usrname(uid_t uid){
	struct passwd* pw_ptr;
	pw_ptr = getpwuid(uid);
	printf("%s ", pw_ptr->pw_name);
}

void print_grname(gid_t gid){
	struct group *grp_ptr;
	grp_ptr = getgrgid(gid);
	printf("%s ", grp_ptr->gr_name);
}

void print_time(time_t* mtime){
	mtime = ctime(mtime);
	char temp[50] = {'\0'};
	strncpy(temp, mtime, strlen(mtime)-1);
	printf("%s ", temp);
}

void print_filename(char* d_name){
	printf("%s", d_name);
}

void myls(char* work_path){
	mkdir("/home/wujie", 0777);  // 在/home目录下创建以自己名字拼音为名的文件夹
	//printf("%s", errno);
	struct stat buf;
	puts(work_path);
	DIR* work_dir = opendir(work_path);
	if(work_dir == NULL){
		printf("open directory fail\n");
		return;
	}
	struct dirent* currentdp = readdir(work_dir);
	while(currentdp != NULL){
		// ls -l
		char temp_work_path[100] = {'\0'};
		strncpy(temp_work_path, work_path, strlen(work_path));
		char* each_file = strcat(temp_work_path, currentdp->d_name);
		if(lstat(each_file, &buf) == -1){
			puts("fail to lstat\n");
		}
		print_type(&buf);
		print_perm(&buf);
		print_link(&buf);
		print_usrname(buf.st_uid);
		print_grname(buf.st_gid);
		print_time(&buf.st_mtim);
		print_filename(currentdp->d_name);
		printf("\n");


		if((strcmp(currentdp->d_name,".") == 0) || (strcmp(currentdp->d_name,"..") == 0)){
			currentdp = readdir(work_dir);
			continue;
		}
			
		
		// cp -ram/mycopy/mycopy
		char path[128] = {'\0'};
		strcpy(path, work_path);
		strncat(path, currentdp->d_name, sizeof(currentdp->d_name));
		pid_t pid = fork();
		if(pid == -1)
			printf("Fork Error!\n");
		else if(pid == 0){
			execl("/home/jhye/Documents/Cprogram/mycopy/mycopy.o","/home/jhye/Documents/Cprogram/mycopy/mycopy.o", path ,"/home/wujie",NULL);
		}
		else if(wait(NULL) != pid){
			printf("A signal must have interrupted the wait!\n");
		}
		else{
			currentdp = readdir(work_dir);
		}
	}
}

int main(void){
	char* buf = getcwd(NULL, 0);
	buf = strcat(buf, "/");
	myls(buf);
	return 0;
}
