#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include "gpio.h"
#include "lcd.h"
#include  <signal.h>

#define IN 0
#define OUT 1
#define LOW 0
#define HIGH 1
#define PIN 20
#define POUT 21

#define BUFFER_MAX 45
#define DIRECTION_MAX 45
#define VALUE_MAX 256

#define LCD_LINE_1  0X80 // LCD RAM ADDRESS FOR THE 1ST LINE
#define LCD_LINE_2  0xC0 // LCD RAM address for the 2nd line
#define LCD_LINE_3  0x94 // LCD RAM address for the 3rn line
#define LCD_LINE_4  0xD4 // LCD RAM address for the 4th line

#define bool   _Bool
#define false  0
#define true   1

int clk_timer[6];
bool pressure_flag = false;
bool is_timer_started = false;
char send_sig[16];
int act_socket;
bool switch_count = false;

int sensor_socket;

void increase_time(){
    clk_timer[5]++;
    if(clk_timer[5] == 10){
        clk_timer[5] = 0;
        clk_timer[4]++;
    }
    if(clk_timer[4] == 6){
        clk_timer[4] = 0;
        clk_timer[3]++;
    }
    if(clk_timer[3] == 10){
        clk_timer[3] = 0;
        clk_timer[2]++;
    }
    if(clk_timer[2] == 6){
        clk_timer[2] = 0;
        clk_timer[1]++;
    }
    if(clk_timer[1] == 10){
        clk_timer[1] = 0;
        clk_timer[0]++;
    } 
    
}
void error_handling(char *message){
    fputs(message,stderr);
    fputc('\n',stderr);
    exit(1);
}

void exit_handler(int sig){ 
   close(act_socket);
   printf(" sensing PI exit\n");
   exit(1);
}

void exit_handler2(int sig){ 
   close(sensor_socket);
   printf(" sensing PI exit\n");
   exit(1);
}

void lcd_timer(){
   char out_time[8];
    while(1){
        sprintf(out_time,"%d%d:%d%d:%d%d",clk_timer[0],clk_timer[1],clk_timer[2],clk_timer[3],clk_timer[4],clk_timer[5],clk_timer[6],clk_timer[7]);
        lcd_string("Studying...",LCD_LINE_1);
        lcd_string(out_time,LCD_LINE_2);
        if(pressure_flag && switch_count)increase_time();
        sleep(1);
   }
}
void act_init(int port_num,char addr[]){
    int sock;
    struct sockaddr_in actu_addr;
    char msg[2];
    char on[2] = "1";
    int str_len;
    int light = 0;
    
    sock = socket(PF_INET,SOCK_STREAM,0);
    act_socket = sock;
    //printf("%d\n",sock);
    memset(&actu_addr, 0, sizeof(actu_addr));
    actu_addr.sin_family = AF_INET;
    // printf("hello\n");
    actu_addr.sin_addr.s_addr = inet_addr(addr);
    // printf("%d\n",port_num);
    actu_addr.sin_port = htons(port_num);
    // printf("1111\n");
    printf("Connecting to Actuator Pi....\n");
    if(connect(sock,(struct sockaddr *)&actu_addr,sizeof(actu_addr))==-1){
        error_handling("connect() error");
    }
    printf("Successfully Connected to Actuator Pi\n");
}
void send_actuator(){
    
    // printf("in send\n");
    char msg[16];
    // sprintf(msg,"%d",send_sig);
    printf("send to actuator : %s\n",send_sig);
    int ret = write(act_socket,send_sig,sizeof(send_sig));
    if(ret == -1){
        printf("Write failed..\n");
    }
    // printf("11\n");
    usleep(500*100);
    sleep(2);
}

void buttonRead(){
    int repeat = 10000;
    int state = 1;
    int prev_state = 0;
    int light = 0;

   //Enable GPIO pins
    if (-1 == GPIOExport(PIN) || -1 == GPIOExport(POUT))
        return (1);
    //Set GPIO directions
    if (-1 == GPIODirection(PIN, IN) || -1 == GPIODirection(POUT, OUT))
        return (2);
    if (-1 == GPIOWrite(POUT, 1))
        return (3);


    do{
        // if( -1 == GPIOWrite(POUT2,1)){
        //     return 3;
        // }
        GPIORead(PIN);
        // printf("I'm reading %d in GPIO %d\n", GPIORead(PIN), PIN);
        int count = 0;
        if(GPIORead(PIN) != 1){ // 버튼이 눌리지 않은 상태, 불은 꺼져있음
            while(GPIORead(PIN) != 1){
            //    printf("waiting for %d\n", state);
             // 버튼이 눌려있는 상태, 불은 꺼져있음
               continue; 
            }
            // GPIOWrite(POUT,state); // 불을 켠다
            char led_on[2];
            sprintf(led_on,"%d",1);
            int ret = write(sensor_socket,led_on,sizeof(led_on));
            if(switch_count){
                switch_count = false;
            }
            else{
                for(int i=0;i<6;i++){
                    clk_timer[i] = 0;
                }
                printf("Program Start with invoking Sensor Pi.\n");
                switch_count = true;
            }
            usleep(100000);
            int temp = state;
            state =prev_state;
            prev_state = temp;

        }
        usleep(100000);
    }
    while(repeat--);

    if(-1 == GPIOUnexport(POUT) || -1 == GPIOUnexport(PIN)){
        return;
    }
}


int main(int argc, char *argv[])
{
    clk_timer[0] = 0;
    clk_timer[1] = 0;
    clk_timer[2] = 0;
    clk_timer[3] = 0;
    struct result{
        int distance;
        int light;
        int press;
    };
    int state = 1;
    int prev_state = 1;
    int light = 0;

    signal(SIGINT, exit_handler);
    signal(SIGINT, exit_handler2);

    char lcd_out[20];

    int serv_sock, clnt_sock = -1;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size;
    char msg[2];
    pthread_t p_thread[2];
    // 여기 활성화 해야함!!!!
    act_init(atoi(argv[2]),argv[3]);
    int button_read = pthread_create(&p_thread[1], NULL, buttonRead, NULL);
    // printf("%d\n",button_read);

    bus_open();
    lcd_init();
    sleep(2);
    //Enable GPIO pins
    if (-1 == GPIOExport(PIN) || -1 == GPIOExport(POUT))
        return (1);
    //Set GPIO directions
    if (-1 == GPIODirection(PIN, IN) || -1 == GPIODirection(POUT, OUT))
        return (2);
    if (-1 == GPIOWrite(POUT, 1))
        return (3);

    // if (argc != 2)
    // {
    //     printf("Usage : %s <port>\n", argv[0]);
    // }

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
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
        printf("Connected to Sensor Pi...\n");
    if (clnt_sock < 0)
    {
        clnt_addr_size = sizeof(clnt_addr);
        clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr,
                           &clnt_addr_size);
        sensor_socket = clnt_sock;
        if (clnt_sock == -1)
            error_handling("accept() error");
    }
    printf("Successfully Connected to Sensor Pi...\n");
    char rec_msg[2];
    int oxygen_cnt = 0;
    int light_cnt = 0;
    int temp_cnt = 0;
    int humid_cnt = 0;
    while (1)
    {
        state = GPIORead(PIN);
        
            int result[15];
            int retor = read(clnt_sock,result,sizeof(result));
            if(retor == -1) printf("read failed\n");
            // else printf("read success\n");
            // printf("%d %d %d\n", ptr->light, ptr->distance, ptr->press);
            // 0 : press, 1 : distance, 2 : light 3: temp 4 : humid
            // sprintf(lcd_out, "%d %d %d", result[0],result[1],result[2]);
            printf("pressure : %d, oxygen: %d, light : %d, temp : %d , humidity : %d\n", result[0], result[1], result[2], result[3], result[4]);
            if(result[0] >400 && !pressure_flag){
                if(!is_timer_started){
                    is_timer_started = true;
                    int check_timer_thread = pthread_create(&p_thread[0], NULL, lcd_timer, NULL);
                }
                pressure_flag = true;
            }
            else if(result[0] <=0){
                pressure_flag = false;
            }

            // Low oxygen -> turn on acuator
            // printf("is it zero?\n");
            
            if(result[1]<=50 && oxygen_cnt == 0){
                //send_sig = result[1];
               // printf("Distance %d\n", send_sig);
                oxygen_cnt = 3;
                if(result[1]>=0 && result[1] <=15){
                    sprintf(send_sig,"Low oxygen:10%%");
                    send_actuator();
                }
                else{
                    sprintf(send_sig,"Low oxygen:30%%");
                    send_actuator();
                }
                
            }
            else if(oxygen_cnt>=1){
                oxygen_cnt --;
            }
            
            //Light
            if(result[2] >=0 && light_cnt == 0){
                // printf("light here\n");
                 if(result[2] >=0 && result[2] <= 100){
                     light_cnt = 3;
                    sprintf(send_sig,"Brightness:10%%");
                    send_actuator();
                }
                if(result[2] >=100 && result[2] <= 200){
                    light_cnt = 3;
                    sprintf(send_sig,"Brightness:20%%");
                    send_actuator();
                }
                if(result[2] >=700 && result[2] <= 800){
                    light_cnt = 3;
                    sprintf(send_sig,"Brightness:80%%");
                    send_actuator();
                }
                if(result[2] >=800){
                    light_cnt = 3;
                    sprintf(send_sig,"Brightness:90%%");
                    send_actuator();
                }
            }
            else if(light_cnt >=1){
                light_cnt --;
            }

            if(result[3] >=0 && temp_cnt == 0){
                temp_cnt = 2;
                if(result[3] <=26){
                    sprintf(send_sig,"Low Temperture");
                    send_actuator();
                }
                else if(result[3] >=30){
                    sprintf(send_sig,"High Temperture");
                    send_actuator();
                }
            }
            else if(temp_cnt >=1){
                temp_cnt--;
            }

            if(result[4] >= 0 && humid_cnt == 0){
                humid_cnt = 2;
                if(result[4] <= 55){
                    sprintf(send_sig,"Too Dry:80%%");
                    send_actuator();
                }
                else if(result[4] >55  &&result[4] <= 60){
                    sprintf(send_sig, "Too Dry:90%%");
                    send_actuator();
                }
                else if(result[4] >= 90 && result[4] < 100){
                    sprintf(send_sig, "Too Humid:10%%");
                    send_actuator();
                }
                else if(result[4] >= 100){
                    sprintf(send_sig, "Too Humid:20%%");
                    send_actuator();
                }
            }
            else if(humid_cnt >= 1){
                humid_cnt--;
            }
            

            // if(result[3])
            // lcd_string("prs lght dist", LCD_LINE_2);
            // lcd_string(lcd_out,LCD_LINE_1);
            // sleep(2);
            // printf("3\n
        // usleep(50 * 100);
    }
    printf("while ended\n");
    close(clnt_sock);
    close(serv_sock);
    //Disable GPIO pins
    if (-1 == GPIOUnexport(PIN) || -1 == GPIOUnexport(POUT))
        return (4);
    return (0);
}
