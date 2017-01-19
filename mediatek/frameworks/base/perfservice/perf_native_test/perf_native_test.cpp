#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <fcntl.h>
#include <errno.h>
#include <dlfcn.h>

#include "PerfServiceNative.h"

#define LIB_FULL_NAME "/system/lib/libperfservicenative.so"

void (*perfBoostEnable)(int) = NULL;
void (*perfBoostDisable)(int) = NULL;
void (*perfBoostEnableTimeout)(int, int) = NULL;
int  (*perfUserScnReg)(int, int) = NULL;
int  (*perfUserScnRegBigLittle)(int, int, int, int) = NULL;
void (*perfUserScnUnreg)(int) = NULL;
int  (*perfUserGetCapability)(int) = NULL;
int  (*perfUserRegScn)(void) = NULL;
void (*perfUserRegScnConfig)(int, int, int, int, int, int) = NULL;
void (*perfUserUnregScn)(int) = NULL;
void (*perfUserScnEnable)(int) = NULL;
void (*perfUserScnDisable)(int) = NULL;
void (*perfUserScnEnableTimeout)(int, int) = NULL;
void (*perfUserScnResetAll)(void) = NULL;
void (*perfUserScnDisableAll)(void) = NULL;
void (*perfDumpAll)(void) = NULL;
void (*perfSetFavorPid)(int) = NULL;

typedef void (*ena)(int);
typedef void (*disa)(int);
typedef void (*ena_timeout)(int, int);
typedef int  (*user_reg)(int, int);
typedef int  (*user_reg_big_little)(int, int, int, int);
typedef void (*user_unreg)(int);
typedef int  (*user_get_capability)(int);
typedef int  (*user_reg_scn)(void);
typedef void (*user_reg_scn_config)(int, int, int, int, int, int);
typedef void (*user_unreg_scn)(int);
typedef void (*user_enable)(int);
typedef void (*user_disable)(int);
typedef void (*user_enable_timeout)(int, int);
typedef void (*user_reset_all)(void);
typedef void (*user_disable_all)(void);
typedef void (*dump_all)(void);
typedef void (*set_favor_pid)(int);

enum {
	CMD_BOOST_ENABLE = 1,
	CMD_BOOST_DISABLE,
	CMD_BOOST_ENABLE_TIMEOUT,
	CMD_USER_REG,
	CMD_USER_REG_BIG_LITTLE,
	CMD_USER_UNREG,
	CMD_USER_GET_CAPABILITY,
	CMD_USER_REG_SCN,
	CMD_USER_REG_SCN_CONFIG,
	CMD_USER_UNREG_SCN,
	CMD_USER_ENANLE,
	CMD_USER_DISABLE,
	CMD_USER_ENABLE_TIMEOUT,
	CMD_USER_RESET_ALL,
	CMD_USER_DISABLE_ALL,
	CMD_DUMP_ALL,
	CMD_SET_FAVOR_PID,
};

static void usage(char *cmd);
static int load_api(void);


int main(int argc, char* argv[])
{
    int command, scenario, timeout, scn_core, scn_freq, scn_core_big, scn_freq_big, scn_core_little, scn_freq_little;
	int cmd, p1, p2, p3, p4, pid, value;
	int handle = -1;

    if(argc < 2) {
		usage(argv[0]);
        return 0;
    }

    command = atoi(argv[1]);
	//printf("argc:%d, command:%d\n", argc, command);
	switch(command) {
		case CMD_USER_RESET_ALL:
		case CMD_USER_DISABLE_ALL:
		case CMD_DUMP_ALL:
		case CMD_USER_REG_SCN:
			if(argc!=2) {
				usage(argv[0]);
				return -1;
			}
			break;

		case CMD_BOOST_ENABLE:
		case CMD_BOOST_DISABLE:
		case CMD_USER_UNREG:
		case CMD_USER_ENANLE:
		case CMD_USER_DISABLE:
		case CMD_USER_GET_CAPABILITY:
	    case CMD_USER_UNREG_SCN:
	    case CMD_SET_FAVOR_PID:
			if(argc!=3) {
				usage(argv[0]);
				return -1;
			}
			break;

		case CMD_BOOST_ENABLE_TIMEOUT:
		case CMD_USER_ENABLE_TIMEOUT:
		case CMD_USER_REG:
			if(argc!=4) {
				usage(argv[0]);
				return -1;
			}
			break;

		case CMD_USER_REG_BIG_LITTLE:
			if(argc!=6) {
				usage(argv[0]);
				return -1;
			}
			break;

		case CMD_USER_REG_SCN_CONFIG:
			if(argc!=8) {
				usage(argv[0]);
				return -1;
			}
			break;

		default:
			usage(argv[0]);
			return -1;
	}

	if(command == CMD_BOOST_ENABLE || command == CMD_BOOST_DISABLE || command == CMD_BOOST_ENABLE_TIMEOUT) {
		switch(atoi(argv[2])) {
			case 1:
				scenario = SCN_APP_SWITCH;
				break;

			case 2:
				scenario = SCN_APP_ROTATE;
				break;

			case 3:
				scenario = SCN_SW_CODEC;
				break;

	        case 4:
				scenario = SCN_SW_CODEC_BOOST;
				break;

			case 5:
				scenario = SCN_APP_TOUCH;
				break;

			default:
				scenario = SCN_NONE;
				break;
		}
	}
	else if(command == CMD_USER_REG) {
		scn_core = atoi(argv[2]);
		scn_freq = atoi(argv[3]);
	}
	else if(command == CMD_USER_REG_BIG_LITTLE) {
		scn_core_big = atoi(argv[2]);
		scn_freq_big = atoi(argv[3]);
		scn_core_little = atoi(argv[4]);
		scn_freq_little = atoi(argv[5]);
	}
	else if(command == CMD_USER_UNREG || command == CMD_USER_ENANLE || command == CMD_USER_DISABLE) {
		handle = atoi(argv[2]);
	}
	else if(command == CMD_USER_ENABLE_TIMEOUT) {
		handle = atoi(argv[2]);
		timeout = atoi(argv[3]);
	}
	else if(command == CMD_BOOST_ENABLE_TIMEOUT) {
		timeout = atoi(argv[3]);
	}
	else if(command == CMD_USER_GET_CAPABILITY) {
		cmd = atoi(argv[2]);
	}
	else if(command == CMD_USER_REG_SCN_CONFIG) {
		handle = atoi(argv[2]);
		cmd = atoi(argv[3]);
		p1 = atoi(argv[4]);
		p2 = atoi(argv[5]);
		p3 = atoi(argv[6]);
		p4 = atoi(argv[7]);
	}
	else if(command == CMD_USER_UNREG_SCN) {
		handle = atoi(argv[2]);
	}
	else if(command == CMD_SET_FAVOR_PID) {
		pid = atoi(argv[2]);
	}

	/* load api */
	if(load_api() != 0)
		return -1;

	/* command */
    if(command == CMD_BOOST_ENABLE)
        perfBoostEnable(scenario);
    else if(command == CMD_BOOST_DISABLE)
        perfBoostDisable(scenario);
	else if(command == CMD_BOOST_ENABLE_TIMEOUT)
		perfBoostEnableTimeout(scenario, timeout);
	else if(command == CMD_USER_REG) {
		handle = perfUserScnReg(scn_core, scn_freq);
		printf("handle:%d\n", handle);
	}
	else if(command == CMD_USER_REG_BIG_LITTLE) {
		handle = perfUserScnRegBigLittle(scn_core_big, scn_freq_big, scn_core_little, scn_freq_little);
		printf("handle:%d\n", handle);
	}
	else if(command == CMD_USER_UNREG) {
		perfUserScnUnreg(handle);
	}
	else if(command == CMD_USER_GET_CAPABILITY) {
		value = perfUserGetCapability(cmd);
		printf("value:%d\n", value);
	}
	else if(command == CMD_USER_REG_SCN) {
		handle = perfUserRegScn();
		printf("handle:%d\n", handle);
	}
	else if(command == CMD_USER_REG_SCN_CONFIG) {
		perfUserRegScnConfig(handle, cmd, p1, p2, p3, p4);
	}
	else if(command == CMD_USER_UNREG_SCN) {
		perfUserUnregScn(handle);
	}
	else if(command == CMD_USER_ENANLE) {
		perfUserScnEnable(handle);
	}
	else if(command == CMD_USER_DISABLE) {
		perfUserScnDisable(handle);
	}
	else if(command == CMD_USER_ENABLE_TIMEOUT) {
		perfUserScnEnableTimeout(handle, timeout);
	}
	else if(command == CMD_USER_RESET_ALL) {
		perfUserScnResetAll();
	}
	else if(command == CMD_USER_DISABLE_ALL) {
		perfUserScnDisableAll();
	}
	else if(command == CMD_DUMP_ALL) {
		perfDumpAll();
	}
	else if(command == CMD_SET_FAVOR_PID) {
		perfSetFavorPid(pid);
	}
}


static int load_api(void)
{
    void *handle, *func;

	handle = dlopen(LIB_FULL_NAME, RTLD_NOW);

	func = dlsym(handle, "PerfServiceNative_boostEnable");
	perfBoostEnable = reinterpret_cast<ena>(func);

	if (perfBoostEnable == NULL) {
        printf("perfBoostEnable error: %s", dlerror());
		dlclose(handle);
		return -1;
	}

	func = dlsym(handle, "PerfServiceNative_boostDisable");
	perfBoostDisable = reinterpret_cast<disa>(func);

	if (perfBoostDisable == NULL) {
        printf("perfBoostDisable error: %s", dlerror());
		dlclose(handle);
		return -1;
	}

	func = dlsym(handle, "PerfServiceNative_boostEnableTimeout");
	perfBoostEnableTimeout = reinterpret_cast<ena_timeout>(func);

	if (perfBoostEnableTimeout == NULL) {
        printf("perfBoostEnableTimeout error: %s", dlerror());
		dlclose(handle);
		return -1;
	}

	func = dlsym(handle, "PerfServiceNative_userReg");
	perfUserScnReg = reinterpret_cast<user_reg>(func);

	if (perfUserScnReg == NULL) {
        printf("perfUserScnReg error: %s", dlerror());
		dlclose(handle);
		return -1;
	}

	func = dlsym(handle, "PerfServiceNative_userRegBigLittle");
	perfUserScnRegBigLittle = reinterpret_cast<user_reg_big_little>(func);

	if (perfUserScnRegBigLittle == NULL) {
        printf("perfUserScnRegBigLittle error: %s", dlerror());
		dlclose(handle);
		return -1;
	}

	func = dlsym(handle, "PerfServiceNative_userUnreg");
	perfUserScnUnreg = reinterpret_cast<user_unreg>(func);

	if (perfUserScnUnreg == NULL) {
        printf("perfUserScnUnreg error: %s", dlerror());
		dlclose(handle);
		return -1;
	}

	func = dlsym(handle, "PerfServiceNative_userGetCapability");
	perfUserGetCapability = reinterpret_cast<user_get_capability>(func);

	if (perfUserGetCapability == NULL) {
        printf("perfUserGetCapability error: %s", dlerror());
		dlclose(handle);
		return -1;
	}

	func = dlsym(handle, "PerfServiceNative_userRegScn");
	perfUserRegScn = reinterpret_cast<user_reg_scn>(func);

	if (perfUserRegScn == NULL) {
        printf("perfUserRegScn error: %s", dlerror());
		dlclose(handle);
		return -1;
	}

	func = dlsym(handle, "PerfServiceNative_userRegScnConfig");
	perfUserRegScnConfig = reinterpret_cast<user_reg_scn_config>(func);

	if (perfUserRegScnConfig == NULL) {
        printf("perfUserRegScnConfig error: %s", dlerror());
		dlclose(handle);
		return -1;
	}

	func = dlsym(handle, "PerfServiceNative_userUnregScn");
	perfUserUnregScn = reinterpret_cast<user_unreg_scn>(func);

	if (perfUserUnregScn == NULL) {
        printf("perfUserUnregScn error: %s", dlerror());
		dlclose(handle);
		return -1;
	}

	func = dlsym(handle, "PerfServiceNative_userEnable");
	perfUserScnEnable = reinterpret_cast<user_enable>(func);

	if (perfUserScnEnable == NULL) {
        printf("perfUserScnEnable error: %s", dlerror());
		dlclose(handle);
		return -1;
	}

	func = dlsym(handle, "PerfServiceNative_userDisable");
	perfUserScnDisable = reinterpret_cast<user_disable>(func);

	if (perfUserScnDisable == NULL) {
        printf("perfUserScnDisable error: %s", dlerror());
		dlclose(handle);
		return -1;
	}

	func = dlsym(handle, "PerfServiceNative_userEnableTimeout");
	perfUserScnEnableTimeout = reinterpret_cast<user_enable_timeout>(func);

	if (perfUserScnEnableTimeout == NULL) {
        printf("perfUserScnEnableTimeout error: %s", dlerror());
		dlclose(handle);
		return -1;
	}

	func = dlsym(handle, "PerfServiceNative_userResetAll");
	perfUserScnResetAll = reinterpret_cast<user_reset_all>(func);

	if (perfUserScnResetAll == NULL) {
        printf("perfUserScnResetAll error: %s", dlerror());
		dlclose(handle);
		return -1;
	}

	func = dlsym(handle, "PerfServiceNative_userDisableAll");
	perfUserScnDisableAll = reinterpret_cast<user_disable_all>(func);

	if (perfUserScnDisableAll == NULL) {
        printf("perfUserScnDisableAll error: %s", dlerror());
		dlclose(handle);
		return -1;
	}

	func = dlsym(handle, "PerfServiceNative_dumpAll");
	perfDumpAll = reinterpret_cast<dump_all>(func);

	if (perfDumpAll == NULL) {
        printf("perfDumpAll error: %s", dlerror());
		dlclose(handle);
		return -1;
	}

	func = dlsym(handle, "PerfServiceNative_setFavorPid");
	perfSetFavorPid = reinterpret_cast<set_favor_pid>(func);

	if (perfSetFavorPid == NULL) {
        printf("perfSetFavorPid error: %s", dlerror());
		dlclose(handle);
		return -1;
	}

	return 0;
}


static void usage(char *cmd) {
    fprintf(stderr, "\nUsage: %s command scenario\n"
                    "    command\n"
                    "        1: boost enbale\n"
                    "        2: boost disable\n"
                    "        3: boost enbale timeout\n"
                    "        4: user reg\n"
                    "        5: user reg big little\n"
                    "        6: user unreg\n"
                    "        7: user get capability\n"
                    "        8: user reg scn\n"
                    "        9: user reg scn config\n"
                    "       10: user unreg scn\n"
                    "       11: user enable\n"
                    "       12: user disable\n"
                    "       13: user enable timeout\n"
                    "       14: user reset all\n"
                    "       15: user disable all\n"
                    "       16: dump all\n"
                    "       17: set favor pid\n"
                    "    scenario\n"
                    "        1: app switch\n"
                    "        2: app rotate\n"
                    "        3: sw codec\n"
                    "        4: sw codec boost\n"
                    "        5: app boost\n", cmd);
}

