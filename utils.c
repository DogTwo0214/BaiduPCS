﻿#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#ifdef WIN32
#  include <conio.h>
#  include <direct.h>
#else
#  include <inttypes.h>
#  include <termios.h>
#endif

#include "pcs/pcs_mem.h"
#include "utils.h"


#if defined(WIN32)
# define snprintf _snprintf
#endif


static inline char *i_strdup(const char *str)
{
	char *res = 0;
	if (str) {
		res = (char *)pcs_malloc(strlen(str) + 1);
		if (!res)
			return 0;
		strcpy(res, str);
	}
	return res;
}

#pragma region std_string(), std_password(), is_absolute_path() 三个函数

#ifdef WIN32

/*
* 从标准输入中输入字符串
* str  - 输入的字符串将填充到 str 中
* size - 最多输入 size 个字节。
*/
void std_string(char *str, int size)
{
	char c;
	int i = 0;

	while ((c = _getch()) != '\r' && c != '\n') {
		if (c == '\b') {
			if (i > 0) {
				i--;
				putchar(c);
				putchar(' ');
				putchar(c);
			}
		}
		else if (isprint(c)) {
			str[i] = c;
			putchar(c);
			i++;
			if (i >= size) {
				break;
			}
		}
	}
	str[i >= size ? (size - 1) : i] = '\0';
	printf("\n");
}

/*
* 从标准输入中输入密码，输入的字符不回显
* password  - 输入的密码将填充到 password 中
* size      - 最多输入size个字节。
*/
void std_password(char *password, int size)
{
	char c;
	int i = 0;

	while ((c = _getch()) != '\r' && c != '\n') {
		if (c == '\b') {
			if (i > 0)
				i--;
		}
		else if (isprint(c)) {
			password[i] = c;
			//putchar('*');
			i++;
			if (i >= size) {
				break;
			}
		}
	}
	password[i >= size ? (size - 1) : i] = '\0';
	printf("\n");
}

int is_absolute_path(const char *path)
{
	if (!path) return 0;
	if (strlen(path) < 2) return 0;
	if (path[1] != ':') return 0;
	if (!((path[0] > 'a' && path[0] < 'z') || (path[0] > 'A' && path[0] < 'Z'))) return 0;
	return 1;
}

#else

#include <termios.h>
#include <unistd.h>

/*
* 从标准输入中输入字符串
* str  - 输入的字符串将填充到 str 中
* size - 最多输入 size 个字节。
*/
void std_string(char *str, int size)
{
	struct termios oflags, nflags;

	/* disabling echo */
	tcgetattr(fileno(stdin), &oflags);
	nflags = oflags;
	//nflags.c_lflag &= ~ECHO;
	//nflags.c_lflag |= ECHONL;

	if (tcsetattr(fileno(stdin), TCSANOW, &nflags) != 0) {
		perror("tcsetattr");
		return;
	}

	//printf("user name: ");
	fgets(str, size, stdin);
	str[size - 1] = 0;
	str[strlen(str) - 1] = 0;
	//printf("you typed '%s'\n", str);

	/* restore terminal */
	if (tcsetattr(fileno(stdin), TCSANOW, &oflags) != 0) {
		perror("tcsetattr");
		return;
	}
}

/*
* 从标准输入中输入密码，输入的字符不回显
* password  - 输入的密码将填充到 password 中
* size      - 最多输入size个字节。
*/
void std_password(char *password, int size)
{
	struct termios oflags, nflags;

	/* disabling echo */
	tcgetattr(fileno(stdin), &oflags);
	nflags = oflags;
	nflags.c_lflag &= ~ECHO;
	nflags.c_lflag |= ECHONL;

	if (tcsetattr(fileno(stdin), TCSANOW, &nflags) != 0) {
		perror("tcsetattr");
		return;
	}

	//printf("password: ");
	fgets(password, size, stdin);
	password[size - 1] = 0;
	password[strlen(password) - 1] = 0;
	//printf("you typed '%s'\n", password);

	/* restore terminal */
	if (tcsetattr(fileno(stdin), TCSANOW, &oflags) != 0) {
		perror("tcsetattr");
		return;
	}

}

int is_absolute_path(const char *path)
{
	if (!path) return 0;
	if (strlen(path) < 1) return 0;
	if (path[0] != '/' && path[0] != '~') return 0;
	return 1;
}

#endif

#pragma endregion

/** detecting whether base is starts with str
*/
int startsWith(char* base, char* str)
{
	return (strstr(base, str) - base) == 0;
}

/** detecting whether base is ends with str
*/
int endsWith(char* base, char* str)
{
	int blen = strlen(base);
	int slen = strlen(str);
	return (blen >= slen) && (strcmp(base + blen - slen, str) == 0);
}

/*
* 判断两个字符串是否相等。
*  s1    - 以'\0'为结束标记的字符串
*  s2    - 待比较的字符串
*  s2len - s2字符串的字节长度。如果传入-1的话，则使用'\0'作为其结束标记
* 如果相等，则返回1，否则返回0。
*/
int streq(const char *s1, const char *s2, int s2len)
{
	const char *p1 = s1, *p2 = s2;
	int i = 0, rc = 1;
	if (s2len == -1) {
		for (i = 0;; i++) {
			if (!s1[i]) {
				if (!s2[i]){
					break;
				}
				else{
					rc = 0;
					break;
				}
			}
			else if (!s2[i]) {
				rc = 0;
				break;
			}
			else if (s1[i] != s2[i]) {
				rc = 0;
				break;
			}
		}
	}
	else {
		for (i = 0; i < s2len; i++) {
			if (!s1[i] || s1[i] != s2[i]) {
				rc = 0;
				break;
			}
		}
	}
	return rc;
}

/*
* 判断arr数组中是否存在字符串str，如果存在则返回其标号（标号为 [索引] + 1），否则返回0。
* 比较时区分大小写。
* arr  - 存储很多字符串的数组，数组最后一个元素必须为NULL。
* str  - 判断是否存在的字符串
* len  - 字符串长度。 如果传入-1，则'\0'作为其结束标记。
*/
int str_in_array(const char **arr, const char *str, int len)
{
	int i = 0, rc = 0;
	const char *p;
	while ((p = arr[i++])) {
		if (streq(p, str, len)) {
			rc = i;
			break;
		}
	}
	return rc;
}

/*
* 合并路径，如果filename传入的是绝对路径，则直接返回filename的拷贝。
*   base     - 基目录
*   basesz   - base的字节长度，传入-1的话，将使用strlen()函数读取。
*   filename - 文件名字
* 使用完后，需调用pcs_free来释放返回值
*/
char *combin_path(const char *base, int basesz, const char *filename)
{
	char *p, *p2;
	int filenamesz, sz;

	if (strcmp(filename, ".") == 0) {
		p = (char *)pcs_malloc(basesz + 1);
		assert(p);
		memset(p, 0, basesz + 1);
		memcpy(p, base, basesz);
		p[basesz] = '\0';
	}
	else if (strcmp(filename, "..") == 0) {
		p = (char *)pcs_malloc(basesz + 1);
		assert(p);
		memset(p, 0, basesz + 1);
		memcpy(p, base, basesz);
		p[basesz] = '\0';
		basesz--;
		if (p[basesz] == '/' || p[basesz] == '\\') p[basesz] = '\0';
		basesz--;
		while (basesz >= 0 && p[basesz] != '/' && p[basesz] != '\\') {
			basesz--;
		}
		if (basesz < 0) {
			p[0] = '\0';
		}
		else if (basesz == 0) {
			p[1] = '\0';
		}
		else {
			p[basesz] = '\0';
		}
	}
	else if (is_absolute_path(filename) || !base || basesz == 0 || !base[0]) {
		p = i_strdup(filename);
	}
	else {
		if (basesz == -1) basesz = strlen(base);
		filenamesz = strlen(filename);
		sz = basesz + filenamesz + 1;
		p = (char *)pcs_malloc(sz + 1);
		assert(p);
		memset(p, 0, sz + 1);
		memcpy(p, base, basesz);
		p[basesz] = '\0';
		if (filename[0] == '/' || filename[0] == '\\') {
			if (p[basesz - 1] == '/' || p[basesz - 1] == '\\') {
				p[basesz - 1] = '\0';
			}
		}
		else {
			if (p[basesz - 1] != '/' && p[basesz - 1] != '\\') {
#ifdef WIN32
				p[basesz] = '\\';
#else
				p[basesz] = '/';
#endif
				p[basesz + 1] = '\0';
			}
		}
		strcat(p, filename);
	}
	p2 = p;
	while (*p2) {
#ifdef WIN32
		if (*p2 == '/') *p2 = '\\';
#else
		if (*p2 == '\\') *p2 = '/';
#endif
		p2++;
	}
	return p;
}

/*
* 合并unix格式的路径，如果filename传入的是绝对路径，则直接返回filename的拷贝。
* 使用完后，需调用pcs_free来释放返回值
*/
char *combin_unix_path(const char *base, const char *filename)
{
	char *p, *p2;
	int basesz, filenamesz, sz;

	if (strcmp(filename, ".") == 0) {
		basesz = strlen(base);
		p = (char *)pcs_malloc(basesz + 1);
		assert(p);
		memset(p, 0, basesz + 1);
		memcpy(p, base, basesz);
		p[basesz] = '\0';
	}
	else if (strcmp(filename, "..") == 0) {
		basesz = strlen(base);
		p = (char *)pcs_malloc(basesz + 1);
		assert(p);
		memset(p, 0, basesz + 1);
		memcpy(p, base, basesz);
		p[basesz] = '\0';
		basesz--;
		if (p[basesz] == '/' || p[basesz] == '\\') p[basesz] = '\0';
		basesz--;
		while (basesz >= 0 && p[basesz] != '/' && p[basesz] != '\\') {
			basesz--;
		}
		if (basesz < 0) {
			p[0] = '\0';
		}
		else if (basesz == 0) {
			p[1] = '\0';
		}
		else {
			p[basesz] = '\0';
		}
	}
	else if (filename[0] == '/' || filename[0] == '\\' || filename[0] == '~') { /*如果是绝对路径，直接返回该值*/
		p = i_strdup(filename);
	}
	else {
		basesz = strlen(base);
		filenamesz = strlen(filename);
		sz = basesz + filenamesz + 1;
		p = (char *)pcs_malloc(sz + 1);
		assert(p);
		memset(p, 0, sz + 1);
		strcpy(p, base);
		if (p[basesz - 1] != '/') {
			p[basesz] = '/';
			p[basesz + 1] = '\0';
		}
		strcat(p, filename);
	}
	p2 = p;
	while (*p2) {
		if (*p2 == '\\') *p2 = '/';
		p2++;
	}
	return p;
}

/*
* 修正路径。
* 即把路径中反斜杠替换为正斜杠。
* 修正完成后，原样返回path
*/
char *fix_unix_path(char *path)
{
	char *p = path;
	while (*p) {
		if (*p == '\\') *p = '/';
		p++;
	}
	return path;
}

/*
* 读取全部文件内容
* file    - 待读取的文件
* pBuffer - 文件的内容所在的内存指针将存入pBuffer指定的内存中
* 返回读取到的字节大小。使用完成后，需调用pcs_free(*pBuffer)
*/
int read_file(const char *file, char **pBuffer)
{
	FILE *fp;
	long int save_pos;
	long size_of_file;
	char *content;

	fp = fopen(file, "rb");
	if (!fp) {
		//printf("Open file fail: %s\n", file);
		return -1;
	}
	save_pos = ftell(fp);
	fseek(fp, 0L, SEEK_END);
	size_of_file = ftell(fp);
	if (size_of_file < 3) {
		printf("Wrong file size: Size=%d, %s\n", size_of_file, file);
		fclose(fp);
		return -1;
	}
	fseek(fp, save_pos, SEEK_SET);
	content = (char *)pcs_malloc(size_of_file + 1);
	save_pos = fread(content, 1, size_of_file, fp);
	fclose(fp);
	content[size_of_file] = '\0';
	if ((((int)content[0]) & 0xEF) == 0xEF) {
		if ((((int)content[1]) & 0xBB) == 0xBB) {
			if ((((int)content[2]) & 0xBF) == 0xBF) {
				content[0] = content[1] = content[2] = ' ';
			}
			else {
				printf("Wrong UTF-8 BOM: BOM=0x%2X%2X%2X %s\n", content[0] & 0xFF, content[1] & 0xFF, content[2] & 0xFF, file);
				return -1;
			}
		}
		else {
			printf("Wrong UTF-8 BOM: BOM=0x%2X%2X%2X %s\n", content[0] & 0xFF, content[1] & 0xFF, content[2] & 0xFF, file);
			return -1;
		}
	}
	*pBuffer = content;
	return size_of_file;
}

/*从程序路径中找到文件名开始的位置，返回开始位置的指针*/
char *filename(char *path)
{
	char *p;
	p = path;
	p += strlen(p);
	while (p > path && *p != '/' && *p != '\\') p--;
	if (*p == '/' || *p == '\\') p++;
	return p;
}

/*
* 获取路径的父路径，如果没有父路径则返回NULL。
*   path  - 当前路径
*   len   - path的字节长度，如果传入-1，则内部使用strlen()获取其长度
* 返回值需要调用pcs_free()
*/
char *base_dir(const char *path, int len)
{
	char *dir, *p;
	if (!path) return NULL;
	if (len == -1) len = strlen(path);
	if (len == 0) return NULL;
	dir = (char *)pcs_malloc(len + 1);
	strcpy(dir, path);
	p = dir + len - 1;
	while (p > dir && *p != '/' && *p != '\\') p--;
	if (p == dir) {
		if (*p != '/' && *p != '\\') {
			pcs_free(dir);
			return NULL;
		}
		p[1] = '\0';
	}
	*p = '\0';
	return dir;
}

/*
string to time_t
时间格式 2009-3-24 0:00:08 或 2009-3-24
*/
int str2time(const char *str, time_t *timeData)
{
	char *pBeginPos = (char*)str;
	char *pPos = strstr(pBeginPos, "-");
	int iYear, iMonth, iDay, iHour, iMin, iSec;
	struct tm sourcedate = { 0 };
	if (pPos == NULL) {
		printf("strDateStr[%s] err \n", str);
		return -1;
	}
	iYear = atoi(pBeginPos);
	iMonth = atoi(pPos + 1);
	pPos = strstr(pPos + 1, "-");
	if (pPos == NULL) {
		printf("strDateStr[%s] err \n", str);
		return -1;
	}
	iDay = atoi(pPos + 1);
	iHour = 0;
	iMin = 0;
	iSec = 0;
	pPos = strstr(pPos + 1, " ");
	//为了兼容有些没精确到时分秒的
	if (pPos != NULL) {
		iHour = atoi(pPos + 1);
		pPos = strstr(pPos + 1, ":");
		if (pPos != NULL) {
			iMin = atoi(pPos + 1);
			pPos = strstr(pPos + 1, ":");
			if (pPos != NULL) {
				iSec = atoi(pPos + 1);
			}
		}
	}

	sourcedate.tm_sec = iSec;
	sourcedate.tm_min = iMin;
	sourcedate.tm_hour = iHour;
	sourcedate.tm_mday = iDay;
	sourcedate.tm_mon = iMonth - 1;
	sourcedate.tm_year = iYear - 1900;
	*timeData = mktime(&sourcedate);
	return 0;
}

/*
time_t to string 时间格式 2009-3-24 0:00:08
*/
char *time2str(char *buf, const time_t *t)
{
	char chTmp[100] = { 0 };
	struct tm *p;
	p = localtime(t);
	p->tm_year = p->tm_year + 1900;
	p->tm_mon = p->tm_mon + 1;

	snprintf(chTmp, sizeof(chTmp), "%04d-%02d-%02d %02d:%02d:%02d",
		p->tm_year, p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
	strcpy(buf, chTmp);
	return 0;
}