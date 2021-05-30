#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include <pthread.h>
#include "gpio.h"

#define POUT_ULTR 23
#define PIN_ULTR 24

int distance = 0;

void error_handling(char *message){
	fputs(message,stderr);
	fputc('\n',stderr);
	exit(1);
}

void *ultrawave_thd(){
	clock_t start_t, end_t;
	double time;
	//Enable GPIO pins
	if(-1 == GPIOExport(POUT_ULTR) || -1 == GPIOExport(PIN_ULTR)) {   // 초음파 익스포트
		printf("gpio export err!\n");
		exit(0);
	}
	// wait for writing to export file
	usleep(100000);

	//Set GPIO directions
	if(-1 == GPIODirection(POUT_ULTR, OUT) || -1 == GPIODirection(PIN_ULTR, IN)){
		printf("gpio direction err!\n");
		exit(0);
	}
	//init ultrawave trigger
	GPIOWrite(POUT_ULTR, 0);
	usleep(10000);
	//start
	while(1){
		if(-1 == GPIOWrite(POUT_ULTR, 1)){
			printf("gpio write/trigger err\n");
			exit(0);
		}
		//lsec == 1000000ultra_sec, 1ms = 10000ultra_sec
		usleep(10);
		GPIOWrite(POUT_ULTR, 0);
		//printf("hello world\n");
		while(GPIORead(PIN_ULTR) == 0){
			start_t = clock();
		}

		while(GPIORead(PIN_ULTR) == 1){
			end_t = clock();
		}

		time = (double)(end_t - start_t)/CLOCKS_PER_SEC; // ms
		distance = time/2 * 34000;

		if(distance > 900) distance = 900;

		//printf("time : %.4lf\n", time);
		//printf("distance : %d cm\n", distance); //.5lf

		usleep(500000);
	}
	//printf("after while\n");
}

static void usingSocket(int *sock, char* inputServerInfo[], struct sockaddr_in* serv_addr){
    *sock = socket(PF_INET, SOCK_STREAM, 0);
	if(*sock == -1)   // socket fd == -1
	    error_handling("socket() error");

    memset(serv_addr, 0, sizeof(struct sockaddr_in));
    serv_addr->sin_family = AF_INET;
    serv_addr->sin_addr.s_addr = inet_addr(inputServerInfo[1]);
    serv_addr->sin_port = htons(atoi(inputServerInfo[2]));

    if(connect(*sock, (struct sockaddr*)serv_addr, sizeof(struct sockaddr)) == -1)
	    error_handling("connect() error");
}

int main(int argc, char *argv[]) {
	int sock;
	struct sockaddr_in *serv_addr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
	char msg[2];
	char on[2]="1";
	int start_len;
	int light = 0;

	if(argc!=3){
		printf("Usage : %s <IP> <port>\n",argv[0]);
		exit(1);
	}

    usingSocket(&sock, argv, serv_addr);

    while(1){      // 2초마다 시스템이 시작할건지 main파이로 부터 읽기
        sleep(2);
        if((start_len = read(sock, msg, sizeof(msg))) == -1)
            continue;

        if(strncmp(on, msg, 1) == 0)
            break;
    }
    printf("switch on and system start!!\n");

    pthread_t p_thread[2];
	pthread_create(&p_thread[0], NULL, ultrawave_thd, NULL); // 초음파 센싱 시작

    // main thread
	while(1){
        //if(-1 == write(sock, distance, sizeof(distance)))  // 센싱값 전송
        //    break;

        printf("distance : %d cm\n", distance);
        write(sock, &distance, sizeof(distance));
        printf("send\n");

        sleep(1); // 1초마다 초음파 센싱 결과 전송
	}

	close(sock);
    sleep(10);

	//Disable GPIO pins
	if (-1 == GPIOUnexport(POUT_ULTR))
		return(4);

	return(0);
}
