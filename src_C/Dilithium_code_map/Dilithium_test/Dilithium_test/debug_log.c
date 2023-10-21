
#include "debug_log.h"

FILE* log_file;

void log_debug_log_init(const char* log_name)
{
	fopen_s(&log_file, log_name, "w");
}


void log_debug_deinit()
{
	fclose(log_file);
}