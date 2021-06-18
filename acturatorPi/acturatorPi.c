#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "LCD.h"
#include "LED.h"
#include <signal.h>
#include "motor1.h"
#define IN 0
#define OUT 1
#define LOW 0
#define HIGH 1
#define PIN 20
int motor_second = -1;//thread를 위한 전역변수 선언
int led1_sleep = -1;
int led2_sleep = -1;
int led3_sleep = -1;
pthread_t p_thread[4];

#define LCD_LINE_1 0X80 // LCD RAM ADDRESS FOR THE 1ST LINE
#define LCD_LINE_2 0xC0 // LCD RAM address for the 2nd line
#define LCD_LINE_3 0x94 // LCD RAM address for the 3rn line
#define LCD_LINE_4 0xD4 // LCD RAM address for the 4th line
int serv_sock, clnt_sock = -1;

void exit_handler(int sig) //강제종료시 발생하는 바인딩 에러를 방지하기 위한 핸들러
{
    close(clnt_sock);
    close(serv_sock);
    printf(" sensing PI exit\n");
    exit(1);
}

void error_handling(char *message) //에러핸들러
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

int motorstart()//모터를 회전시키기 위한 펑션
{

    PWMExport(0);
    PWMWritePeriod(0, 1000000);
    PWMWriteDutyCycle(0, 1000000); //초기값 팬이 회전하지 않음
    PWMEnable(0, 1);
     (led3_sleep == 1 && repeat % 2 == 0)
   
    while (1)
    {

        if (motor_second > 0) //main에서 전역변수 motor_second를 0보다 크게 설정하면 if문이 실행
        {
            PWMWriteDutyCycle(0, 100000);
            printf("oxygen fan : %d\n", motor_second); //1이상의 값을 넘겨주면 팬이 돌기 시작함. 메인에서 판단한 숫자에 따라 팬을 돌리는 시간을 달리한다.
            sleep(1);
            motor_second--;
        }
        else
        {
            PWMWriteDutyCycle(0, 1000000); //0이되면 쿨링팬이 작동하지 않음.
            sleep(1);
        }
    }
    //close(sock);
    return 0;
}

int LED1start() // 조도 조절 시스템
{
    int repeat = 10;
    if (-1 == GPIOExport(POUT))
        return (1);

    if (-1 == GPIODirection(POUT, OUT))
        return (2);

    while (1)
    {
        if (led1_sleep >= 0) //main에서 전역변수 Led1_sleep를 0보다 크거나 같게 설정하면 if문이 실행
        {
            do
            {
                if (-1 == GPIOWrite(POUT, repeat % 2))
                    return (3);
                usleep(100 * 1000 * led1_sleep);
                if (led1_sleep == 3 && repeat % 3 == 0)
                    printf("on Brightness System : %d\n", repeat / 3); //시스템 조절 출력 - 정도에 따라 출력 횟수의 변화
                if (led1_sleep == 1 && repeat % 2 == 0)
                    printf("on Brightness System : %d\n", repeat / 2); //시스템 조절 출력 - 정도에 따라 출력 횟수의 변화
            } while (repeat--);
            led1_sleep = -1;
            repeat = 10;
        }
    }

    if (-1 == GPIOUnexport(POUT))
        return (4);

    return (0);
}

int LED2start() // 온도 조절 시스템
{

    int repeat = 10;
    if (-1 == GPIOExport(POUT2))
        return (1);
    sleep(0.3);

    if (-1 == GPIODirection(POUT2, OUT))
        return (2);
    sleep(0.3);

    while (1)
    {
        if (led2_sleep >= 0) //main에서 전역변수 Led2_sleep를 0보다 크거나 같게 설정하면 if문이 실행
        {
            do
            {
                if (-1 == GPIOWrite(POUT2, repeat % 2))
                    return (3);
                usleep(100 * 1000 * led2_sleep);
                if (led2_sleep == 3 && repeat % 3 == 0)
                    printf("on Temperture System : %d\n", repeat / 3); //시스템 조절 출력 - 단계에 따라 출력 속도의 변화
                if (led2_sleep == 1 && repeat % 2 == 0)
                    printf("on Temperture System : %d\n", repeat / 2); //시스템 조절 출력 - 단계에 따라 출력 속도의 변화
            } while (repeat--);
            led2_sleep = -1;

            repeat = 10;//횟수는 10회
        }
    }

    sleep(0.3);
    if (-1 == GPIOUnexport(POUT2))
        return (4);

    return (0);
}

int LED3start() // 온습도 조절 시스템
{

    int repeat = 10;
    if (-1 == GPIOExport(POUT3))
        return (1);
    sleep(0.3);

    if (-1 == GPIODirection(POUT3, OUT))
        return (2);
    sleep(0.3);

    while (1)
    {
        if (led3_sleep >= 0) //main에서 전역변수 Led3_sleep를 0보다 크거나 같게 설정하면 if문이 실행
        {
            do
            {
                if (-1 == GPIOWrite(POUT3, repeat % 2))
                    return (3);
                usleep(100 * 1000 * led3_sleep);
                if (led3_sleep == 3 && repeat % 3 == 0)
                    printf("on Humid & Dry System : %d\n", repeat / 3); //시스템 조절 출력 - 정도에 따라 출력 횟수의 변화
                if (led3_sleep == 1 && repeat % 2 == 0)
                    printf("on Humid & Dry System : %d\n", repeat / 2); //시스템 조절 출력 - 정도에 따라 출력 횟수의 변화
            } while (repeat--);
            led3_sleep = -1;
            repeat = 10;
        }
    }

    sleep(0.3);
    if (-1 == GPIOUnexport(POUT3))
        return (4);

    return (0);
}

int main(int argc, char *argv[])
{
    signal(SIGINT, exit_handler);
    int state = 1;
    int prev_state = 1;
    int light = 0;

    struct sockaddr_in serv_addr, clnt_addr;//소켓 생성
    socklen_t clnt_addr_size;
    char msg[18];

    bus_open();
    lcd_init();
    sleep(2);

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);//소켓연결
    if (serv_sock == -1)
        error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");
    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");
    if (clnt_sock < 0)
    {
        clnt_addr_size = sizeof(clnt_addr);
        clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr,
                           &clnt_addr_size);
        if (clnt_sock == -1)
            error_handling("accept() error");
    }
    printf("Success connect to Main Pi\n");// if문 없이 출력이 된다면 연결 성공

    pthread_create(&p_thread[0], NULL, LED1start, NULL);//엑츄레이터 스레드 생성
    pthread_create(&p_thread[1], NULL, LED2start, NULL);
    pthread_create(&p_thread[2], NULL, LED3start, NULL);
    pthread_create(&p_thread[3], NULL, motorstart, NULL);

    while (1)
    {
        int ret = read(clnt_sock, msg, sizeof(msg));//서버로부터 정확한 입력여부 확인
        if (ret == -1)
            printf("read error\n");//입력 실패시 에러메세지

        if (msg > 0)
        {
            char arr[17];

            sprintf(arr, msg);
            lcd_string(arr, LCD_LINE_1);

            if (strncmp(msg, "Low oxygen:10%", 14) == 0)//입력값이 while문 내에서 if문을 변경, 전역변수 값을 바꾸며 thread 실행 
            {
                printf("%s\n", msg);
                //motorstart(10);
                motor_second = 5;
            }
            if (strncmp(msg, "Low oxygen:30%", 14) == 0)
            {
                printf("%s\n", msg);
                //motorstart(5);
                motor_second = 2;
            }
            if (strncmp(msg, "Brightness:10%", 14) == 0)
            {
                printf("%s\n", msg);
                //LED1start(1);
                led1_sleep = 1;
            }
            if (strncmp(msg, "Brightness:20%", 14) == 0)
            {
                printf("%s\n", msg);
                //LED1start(3);
                led1_sleep = 3;
            }
            if (strncmp(msg, "Brightness:80%", 14) == 0)
            {
                printf("%s\n", msg);
                //LED1start(3);
                led1_sleep = 3;
            }
            if (strncmp(msg, "Brightness:90%", 14) == 0)
            {
                printf("%s\n", msg);
                //LED1start(1);
                led1_sleep = 1;
            }
            if (strncmp(msg, "Low Temperture", 15) == 0)
            {
                printf("%s\n", msg);
                //LED2start(3);
                led2_sleep = 3;
            }
            if (strncmp(msg, "High Temperture", 15) == 0)
            {
                printf("%s\n", msg);
                //LED2start(1);
                led2_sleep = 1;
            }

            if (strncmp(msg, "Too Humid:10%", 13) == 0)
            {
                printf("%s\n", msg);
                // LED3start(1);
                led3_sleep = 1;
            }
            if (strncmp(msg, "Too Humid:20%", 13) == 0)
            {
                printf("%s\n", msg);
                // LED3start(3);
                led3_sleep = 3;
            }

            if (strncmp(msg, "Too Dry:80%", 11) == 0)
            {
                printf("%s\n", msg);
                led3_sleep = 3;
                //LED3start(3);
            }
            if (strncmp(msg, "Too Dry:90%", 11) == 0)
            {
                printf("%s\n", msg);
                led3_sleep = 1;
                //LED3start(1);
            }
        }
        usleep(500 * 100);
    }
    printf("end while\n");
    close(clnt_sock);
    close(serv_sock);
    if (-1 == GPIOUnexport(PIN) || -1 == GPIOUnexport(POUT))
        return (4);
    return (0);
}