#ifndef _DEBUG_LOG_H_
#define _DEBUG_LOG_H_

#include <stdio.h>

extern FILE* log_file;

void log_debug_log_init(const char* log_name);
void log_debug_deinit();

#define USB_DEBUG_MSG(...) fprintf(log_file, __VA_ARGS__)

#endif