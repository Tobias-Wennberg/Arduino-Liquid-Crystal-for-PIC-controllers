/* 
 * File:   lq.h
 * Author: 21towe
 *
 * Created on October 20, 2023, 9:58 PM
 */

#ifndef LQ_H
#define	LQ_H

#ifdef	__cplusplus
extern "C" {
#endif
#include "mcc_generated_files/mcc.h"

#define D0 lcd_D0_LAT
#define D1 lcd_D1_LAT
#define D2 lcd_D2_LAT
#define D3 lcd_D3_LAT
#define D4 lcd_D4_LAT
#define D5 lcd_D5_LAT
#define D6 lcd_D6_LAT
#define D7 lcd_D7_LAT
#define RS lcd_RS_LAT
#define EN lcd_RW_LAT
    
    
void init();
void clear();
void home();
void noDisplay();
void display();
void noBlink();
void blink();
void noCursor();
void cursor();
void scrollDisplayLeft();
void scrollDisplayRight();
void leftToRight();
void rightToLeft();
void autoscroll();
void noAutoscroll();

void setRowOffsets(int row1, int row2, int row3, int row4);
void createChar(char, char[]);
void setCursor(char, char);
void write(char);
void command(char);


#ifdef	__cplusplus
}
#endif

#endif	/* LQ_H */

