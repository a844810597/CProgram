#include<stdio.h>
#include<sys/stat.h>
#include<dirent.h>
#include<string.h>

void copyFile(char* source, char* dest);  // 复制文件
char* getFileName(char* source);  // 获得文件名
void mycopy(char* source, char* dest);  // 拷贝文件
int isDir(char* filename);  // 判断是否为目录文件：判断失败，返回-1；是目录文件返回1；非目录文件返回0
char* addSlash(char* filename);  // 给路径末尾追加斜线
int isEndWithSlash(char* filename);  // 判断路径末尾是否有斜杠
char* strConcat(char* dest, char* src);  // 拼接字符串
char* fullPathCat(char* path, char* filename);  // 拼接为绝对路径

// 复制文件
void copyFile(char* source, char* dest){
	FILE* fpSource = fopen(source, "r");
	FILE* fpDest = fopen(dest, "w");
	int ch = 0;
	while((ch = fgetc(fpSource)) != EOF){
		fputc(ch, fpDest);
	}
	fclose(fpSource);
	fclose(fpDest);
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
	static char result[100] = {'\0'};
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
	char* result = (char*)malloc(50);
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

void mycopy(char src[], char dest[]){
	// dest file must be a directory
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
				printf("跳过了文件: %s\n", currentdp->d_name);
				currentdp = readdir(srcOpen);
				continue;
			}
			
			// 如果待复制的子文件是目录文件，则递归复制
			if(isDir(childSrcFile)){
				char* childDestDir = newDestDir;
				mycopy(childSrcFile, childDestDir);
			}
			
			// 如果待复制的子文件非目录文件，则复制到刚刚创建的目录下
			else{
				char* childDestFile = fullPathCat(newDestDir, currentdp->d_name);
				puts(childSrcFile);
				puts(childDestFile);
				copyFile(childSrcFile, childDestFile);
			}

			currentdp = readdir(srcOpen);
		}
		closedir(srcOpen);
	}
}


int main(int argc, char* argv[]){
	mycopy(argv[1], argv[2]);
	return 0;
}