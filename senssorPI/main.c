#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include <pthread.h>

#include "gpio.h"
#include "spi.h"
#include <wiringPi.h>

#define bool   _Bool
#define false  0
#define true   1

// 초음파 센서 관련
#define POUT_ULTR 23
#define PIN_ULTR 24

// 온습도 센서 관련
#define USING_DHT11 true
#define DHT_GPIO 17
#define LH_THRESHOLD 26

// spi
static const char* DEVICE = "/dev/spidev0.0";

bool isSystemStart = false;
int press_result = -1, distance_result = -1, light_result = -1;
int humid_result = -1, temp_result = -1;
int sock;

void error_handling(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

void sense_ultrawave() {
	clock_t start_t, end_t;
	double time;
	//Enable GPIO pins
	if (-1 == GPIOExport(POUT_ULTR) || -1 == GPIOExport(PIN_ULTR)) {   // 초음파 익스포트
		printf("gpio export err!\n");
		exit(0);
	}
	// wait for writing to export file
	usleep(100000);

	//Set GPIO directions
	if (-1 == GPIODirection(POUT_ULTR, OUT) || -1 == GPIODirection(PIN_ULTR, IN)) {
		printf("gpio direction err!\n");
		exit(0);
	}
	//init ultrawave trigger
	GPIOWrite(POUT_ULTR, 0);
	usleep(10000);

	//start sensing
	if (-1 == GPIOWrite(POUT_ULTR, 1)) {
		printf("gpio write/trigger err\n");
		exit(0);
	}

	//lsec == 1000000ultra_sec, 1ms = 10000ultra_sec
	usleep(10);
	GPIOWrite(POUT_ULTR, 0);

	while (GPIORead(PIN_ULTR) == 0) {
		start_t = clock();
	}
	while (GPIORead(PIN_ULTR) == 1) {
		end_t = clock();
	}

	time = (double)(end_t - start_t) / CLOCKS_PER_SEC; // ms
	distance_result = time / 2 * 34000;   // distance 결과 저장

	if (distance_result > 900) distance_result = 900;

	usleep(500000);

	//Disable GPIO pins
	if (-1 == GPIOUnexport(POUT_ULTR))
		printf("GPIOUnexport fail\n");
}

static void usingSocket(int* sock, char* inputServerInfo[], struct sockaddr_in* serv_addr) {
	*sock = socket(PF_INET, SOCK_STREAM, 0);
	if (*sock == -1)   // socket fd == -1
		error_handling("socket() error");

	memset(serv_addr, 0, sizeof(struct sockaddr_in));
	serv_addr->sin_family = AF_INET;
	serv_addr->sin_addr.s_addr = inet_addr(inputServerInfo[1]);
	serv_addr->sin_port = htons(atoi(inputServerInfo[2]));

	if (connect(*sock, (struct sockaddr*)serv_addr, sizeof(struct sockaddr)) == -1)
		error_handling("connect() error");
}

void sense_press_and_light() {
	int fd = open(DEVICE, O_RDWR);

	if (fd <= 0) {
		printf("Device %s not found\n", DEVICE);
		return;
	}

	if (prepareSpi(fd) == -1) {
		return;
	}
	press_result = readadc(fd, 0);   // 채널 0번으로 부터 읽음
	light_result = readadc(fd, 6);

	close(fd);
}

void sense_temp_and_hum() {
	bool isValid = false;
	int curTemper, curHumid;

	wiringPiSetupGpio();
	piHiPri(99); // 타이밍 코드를 위해 가장 높은 우선 순위 사용

	while (isValid == false) {
		unsigned char data[5] = { 0, };

		// 시작 신호 보내기
		pinMode(DHT_GPIO, OUTPUT);
		digitalWrite(DHT_GPIO, LOW);
		delay(18);  // 최소 0.018초 동안 유지
		digitalWrite(DHT_GPIO, HIGH);
		pinMode(DHT_GPIO, INPUT);

		// 시작 신호 받기
		do {
			delayMicroseconds(1);
		} while (digitalRead(DHT_GPIO) == HIGH);
		do {
			delayMicroseconds(1);
		} while (digitalRead(DHT_GPIO) == LOW);
		do {
			delayMicroseconds(1);
		} while (digitalRead(DHT_GPIO) == HIGH);

		for (int dataIdx = 0; dataIdx < 5; dataIdx++) {  // 데이터 받기
			for (int bit = 0; bit < 8; bit++) {  // 8bit씩
				do {
					delayMicroseconds(1);
				} while (digitalRead(DHT_GPIO) == LOW); // Low 무시

				int width = 0;
				do {  // HIGH가 지속될 경우 무효
					width++;
					delayMicroseconds(1);
					if (width > 1000) break;
				} while (digitalRead(DHT_GPIO) == HIGH);

				data[dataIdx] = data[dataIdx] | ((width > LH_THRESHOLD) << (7 - bit));
			}
		}
		curTemper = data[2];  // 소수점 무시 (온도 소수점 윗자리)
		curHumid = data[0];   // 소수점 무시 (습도 소수점 윗자리)

		// 유효성 검사
		unsigned char sum = 0;
		for (int i = 0; i < 4; i++) {
			sum += data[i];
		}

		if (sum == data[4]) {   // 데이터가 유효한 경우 (data[4] : 패리티 비트)
			isValid = true;
			temp_result = curTemper;
			humid_result = curHumid;
		}
		else {
			sleep(2);
			printf("checksum is no valid!\n");
			temp_result = humid_result = -1;
		}
	}
}

void* checkSystemStart() {
	char isSwitchOn[2];
	char on[2] = "1";

	while (1) {
		if (read(sock, isSwitchOn, sizeof(isSwitchOn)) == -1)
			printf("system start read error\n");

		if (strncmp(on, isSwitchOn, 1) == 0) {
			isSystemStart = true;
			printf("switch on and sensing start\n");
		}
		else {
			isSystemStart = false;
			printf("switch off and sensing stop\n");
		}

		sleep(2);  // 1초마다 시스템이 on/off를 확인함
	}
}

int main(int argc, char* argv[]) {
	struct sockaddr_in* serv_addr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
	int sensing_result[5];  // 센싱 결과를 담을 배열, 인덱스 의미를 서로 조율함

	if (argc != 3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}

	usingSocket(&sock, argv, serv_addr);

	pthread_t p_thread[2];
	pthread_create(&p_thread[0], NULL, checkSystemStart, NULL);

	while (1) {
		while (isSystemStart == false) { sleep(0.5); }  // false이면 센싱하지 않는다.

		/* 아래는 isSystemStart == true 인 상태 */

		// 1. sensing 함수 호출
		sense_ultrawave();
		sense_press_and_light();
		sense_temp_and_hum();

		// 2. 결과 저장
		sensing_result[0] = press_result;
		sensing_result[1] = distance_result;
		sensing_result[2] = light_result;
		sensing_result[3] = temp_result;
		sensing_result[4] = humid_result;

		// 3. 결과 출력
		printf("press: %d, distance(oxygen): %d, light: %d, temp: %d, humid: %d\n",
			press_result, distance_result, light_result, temp_result, humid_result);
		sleep(0.5);

		// 4. 결과 전송
		if (-1 == write(sock, sensing_result, sizeof(sensing_result)))
			printf("send error\n");

		sleep(0.5);
		printf("send success!\n");

		sleep(3); // 3초마다 센싱 결과 보내기
	}

	close(sock);
	sleep(10);

	return(0);
}
