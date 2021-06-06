#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <string.h>


/*
    Reference Python Code for i2c: https://bitbucket.org/MattHawkinsUK/rpispy-misc/raw/master/python/lcd_i2c.py
    Reference C Code for i2c bus opening : https://raspberry-projects.com/pi/programming-in-c/i2c/using-the-i2c-interface
*/

#define I2C_ADDR 0x27 
#define LCD_WIDTH 16

#define LCD_CHR 1 // sending data
#define LCD_CMD 0 // sending command

// #define LCD_LINE_1  0X80 // LCD RAM ADDRESS FOR THE 1ST LINE
// #define LCD_LINE_2  0xC0 // LCD RAM address for the 2nd line
// #define LCD_LINE_3  0x94 // LCD RAM address for the 3rn line
// #define LCD_LINE_4  0xD4 // LCD RAM address for the 4th line 

#define LCD_BACKLIGHT  0x08 // ON
// #define LCD_BACKLIGHT = 0x00 //OFF

#define ENABLE  0b00000100 // ENABLE BIT

//Timing constants
#define E_PULSE  0.0005
#define E_DELAY  0.0005


int file_i2c;
int length;
unsigned char buffer[60] = {0};
unsigned char text_buffer[15] = {0};




void bus_open(){
  //OPEN I2C BUS
    char *filename = (char*)"/dev/i2c-1";
    if((file_i2c = open(filename, O_RDWR)) < 0){
        //ERROR 
        printf("Failed to open i2c bus");
        return;
    }

    // int addr = 0x5a;
    if(ioctl(file_i2c, I2C_SLAVE, I2C_ADDR) < 0)
    {
        printf("Failed to acquire bus access and / or talk to slave. \n");
        //ERROR HANDLING; you can check errno to see what went wrong
        return;
    } 
  
}

void bus_read(){
   //------ READ BYTES -------
    length = 4;
    if(read(file_i2c, buffer, length) != length)
    {
        //ERROR
        printf("Failed to read from the i2c bus.\n");
    }
    else{
        printf("Data read: %s\n",buffer);
    } 
}

void bus_write_bit(int write_bit){
    // WRITE BYTES
    buffer[0] = write_bit;
    length = 1;
    if(write(file_i2c,buffer,length) != length)
    {
        //ERRPR
        printf("Failed to write to the i2c bus.\n");
    }
}

void bus_write_text(char character){
    // WRITE BYTES
    text_buffer[0] = character;
    length = 1;
    if(write(file_i2c,text_buffer,length) != length)
    {
        //ERRPR
        printf("Failed to write to the i2c bus.\n");
    }
}
void lcd_toggle_enable(int bits){
    sleep(E_DELAY);
    bus_write_bit(bits | ENABLE);

    sleep(E_PULSE);
    bus_write_bit(bits & ~ENABLE);
    
    sleep(E_DELAY);
}


void lcd_byte(char bits, int mode){
    // send byte to data pins
    // bits = data
    // mode 1 for data, 0 for command
    // 데이터를 4 비트 씩 쪼개서 전송한다! 따라서 비트마스킹 필요함

    char bits_high = mode | (bits & 0xF0) | LCD_BACKLIGHT;
    char bits_low = mode | ((bits<<4) & 0xF0) | LCD_BACKLIGHT;

    // High bits
    bus_write_text(bits_high);
    lcd_toggle_enable(bits_high);

    bus_write_text(bits_low);
    lcd_toggle_enable(bits_low);
}

void lcd_init(){
    lcd_byte(0x33,LCD_CMD);
    lcd_byte(0x32, LCD_CMD);
    lcd_byte(0x06, LCD_CMD);
    lcd_byte(0x0C, LCD_CMD);
    lcd_byte(0x28, LCD_CMD);
    lcd_byte(0x01, LCD_CMD);

    sleep(E_DELAY);
}




void lcd_string(char *message, char line){
    // Send string to display
    lcd_byte(line,LCD_CMD);

    for(int i = 0 ; i < strlen(message); i ++){
        lcd_byte(message[i],LCD_CHR);
    }
    for(int i = strlen(message);i < LCD_WIDTH;i++){
        lcd_byte(' ',LCD_CHR);
    }

}

// int main(){
//     bus_open();
//     lcd_init();
//     sleep(2);

//     lcd_string(" HELLO WORLD!!!", LCD_LINE_1);
//     lcd_string("I'M RASPBERRY PI",LCD_LINE_2);

//     sleep(5);

//     lcd_string("LET'S ROCK",LCD_LINE_1);
//     lcd_string("LETS ROLLLL", LCD_LINE_2);

//     sleep(5);

//     return 0;
// }


