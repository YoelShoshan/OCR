#include "zedusOS.h"
#include <assert.h>
#include <stdio.h>

#if defined(_MSC_VER)

#include <Windows.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <conio.h>

unsigned int zosGetProcessID()
{
	return GetCurrentProcessId();
}

bool zosCreateDirectory(const char *dir_path)
{
	//assert(0=="zosCreateDirectory not implemented for windows yet!");
	printf("Trying to created directory [%s]\n", dir_path);
	bool res = CreateDirectory(dir_path, NULL);

	if (!res)
	{
		printf("Failed!\n");
	}
	return false;
}

void zosShowMessageBox(const char *title, const char *msg)
{
	MessageBoxA(0,msg,title,0);
}

void zosExitProcess(unsigned int exit_code)
{
	ExitProcess(exit_code);
}

bool zosDoesFileExist(const char *file_path)
{
	struct stat s;
	if( stat(file_path,&s) == 0 )
	{
		if( s.st_mode & S_IFREG )
		{
			return true;
		}
		return false;
	}

	return false;
}

bool zosDoesDirectoryExist(const char* path)
{
	struct stat s;
	if( stat(path,&s) == 0 )
	{
		if( s.st_mode & S_IFDIR )
		{
			return true;
		}
		return false;
	}
	return false;
}

void zosPressAnyKeyToContinue()
{
	int c;
	printf( "\nPress a key to continue...\n" );
	c = getch();
	if (c == 0 || c == 224) getch();
}


#elif defined(__GNUC__)

#include <stdlib.h>
#include <unistd.h>
unsigned int zosGetProcessID()
{
	return getpid();
}

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

bool zosCreateDirectory(const char *dir_path)
{
	struct stat st = {0};

	if (stat(dir_path, &st) == -1) 
	{
	    printf("Creating directory [%s]\n", dir_path);
	    if(!mkdir(dir_path, 0700))
	    {
		printf("Error! errno=%d - Maybe the directory exists already?\n", errno);
  	    }
	} else
	{
	    printf("Directory already exists.\n");
	}

	return true;
}

void zosShowMessageBox(const char *title, const char *msg)
{
	printf("%s not implemented yet!!!\n",__FUNCTION__);
	assert(0=="Not implemented yet!");
}

void zosExitProcess(unsigned int exit_code)
{
	exit(0);
}

bool zosDoesFileExist(const char *file_path)
{
	struct stat s;
	if( stat(file_path,&s) == 0 )
	{
		if( s.st_mode & S_IFREG )
		{
			return true;
		}
		return false;
	}

	return false;
}

bool zosDoesDirectoryExist(const char* dir_path)
{
	struct stat s;
	if( stat(dir_path,&s) == 0 )
	{
		if( s.st_mode & S_IFDIR )
		{
			return true;
		}
		return false;
	}
	return false;
}

void zosPressAnyKeyToContinue()
{
	printf("%s not implemented yet!!!\n",__FUNCTION__);
	assert(0=="Not implemented yet!");
}

#else
#error define your copiler
#endif
