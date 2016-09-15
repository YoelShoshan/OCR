#ifndef ZEDUS_OS_H
#define ZEDUS_OS_H

void zosShowMessageBox(const char *title, const char *msg);

void zosExitProcess(unsigned int exit_code);

bool zosDoesFileExist(const char *file_path);

bool zosDoesDirectoryExist(const char *dir_path);

void zosPressAnyKeyToContinue();

bool zosCreateDirectory(const char *dir_path);

unsigned int zosGetProcessID();

#endif
