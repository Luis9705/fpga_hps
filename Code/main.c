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
#define LENGHT 16
#define HEIGHT 4
char p[HEIGHT][LENGHT+1];

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

void LCD_print_paragraphs(){
    DRAW_Clear(&LcdCanvas, LCD_WHITE);
    int j;
    for(j = 0; j <HEIGHT; j++){
        DRAW_PrintString(&LcdCanvas, 0, 16*j, p[j], LCD_BLACK, &font_16x16);
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
        LCDHW_BackLight(true); // turn on LCD backlight
        
        LCD_Init();
        
        DRAW_Clear(&LcdCanvas, LCD_WHITE);
        LCD_clear_paragraphs();
        LCD_set_string("1234567890123456", 0,0);
        LCD_set_string("ABCDEFGHIJABCDEF", 1,0);
        LCD_set_string("abcdefghijabcdef", 2,0);
        LCD_set_string("+-/*@#$%^&*()!,.", 3,0);
        LCD_print_paragraphs();

        usleep( 5000*1000 ); //sleep 2 seconds

        LCD_clear_paragraphs();
        LCD_set_string("Write ready", 1,2);
        LCD_print_paragraphs();
        printf("Write ready\n\r") ;


        int num, pio_read;
        //int junk; 
        int prev = 0;
        while(1) 
        {
            // input a number
            //junk = scanf("%d", &num);
            // send to PIOs
            //*(axi_pio_ptr) = num ; 
            pio_read = *(axi_pio_read_ptr);

            if (prev != pio_read){

                char arr = pio_read +'0';
                LCD_set_char(arr, 2,7);
                LCD_print_paragraphs();
                printf("LCD Modificado\n\r") ;
            }
            // receive back and print
            //printf("pio in=%d\n\r", pio_read) ;

            prev = pio_read;

            usleep(10);
            
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