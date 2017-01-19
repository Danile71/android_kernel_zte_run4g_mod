#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <termios.h>

extern void CLI_Init(void);

#define CLI_MAGIC 'C'
//#define CLI_MAGIC 'CLI_MU3D'
#define IOCTL_READ _IOR(CLI_MAGIC, 0, int)
#define IOCTL_WRITE _IOW(CLI_MAGIC, 1, int)

int fd = -1;

int write_cmd(char *buf)
{
	int ret;

	if ((ret = ioctl(fd, IOCTL_WRITE, buf)) != 0)
		printf("[FAIL]IOCTL_WRITE ioctl=%x, errno=%x\n", IOCTL_WRITE, errno);
	else
		printf("[PASS]IOCTL_WRITE: %s\n", buf);

	//negative value is returned if there is problem
	return ret;
}

int read_msg(char *buf)
{
	int ret;

	if ((ret = ioctl(fd, IOCTL_READ, buf)) < 0)
		printf("IOCTL_READ ERROR");
	else
		printf("IOCTL_READ: %s\r\n", buf);

	//negative value is returned if there is problem
	return ret;
}

// eddie
void SetTTY(int restore)
{
	struct termios tty;
	static struct termios otty;
	//    int STDIN_FILENO = fileno(stdin);
	if (!restore)
	{
		// Get original tty settings and save them in otty
		tcgetattr(STDIN_FILENO, &otty);
		//tty = otty;
		memcpy(&tty, &otty, sizeof(struct termios));
		// Now set the terminal to char-by-char input
		tty.c_lflag = tty.c_lflag & (unsigned int) (~(ECHO | ECHOK | ICANON));
		tty.c_cc[VTIME] = 1;
		tcsetattr(STDIN_FILENO, TCSANOW, &tty);
	}
	else{
		// Reset to the original settings
		memcpy(&tty, &otty, sizeof(struct termios));
		tty.c_lflag = tty.c_lflag | (unsigned int) ((ECHO | ECHOK | ICANON));
		tcsetattr(STDIN_FILENO, TCSANOW, &tty);
		//        tcsetattr(STDIN_FILENO, TCSANOW, &otty);
	}
}


int main(int argc, const char ** argv)
{
	char cmd_buf[256] = "";
	int i ;
	//open cli character device
	//if ((fd = open("/dev/mu3d_mtk_test", O_RDWR)) < 0)
	if ((fd = open("/dev/cli", O_RDWR)) < 0)
	{
		printf("cannot open /dev/cli");
		return -1;
	}
	//eddie
	SetTTY(0);
#if 0	
	CLI_Init();
#else	
	// printf("[U3D_CLI] input , argc = %d\n", argc);

	// printf("[U3D_CLI] input , argv =[", argc);
	
	for (i= 1 ; i < argc ; i++){
		
		printf("%s ", argv[i]);
		strcat(cmd_buf, argv[i]) ;
		strcat(cmd_buf, " ") ;
	}		

	printf("]\n") ;

	printf("[U3D_CLI] the cmd buffer is %s\n", cmd_buf) ;

	write_cmd(cmd_buf) ;
#endif

	close(fd);
	SetTTY(1);
	return 0;
}

