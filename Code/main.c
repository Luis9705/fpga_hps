///////////////////////////////////////
/// 640x480 version!
/// test VGA with hardware video input copy to VGA
// compile with
// gcc pio_test_1.c -o pio 
///////////////////////////////////////
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <sys/mman.h>
#include <sys/time.h> 
#include <math.h> 

#include "terasic_os_includes.h"
#include "LCD_Lib.h"
#include "lcd_graphic.h"
#include "font.h"


#define HW_REGS_BASE ( ALT_STM_OFST )
#define HW_REGS_SPAN ( 0x04000000 )
#define HW_REGS_MASK ( HW_REGS_SPAN - 1 )

// main bus; PIO
#define FPGA_AXI_BASE   0xC0000000
#define FPGA_AXI_SPAN   0x00001000

// read offset is 0x10 for both busses
// remember that eaxh axi master bus needs unique address
#define FPGA_PIO_READ   0x10
#define FPGA_PIO_WRITE  0x00

// main axi bus base
void *h2p_virtual_base;
volatile unsigned int * axi_pio_ptr = NULL ;
volatile unsigned int * axi_pio_read_ptr = NULL ;

void *virtual_base;
// /dev/mem file id
int fd;
LCD_CANVAS LcdCanvas;

// allocate space for the "string" pointers 
#define DISPLAY_LENGTH 16
#define LENGHT 32
#define HEIGHT 4
char p[HEIGHT][LENGHT+1];

enum{               //Numeración para la máquina de estados 
    INIT,
    READ_DATA,
    READ,
    WRITE,
    COMMAND,
    SHOW_DATA,
    DDRAM, //Display data RAM
    CGRAM, //Character Generator RAM
    FUNCTION_SET,
    CURSOR_DISPLAY_SHIFT,
    DISPLAY_CONTROL,
    ENTRY_MODE_SET,
    RETURN_HOME,
    CLEAR_DISPLAY,
    END,
    POST_END
};

void LCD_clear_paragraphs(){
    int j;
    int i;
    for(j = 0; j < HEIGHT; j++){
        for (i = 0; i < LENGHT; i++){
            p[j][i] = ' ';
        }  
        p[j][LENGHT] = '\0';
    }
}

void LCD_clear_paragraph(int paragraph){
    int i;
     for (i = 0; i < LENGHT; i++){
            p[paragraph][i] = ' ';
        }  
}

void LCD_print_paragraphs(int shift_pos){
    DRAW_Clear(&LcdCanvas, LCD_WHITE);
    int j;
    int k;
    int i;
    for(j = 0; j <HEIGHT; j++){
        for(k = 0; k < DISPLAY_LENGTH; k++){
            i = k+shift_pos;
            if (i < 0)
                i = LENGHT + i;
            else
                i = i%DISPLAY_LENGTH;
            DRAW_PrintChar(&LcdCanvas, k*8, j*16, p[j][i], LCD_BLACK, &font_16x16);
        }
    }
    DRAW_Refresh(&LcdCanvas);
}

void LCD_print_paragraph(int paragraph){
    DRAW_Clear(&LcdCanvas, LCD_WHITE);
    DRAW_PrintString(&LcdCanvas, 0, 16*paragraph, p[paragraph], LCD_BLACK, &font_16x16);
    DRAW_Refresh(&LcdCanvas);
}

void LCD_set_char(char word, int paragraph, int pos){
    if ((paragraph < HEIGHT) && (paragraph >= 0)){
        if ((pos < LENGHT ) && (paragraph >= 0)){
            p[paragraph][pos] = word;
        }
    }      
}

void LCD_set_string(char* string, int paragraph, int pos){
    int nLen, i;

    nLen = strlen(string);
    if(pos + nLen > LENGHT){
        nLen = LENGHT-pos;
    }

    for(i=0;i<nLen;i++){
        LCD_set_char(*(string+i), paragraph, pos+i);
    }

}

void LCD_start(){

    LCDHW_BackLight(true); // turn on LCD backlight

    LCD_clear_paragraphs();
    LCD_set_string("1234567890123456", 0,0);
    LCD_set_string("ABCDEFGHIJABCDEF", 1,0);
    LCD_set_string("abcdefghijabcdef", 2,0);
    LCD_set_string("+-/*@#$%^&*()!,.", 3,0);
    LCD_print_paragraphs(0);

    usleep( 1000*1000 ); //sleep 2 seconds

    LCD_clear_paragraphs();
    LCD_set_string("     HELLO", 1,0);
    LCD_set_string("   STARTING...", 2,0);
    LCD_print_paragraphs(0);

    usleep( 1000*1000 ); //sleep 2 seconds

    LCD_clear_paragraphs();
    LCD_print_paragraphs(0);

}

void send_start(volatile unsigned int * ptr){
    *ptr |= 1;
    usleep(1);
    *ptr ^= 1;
}

void send_rst(volatile unsigned int * ptr){
    *ptr |= 2;
    usleep(1);
    *ptr ^= 2;
}

int main(void)
{

    // Declare volatile pointers to I/O registers (volatile     
    // means that IO load and store instructions will be used   
    // to access these pointer locations,  
    // === get FPGA addresses ==================
    // Open /dev/mem
    if( ( fd = open( "/dev/mem", ( O_RDWR | O_SYNC ) ) ) == -1 )    {
        printf( "ERROR: could not open \"/dev/mem\"...\n" );
        return( 1 );
    }

    virtual_base = mmap( NULL, HW_REGS_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd, HW_REGS_BASE );
    if( virtual_base == MAP_FAILED ) {
        printf( "ERROR: mmap() failed...\n" );
        close( fd );
        return( 1 );
    }
    // ===========================================
    // get virtual address for
    // AXI bus addr 
    h2p_virtual_base = mmap( NULL, FPGA_AXI_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd, FPGA_AXI_BASE);   
    if( h2p_virtual_base == MAP_FAILED ) {
        printf( "ERROR: mmap3() failed...\n" );
        close( fd );
        return(1);
    }
    // Get the addresses that map to the two parallel ports on the AXI bus
    axi_pio_ptr =(unsigned int *)(h2p_virtual_base);
    axi_pio_read_ptr =(unsigned int *)(h2p_virtual_base + FPGA_PIO_READ);
    //============================================
    
    printf("Graphic LCD\r\n");
        
    LcdCanvas.Width = LCD_WIDTH;
    LcdCanvas.Height = LCD_HEIGHT;
    LcdCanvas.BitPerPixel = 1;
    LcdCanvas.FrameSize = LcdCanvas.Width * LcdCanvas.Height / 8;
    LcdCanvas.pFrame = (void *)malloc(LcdCanvas.FrameSize);
        
    if (LcdCanvas.pFrame == NULL){
            printf("failed to allocate lcd frame buffer\r\n");
    }else{          
        
        LCDHW_Init(virtual_base);
        
        LCD_Init();
        DRAW_Clear(&LcdCanvas, LCD_WHITE);

        unsigned int pio_read;
        unsigned char data, data_raw, rs, en, rw, rst;
        //unsigned char num, prev_num,st, prev_st;;

        int prev = 0;
        int state = INIT;
        int address_counter = 0;
        int shift_pos = 0;
        char mode_set = 0;
        bool show_cursor = false;
        *(axi_pio_ptr) = 0 ; 
        //prev_num = 0;
        //prev_st = 0;
        while(1) 
        {
            pio_read = *(axi_pio_read_ptr);

            if (prev != pio_read){

                data_raw = pio_read&0xFF;
                rw = (pio_read&0x100)>>8;
                rs = (pio_read&0x200)>>9;
                en = (pio_read&0x400)>>10;
                rst = (pio_read&0x800)>>11;
                //num = (pio_read&0xFE000)>>14;
                //st = (pio_read&0x700000)>>21;

                if((pio_read&0x1000)>>12)
                    printf("Reset recieved\r\n");
                if((pio_read&0x2000)>>13)
                    printf("Start recieved\r\n");
                
                if(((prev&0xFFF) != (pio_read&0xFFF)) && en){
                    printf("--- NEW COMMAND ---\r\n");
                    printf("E  RS  RW      DATA\r\n");
                    printf("%s", en ? "1":"0");
                    printf(" - ");
                    printf("%s", rs ? "1":"0");
                    printf(" - ");
                    printf("%s", rw ? "1":"0");
                    printf(" - ");
                    char n = data_raw;
                    char i = 8;
                    while (i) {
                        if(i==4)
                            printf(" ");
                        printf("%s", (n & 0x80) ? "1":"0");
                        n <<= 1;
                        i--;
                    }
                    printf("\r\n\r\n");
                }
                /*
                if((prev_num != num) && (state != INIT)){

                    char string = num + '0';
                    LCD_set_char(string, 3,0);
                    LCD_print_paragraphs(0);
                }

                if((prev_st != st) && (state != INIT)){

                    char string1 = st + '0';
                    LCD_set_char(string1, 3,3);
                    LCD_print_paragraphs(0);
                }
                */

                
            }
            if(rst){
                printf("Reset pressed\r\n\n\r");
                state = INIT;
                address_counter = 0;
                shift_pos = 0;
                mode_set = 0;
                show_cursor = false;
            }

            switch(state){
                case INIT:
                    printf("Initializing...\r\n");
                    LCD_start();
                    send_rst(axi_pio_ptr);
                    printf("Reset sended\r\n");
                    //send_start(axi_pio_ptr);
                    //printf("Send start\r\n");
                    printf("LCD ready\r\n\r\n") ;
                    LCDHW_BackLight(false);

                    printf("Waiting for commands...\r\n") ;
                    state = READ_DATA;
                    break;
                case READ_DATA:
                    if (en){
                        printf("Enable detected\r\n");
                        //usleep(1); //wait for Data Delay Time (Tddr)
                        data = data_raw;
                        if (rw)
                            state = READ;
                        else
                            state = WRITE;                           
                    }

                    break;
                case READ:
                    printf("Enter to READ\r\n");
                    printf( "Read is not enabled\r\n" );
                    state = END;  
                    break;
                case WRITE:
                    printf("Enter to WRITE\r\n");
                    if (rs)
                        state = SHOW_DATA;
                    else
                        state = COMMAND;
                    break;
                case COMMAND:
                    printf("Enter to COMMAND\r\n");
                    if (prev != pio_read) //Data not hold
                        state = 1000;
                    else{
                        if(data&0x80)  
                            state = DDRAM;
                        else if(data&0x40)
                            state = CGRAM;
                        else if(data&0x20)
                            state = FUNCTION_SET;
                        else if(data&0x10)
                            state = CURSOR_DISPLAY_SHIFT;
                        else if(data&0x8)
                            state = DISPLAY_CONTROL;
                        else if(data&0x4)
                            state = ENTRY_MODE_SET;
                        else if(data&0x2)
                            state = RETURN_HOME;
                        else 
                            state = CLEAR_DISPLAY;
                    }
                    break;
                case SHOW_DATA:
                    printf("Enter to SHOW_DATA\r\n");
                    printf("Printing: %c\r\n",data);
                    LCD_set_char(data, (address_counter&0x60)>>5, address_counter&0x1F);
                    LCD_print_paragraphs(shift_pos);

                    switch (mode_set){
                        case 0:
                            printf("Mode 0: Address counter incremented\r\n");
                            address_counter++;
                            break;
                        case 1:
                            printf("Mode 1: Address counter incremented, shift display\r\n");
                            address_counter++;
                            shift_pos++;
                            if (shift_pos > (LENGHT -1))
                                shift_pos = -LENGHT;
                            break;
                        case 2:
                            printf("Mode 2: Address counter incremented\r\n");
                            address_counter--;
                            break;
                         case 3:
                            printf("Mode 3: Address counter decremented, shift display\r\n");
                            address_counter--;
                            shift_pos--;
                            if (shift_pos < -LENGHT)
                                shift_pos = LENGHT -1;
                            break;
                        default:
                            address_counter = 0;
                            shift_pos = 0;
                            break;
                        if(address_counter < 0)
                            address_counter = 0x7F;
                        else if (address_counter > 0x7F)
                            address_counter = 0;
                    }
                    state = END;  
                    break;
                case DDRAM:
                    printf("Enter to DDRAM\r\n");
                    printf("Cursor set to (%d,%d)\r\n", (address_counter&0x60)>>5, address_counter&0x1F);
                    address_counter = data&0x7F;
                    state = END;  
                    break;
                case CGRAM:
                    printf("Character generator not enabled\n");
                    state = END;  
                    break;
                case FUNCTION_SET:
                    printf("Enter to FUNCTION_SET\r\n");
                    if(data&0x8)
                        printf("Changing the number of display lines is not enabled\n");
                    if(data&0x4)
                        printf("Changing the display font is not enabled\n");
                    if(data&0x10)
                        printf("Changing the display font is not enabled\n");
                    else
                        printf("4 bits data length is not enabled\n");
                    state = END;  
                    break;
                case CURSOR_DISPLAY_SHIFT:
                    printf("Enter to CURSOR_DISPLAY_SHIFT\r\n");
                    if(data&0x8){
                        if(data&0x4){
                            printf("Display shifted to right\r\n");
                            shift_pos++;
                            if (shift_pos > (LENGHT -1))
                                shift_pos = -LENGHT;
                        }
                        else{
                            printf("Display shifted to left\r\n");
                            shift_pos--;
                            if (shift_pos < -LENGHT)
                                shift_pos = LENGHT -1;
                        }
                    }
                    else{
                        if(data&0x4){
                            printf("Address counter incremented\r\n");
                            address_counter++;
                            if (address_counter > 0x7F)
                                address_counter = 0;
                        }
                        else{
                            printf("Address counter decremented\r\n");
                            address_counter--;
                            if(address_counter < 0)
                                address_counter = 0x7F;
                        }

                    }

                    state = END;  
                    break;
                case DISPLAY_CONTROL:
                    printf("Enter to DISPLAY_CONTROL\r\n");
                    if(data&0x4){
                        printf("Display turned on\r\n");
                        LCDHW_BackLight(true); 
                    }
                    else{
                        printf("Display turned off\r\n");
                        LCDHW_BackLight(false); 
                    }
                    if(data&0x2)
                        show_cursor = true;
                    else
                        show_cursor = false;
                    if(data&0x1)
                       printf("Cursor blinking not enabled\n");
                    state = END;  
                    break;
                case ENTRY_MODE_SET:
                    printf("Enter to ENTRY_MODE_SET\r\n");
                    printf("Mode set to %d\r\n",data&0x3);
                    mode_set = data&0x3;
                    state = END;  
                    break;
                case RETURN_HOME:
                    printf("Enter to RETURN_HOME\r\n");
                    if (prev != pio_read) //Data not hold
                        state = 1000;
                    else{
                        shift_pos = 0;
                        address_counter = 0;
                        state = END;
                    }
                    break;
                case CLEAR_DISPLAY:    
                    printf("Enter to CLEAR_DISPLAY\r\n"); 
                    if (prev != pio_read) //Data not hold
                        state = 1000;
                    else{
                        shift_pos = 0;
                        address_counter = 0;
                        LCD_clear_paragraphs();
                        LCD_print_paragraphs(shift_pos);
                        state = END;
                    }
  
                    break;
                case END:
                    printf("Waiting for enable=0.\r\n");
                    state = POST_END;
                    break;
                case POST_END:
                    //printf("Enter to END\r\n");
                    if(en == 0){
                        printf("Enable cleared. END.\r\n\r\n");
                        state = READ_DATA;
                    }
                    break;
                default:
                    printf( "ERROR\n" );
                    state = INIT;
                    break;
            }//end switch case

            prev = pio_read;
            //prev_num = num;
            //prev_st = st;
        } // end while(1)

    free(LcdCanvas.pFrame);
    }    
    // clean up our memory mapping and exit
    if( munmap( virtual_base, HW_REGS_SPAN ) != 0 ) {
        printf( "ERROR: munmap() failed...\n" );
        close( fd );
        return( 1 );
    }
    close( fd );
    return( 0 );
} // end main

/// end /////////////////////////////////////