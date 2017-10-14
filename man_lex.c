#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<string.h>

#define MAX_LEN 100
int charClass;  // 用来标记当前字符的类型
char lexeme[MAX_LEN];  // 符号表
char nextChar;
char next2Char;
int lexLen;
int token;
int nextToken;
FILE* inFile;

#define LETTER 0
#define DIGIT 1
#define UNKNOWN 999

enum{ABSTRACT = 258, CASE, CATCH, CLASS, DEF, DO, ELSE, EXTENDS, False, FINAL, FINALLY, FOR, FORSOME, IF, IMPLICIT, IMPORT,
	LAZY, MACRO, MATCH, NEW, Null, OBJECT, OVERRIDE, PACKAGE, PRIVATE, PROTECTED, RETURN, SEALED, SUPER, THIS, THROW, TRAIT,
	TRY, True, TYPE, VAL, VAR, WHILE, WITH, YIELD, UNDERLINE, COLON, EQU, EQUGRT, LESMINUS, LESCOLON, LESPERCENT, GRTCOLON,
	HASH, AT, GEQU, LEQU, ERROR, ID, INT, STR};

char* keywords[] = {"abstrack", "case", "catch", "class", "def", "do", "else","extends", "false", "final", "finally", "for",
					"forSome", "if", "implicit", "import", "lazy", "macro", "new", "null", "object", "override", "package", 
					"private", "protected", "return", "return", "sealed", "super", "this", "throw", "trait", "try", "true",
					"type", "val", "var", "while", "with", "yield", 0};

void addChar();   // 将全局变量nextChar加入到全局变量lexeme中
void getChar();  // nextChar 读取下一个字符
void getNonBlank();
int checkSymbol(char ch, char nextCh);
void checkKeywords(char* pword);
int lexer();
void checkNotes();

//将标识符加入符号表
void addChar(){
	if(lexLen <= MAX_LEN - 2){
		lexeme[lexLen++] = nextChar;
		lexeme[lexLen] = 0;
	}
	else
		printf("ERROR:lexeme is too long.\n");
}

void getChar(){
	static int firstRun = 1;  // 判断是否为第一次读取，第一次读取两个字符
	if(firstRun){
		nextChar = getc(inFile);
		next2Char = getc(inFile);
		firstRun = 0;
	}
	else{
		nextChar = next2Char;
		next2Char = getc(inFile);
	}

	// 判断标识符类型，并修改charClass	
	if(nextChar == EOF){
		charClass = EOF;
	}
	else{
		if(isalpha(nextChar))
			charClass = LETTER;
		else if(isdigit(nextChar))
			charClass = DIGIT;
		else
			charClass = UNKNOWN;
	}
}

// 跳过空白
void getNonBlank(){
	while(isspace(nextChar))
		getChar();
}

// 跳过注释
void checkNotes(){
	if(nextChar == '/' && next2Char == '/'){
		do{
			getChar();
		}while(nextChar != '\n');
		getNonBlank();
	}
	else if(nextChar == '/' && next2Char == '*'){
		getChar();
		getChar();
		while(nextChar != '*' || next2Char != '/'){
			getChar();
		}
		getChar();
		getChar();
		getNonBlank();
	}
}

int checkSymbol(char ch, char nextCh){
	switch(ch){
		case '(': case ')': case ';': case '+': case '-': case '{': case '}': case '\'': 
		case '"': case '[': case ']': case '|': case '*': case '.': case '/': 
			addChar();
			nextToken = ch;
			break;
			
		case '=':
			addChar();
			nextToken = ch;
			if(nextCh == '='){
				getChar();
				addChar();
				nextToken = EQU;
			}
			else if(nextCh == '>'){
				getChar();
				addChar();
				nextToken = EQUGRT;
			}
			break;
			
		case '>':
			addChar();
			nextToken = ch;
			if(nextCh == '='){
				getChar();
				addChar();
				nextToken = GEQU;
			}
			else if(nextCh == ':'){
				getChar();
				addChar();
				nextToken = GRTCOLON;
			}
			break;
			
		case '<':
			addChar();
			nextToken = ch;
			if(nextCh == '='){
				getChar();
				addChar();
				nextToken = LEQU;
			}
			else if(nextCh == '-'){
				getChar();
				addChar();
				nextToken = LESMINUS;
			}
			else if(nextCh == ':'){
				getChar();
				addChar();
				nextToken = LESCOLON;
			}
			else if(nextCh = '%'){
				getChar();
				addChar();
				nextToken = LESPERCENT;
			}
			break;

		case ':':
			addChar();
			nextToken = COLON;
			break;
		
		case '_':
			addChar();
			nextToken = UNDERLINE;

		case '#':
			addChar();
			nextToken = HASH;
			break;
		
		case '@':
			addChar();
			nextToken = AT;
			break;
		
		case EOF:
			addChar();
			nextToken = EOF;
			break;
			
		default:
			printf("ERROR:unknown charater '%c'.\n", ch);
			nextToken = ERROR;
	}
	return nextToken;
}

// 检查是否为关键字
void checkKeywords(char* pword){
	int i = 0;
	while(keywords[i] != 0){
		char* pkeyword = keywords[i];
		if(strcmp(pword, pkeyword) == 0){
			nextToken = 258 + i;
			return;
		}
		i++;
	}
}

int lexer(){
	lexLen = 0;
	getNonBlank();
	checkNotes();
	switch(charClass){
		case LETTER:
			addChar();
			getChar();
			while(charClass == LETTER || charClass == DIGIT){
				addChar();
				getChar();
			}
			nextToken = ID;
			//检查当前标识符是否为关键字
			checkKeywords(lexeme);
			break;
		
		case DIGIT:
			addChar();
			getChar();
			while(charClass == DIGIT){
				addChar();
				getChar();
			}
			nextToken = INT;
			break;
		
		case UNKNOWN:
			checkSymbol(nextChar, next2Char);
			getChar();
			break;
		
		case EOF:
			nextToken = EOF;
			lexeme[0] = 'E';
			lexeme[1] = 'O';
			lexeme[2] = 'F';
			lexeme[3] = 0;
			break;
	}
	
	printf("<%6d,   %s   >\n", nextToken, lexeme);
	return nextToken;
}

void main(int argc, char* argv[]){
	if(argc<2){
		printf("ERROR:input file name is needed.\n");
		exit(0);
	}
	inFile = fopen(argv[1], "r");
	if(inFile == NULL){
		printf("ERROR:can not open file.\n");
		exit(0);
	}
	
	getChar();
	while(nextToken != EOF){
		lexer();
	}
	return;
}
