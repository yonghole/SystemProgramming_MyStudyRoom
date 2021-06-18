#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


#define IN 0
#define OUT 1
#define LOW 0
#define HIGH 1
#define PIN 20
#define POUT 21


#define BUFFER_MAX 45
#define DIRECTION_MAX 45
#define VALUE_MAX 256

static int GPIOUnexport(int pin){
   char buffer[BUFFER_MAX];
   ssize_t bytes_written;
   int fd;
   
   fd=open("/sys/class/gpio/unexport",O_WRONLY);
   if(-1==fd){
      fprintf(stderr,"falied to pen unexport\n");
      return -1;
   }
   
   bytes_written=snprintf(buffer,BUFFER_MAX,"%d",pin);
   write(fd,buffer,bytes_written);
   close(fd);
   return 0;
}

static int GPIOExport(int pin){

   char buffer[BUFFER_MAX];
   ssize_t bytes_written;
   int fd;
   
   fd=open("/sys/class/gpio/export",O_WRONLY);
   if(-1==fd){
      fprintf(stderr, "failed export wr");
      return 1;
   }
   bytes_written=snprintf(buffer,BUFFER_MAX, "%d", pin);
   write(fd,buffer,bytes_written);
   close(fd);
   return 0;
}


static int GPIODirection(int pin,int dir){
      static const char s_directions_str[]="in\0out";
      
   
   char path[DIRECTION_MAX]="/sys/class/gpio/gpio%d/direction";
   int fd;
   
   snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction",pin);
   
   fd=open(path,O_WRONLY);
   if(-1==fd){
      fprintf(stderr,"Failed to open gpio direction for writing!\n");
      return -1;
   }
   
   if(-1==write(fd,&s_directions_str[IN == dir ? 0 :3], IN==dir ? 2:3)){
      fprintf(stderr,"failed to set direction!\n");
      return -1;
   }
   
   close(fd);
   return 0;
}



static int GPIOWrite(int pin, int value){
      static const char s_values_str[] ="01";
      
      char path[VALUE_MAX];
      int fd;
      
    //   printf("write value!\n");
      
      snprintf(path,VALUE_MAX, "/sys/class/gpio/gpio%d/value",pin);
      fd=open(path,O_WRONLY);
      if(-1==fd){
         fprintf(stderr,"failed open gpio write\n");
         return -1;
      }
      
      //0 1 selection
      if(1!=write(fd,&s_values_str[LOW==value ? 0:1],1)){
         fprintf(stderr,"failed to write value\n");
         return -1;
      }
      close(fd);
      return 0;
}

static int GPIORead(int pin){
    char path[VALUE_MAX];
    char value_str[3];
    int fd;

    snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value",pin);
    fd=open(path, O_RDONLY);
    if(-1==fd){
        fprintf(stderr,"failed to open gpio value for reading\n");
        return -1;
    }

    if(-1==read(fd,value_str,3)){
        fprintf(stderr,"failed to read val\n");
        return -1;
    }
    close(fd);

    return(atoi(value_str));
}


