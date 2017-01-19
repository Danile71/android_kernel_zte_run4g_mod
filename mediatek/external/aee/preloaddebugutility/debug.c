#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>
#include <pthread.h>
#include <dlfcn.h>
#include <cutils/properties.h>

#define SIGNUM 6
const int sigtype[SIGNUM] = {SIGABRT, SIGBUS, SIGFPE, SIGILL, SIGSEGV, SIGTRAP};

__attribute__((constructor)) static void __pldbgutil_init()
{
	char value[PROPERTY_VALUE_MAX] = {'\0'};
	property_get("ro.build.type", value, "user");
	if (!strncmp(value, "eng", sizeof("eng"))) {
		property_get("persist.aee.core.direct", value, "default");
		if (strncmp(value, "disable", sizeof("disable"))) {
			int loop;
			for (loop = 0; loop < SIGNUM; loop++) {
				signal(sigtype[loop], SIG_DFL);
			}
		}
	}
	else {
		property_get("persist.aee.core.direct", value, "default");
		if (!strncmp(value, "enable", sizeof("enable"))) {
			int loop;
			for (loop = 0; loop < SIGNUM; loop++) {
				signal(sigtype[loop], SIG_DFL);
			}
		}
	}
}
