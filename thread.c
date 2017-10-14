#include<stdio.h>
#include<sys/stat.h>
#include<unistd.h>
#include<dirent.h>
#include<malloc.h>
#include<string.h>
#include<pwd.h>
#include<time.h>
#include<grp.h>
#include<stdlib.h>

// pthread_create传入的两个参数
struct cppara
{
    char* para1;//参数1
    char* para2;//参数2
};

// ls -l
void* myls(void* lspath);
void print_type(struct stat* buf);  //打印文件类型
void print_perm(struct stat* buf);  //打印文件权限
void print_link(struct stat* buf);
void print_usrname(uid_t uid);
void print_grname(gid_t gid);
void print_time(time_t* mtime);
void print_filename(char* d_name);

// cp -r
void copyFile(char* source, char* dest);  // 复制文件
char* getFileName(char* source);  // 获得文件名
void* mycopy(void* para);  // 拷贝文件
int isDir(char* filename);  // 判断是否为目录文件：判断失败，返回-1；是目录文件返回1；非目录文件返回0
char* addSlash(char* filename);  // 给路径末尾追加斜线
int isEndWithSlash(char* filename);  // 判断路径末尾是否有斜杠
char* strConcat(char* dest, char* src);  // 拼接字符串
char* fullPathCat(char* path, char* filename);  // 拼接为绝对路径


// ls -l
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

void* myls(void* lspath){
	char* work_path = (char*)lspath;
	mkdir("/home/jhye/wujie", 0777);  // 在/home目录下创建以自己名字拼音为名的文件夹

	struct stat buf;
	puts(work_path);
	DIR* work_dir = opendir(work_path);
	if(work_dir == NULL){
		printf("open directory fail\n");
		return;
	}
	struct dirent* currentdp = readdir(work_dir);
	while(currentdp != NULL){
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

		// 如果文件是 . 或者 .. 则跳过
		if((strcmp(currentdp->d_name,".") == 0) || (strcmp(currentdp->d_name,"..") == 0)){
			currentdp = readdir(work_dir);
			continue;
		}

		//设置执行mycp的参数cpsrc, cpdest
		char cpsrc[128] = {'\0'};
		strcpy(cpsrc, work_path);
		strncat(cpsrc, currentdp->d_name, sizeof(currentdp->d_name));

		char* cpdest = "/home/jhye/wujie";

		struct cppara para = {cpsrc, cpdest};
		
		// 创建mycp的线程
		pthread_t tid;
		pthread_create(&tid, NULL, (void *)mycopy, (void*)&para);
		//等待mycp的结束
		pthread_join(tid, NULL);
		
		//扫描下一个子文件
		currentdp = readdir(work_dir);
	}
}

// cp -r
// 复制文件
void copyFile(char *source,char *dest){
    FILE *source_fp,*dest_fp;
	int ch;

    //Open
    if((source_fp = fopen(source,"rb")) == NULL){
        fprintf(stderr,"ERROR: can't open %s.\n",source);
        exit(EXIT_FAILURE);     
	}

    if((dest_fp = fopen(dest,"wb")) == NULL){
    	fprintf(stderr,"ERROR: can't create %s.\n",dest);
        exit(EXIT_FAILURE);    
    }
    //Copy       
    while((ch = getc(source_fp)) != EOF){
        putc(ch,dest_fp);
    }
    
    //Close
    fclose(source_fp);
    fclose(dest_fp);
}

// 获得文件名
char* getFileName(char source[]) {
	// 不能直接返回局部变量
	// strrchr函数返回从字符串最后开始，第一次与第二个参数字符串匹配的位置
	return (char*)(strrchr(source,'/')+1);
}

// 判断文件是否为目录文件
int isDir(char* filename){
	struct stat buf;
	if(lstat(filename, &buf) == -1){
		printf("fail to lstat: %s.\n", filename);
		return -1;  // 判断失败，返回-1
	}

	if(S_ISDIR(buf.st_mode)){
		return 1;  // 如果是目录文件，则返回1
	}
	else{
		return 0;  // 否则返回0
	}
}

// 在文件末尾添加斜线
char* addSlash(char filename[]){
	char* result = (char*)malloc(256*sizeof(char));
	strcpy(result, filename);
	strcat(result, "/");
	return result;
}

// 判断文件是否以'/'结尾
int isEndWithSlash(char filename[]){
	int len = strlen(filename);
	if(filename[len - 1] == '/')
		return 1;
	else
		return 0;
}

// 拼接字符串
char* strConcat(char* dest, char* src){
	// 在堆中分配内存，返回字符串；不能直接返回局部变量。
	char* result = (char*)malloc(256*sizeof(char));
	strcpy(result, dest);
	strcat(result, src);
	return result;
}

// 拼接为绝对路径
char* fullPathCat(char* path, char* filename){
	if(!isEndWithSlash(path))
		path = addSlash(path);
	
	return strConcat(path, filename);
}

void* mycopy(void* para){
	// dest file must be a directory
	char* src = ((struct cppara*)para)->para1;
	char* dest = ((struct cppara*)para)->para2;
	if(isDir(dest) != 1){
		printf("ERROR: '%s' must be a directory.\n", getFileName(dest));
		return;
	}


	// 如果src是非目录文件，则拷贝到目标目录下
	if(isDir(src) == 0){
		char* newFile = fullPathCat(dest, getFileName(src));
		copyFile(src, newFile);
	}
	
	// 如果src是目录文件，则以拷贝目录的方式进行拷贝
	else if(isDir(src) == 1){
		// 获得待拷贝的目录名
		char* dirName = getFileName(src);
		// 在dest路径下，新建一个与src同名的dir
		char* newDestDir = fullPathCat(dest, dirName);
		mkdir(newDestDir, 0777);
		// 打开源目录
		DIR* srcOpen = opendir(src);
		if(srcOpen == NULL){
			printf("open directory fail.\n");
			return;
		}
		
		// 遍历每个srcdir中的每个文件
		struct dirent* currentdp = readdir(srcOpen);
		
		while(currentdp != NULL){
			// 生成待复制的子文件的绝对路径
			char* childSrcFile = fullPathCat(src, currentdp->d_name);
			// 如果为'.' 或 '..'，则不复制
			if(currentdp->d_name[0] == '.'){
				currentdp = readdir(srcOpen);
				continue;
			}
			
			// 如果待复制的子文件是目录文件，则递归复制
			if(isDir(childSrcFile)){
				char* childDestDir = newDestDir;
				struct cppara temp = {childSrcFile, childDestDir};
				mycopy((void*)&temp);
			}
			// 如果待复制的子文件非目录文件，则复制到刚刚创建的目录下
			else{
				char* childDestFile = fullPathCat(newDestDir, currentdp->d_name);
				copyFile(childSrcFile, childDestFile);
			}

			currentdp = readdir(srcOpen);
		}
		closedir(srcOpen);
	}
	
}

int main(){
	pthread_t tid;

	// ls 获取工作目录
	char* buf = getcwd(NULL, 0);
	buf = strcat(buf, "/");
	myls(buf);

	pthread_create(&tid, NULL, (void*)myls, &buf);
	
}