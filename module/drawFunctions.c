#include "drawFunctions.h"

uint8_t imageBuffer[200][25];

void drawPixel(uint8_t x, uint8_t y, bool color)
{
    if(x > 199 || y > 199)
    {
        PDEBUG("%s: out of bound for X:%d Y:%d\n", __func__, x, y);
        return;
    }

    uint8_t xPos, xBitPos;

    xPos = x / 8;
    xBitPos = x % 8;
    
    if(color)
        imageBuffer[y][xPos] &= ~(1 << (7 - xBitPos));
    else
        imageBuffer[y][xPos] |= (1 << (7 - xBitPos));

    // imageBuffer[y][xPos] = 0xFF;
    // uint8_t bit = (1 << (7 - xBitPos));

    // printf("For X: %d Y:%d | Binary: "BYTE_TO_BINARY_PATTERN"\n", x, y, BYTE_TO_BINARY(imageBuffer[y][xPos]));
    // printf("For X: %d Y:%d | Binary: "BYTE_TO_BINARY_PATTERN"\n", x, y, BYTE_TO_BINARY(bit));
    
}

void drawLineX(uint16_t startPosX, uint16_t length, uint16_t startPosY, bool color)
{
    if(startPosX > 199 || startPosY > 199)
    {
        PDEBUG("%s: out of bound for X:%d Y:%d\n", __func__, startPosX, startPosY);
        return;
    }

    int i;
    for(i = 0; i < length; i++)
    {
        if(startPosX + i > 199)
        {
            PDEBUG("%s: Warning: Line write out of bound for X:%d Y:%d at rel length: %d\n", __func__, startPosX, startPosY, startPosX + i);
        }
        drawPixel(startPosX + i, startPosY, color);
    }
}

void drawLineY(uint16_t startPosX, uint16_t length, uint16_t startPosY, bool color)
{
    if(startPosX > 199 || startPosY > 199)
    {
        PDEBUG("%s: out of bound for X:%d Y:%d\n", __func__, startPosX, startPosY);
        return;
    }

    int i;
    for(i = 0; i < length; i++)
    {
        if(startPosY + i > 199)
        {
            PDEBUG("%s: Warning: Line write out of bound for X:%d Y:%d at rel length: %d\n", __func__, startPosX, startPosY, startPosX + i);
        }
        drawPixel(startPosX, startPosY + i, color);
    }
}

//Write a single letter on the LCD
void writeLetter(uint16_t posX, uint16_t posY, bool color, char inChar)
{
    if(posX > 199 || posY > 199)
    {
        PDEBUG("%s: out of bound for X:%d Y:%d\n", __func__, posX, posY);
        return;
    }


    //Return if invalid character entered
    if(inChar < ' ' || inChar > '~')
        return;

    //Offset the input if spce is detected
    if(inChar == ' ')
        inChar = '~' + 1;

    //Offset character to index 0
    inChar -= '!';

    //Loop counters
    int i = 0;
    int j = 0;
    //Start and end index of the array
    int startFont = inChar * 11;
    int endFont = (inChar * 11) + 10;
    // setCursorXY(posX, posY);

    //Send command
    for(i = startFont; i <= endFont; i++){
        //Write X axis
        for(j = 7; j >= 0; j--){
            //Write Y axis
            uint16_t temp = ((lucidaConsole_8ptBitmaps_narrow[i] >> j) & 0x01);
            if(temp){
                // spi16bytes(color);
                drawPixel(posX, posY, DISP_BLACK);
            } else {
                // spi16bytes(BLACK);
                drawPixel(posX, posY, DISP_WHITE);
            }
            posX++;
        }
        posX -= 8;

        //Set the new cursor position
        // setCursorXY(posX, ++posY);
        posY++;

    }
}

//Write a string of characters on the LCD
void writeString(uint16_t posX, uint16_t posY, bool color, char *inString)
{
    //If the string still exists, write it to the LCD
    while(*inString){
        writeLetter(posX, posY, color, *inString++);
        //Move X to the right by the character width
        posX += 7;
    }
}

void drawLine(int x0, int y0, int x1, int y1, bool color) 
{
    if(x0 == x1)
    {
        int len = abs(y0 - y1);
        drawLineY(x0, len, (y0 > y1) ? y1 : y0, color);
        return;
    }

    if(y0 == y1)
    {
        int len = abs(x0 - x1);
        drawLineX((x0 > x1) ? x1 : x0, len, y0, color);
        return;
    }


    /* Bresenham algorithm */
    int dx = x1 - x0 >= 0 ? x1 - x0 : x0 - x1;
    int sx = x0 < x1 ? 1 : -1;
    int dy = y1 - y0 <= 0 ? y1 - y0 : y0 - y1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    // PDEBUG("\n\n\n\n");
    while((x0 != x1) && (y0 != y1)) {
        drawPixel(x0, y0 , color);
        // PDEBUG("LINE X: %d Y: %d\n", x0, y0);
        if (2 * err >= dy) {     
            err += dy;
            x0 += sx;
        }
        if (2 * err <= dx) {
            err += dx; 
            y0 += sy;
        }
    }
}

// void drawLine(int x1, int y1, int x2, int y2, bool color) 
// {
//     int gd,gm,x,y,end,p,dx,dy;
	
// 	dx=abs(x1-x2);
// 	dy=abs(y1-y2);
// 	p = 2*dy-dx;
	
// 	if(x1>x2)
// 	{
// 		x=x2;
// 		y=y2;
// 		end=x1;
// 	}
// 	else
// 	{
// 		x=x1;
// 		y=y1;
// 		end=x2;
// 	}
// 	drawPixel(x, y, color);
	
// 	while(x<=end)
// 	{
// 		if(p<0)
// 		{
// 			x++;
// 			p=p+2*dy;
// 		}
// 		else
// 		{
// 			x++;
// 			y++;
// 			p=p+2*(dy-dx);
// 		}
// 		drawPixel(x, y, color);
//         PDEBUG("LINE X: %d Y: %d\n", x, y);
// 	}
// }


void updateDisplay(void)
{
    SetMemoryArea(0, 0, 200 - 1, 200 - 1);
    SetMemoryPointer(0, 0);
    SendCommand(WRITE_RAM);
    int i, j;
    for(i = 0; i < 200; i++)
    {
        for(j = 0; j < 200 / 8; j++)
        {
            // printf("Send: %d %d\n", i, j);
            SendData(imageBuffer[i][j]);
            // printf("SENT FUNC X: %d Y:%d \t Binary: "BYTE_TO_BINARY_PATTERN"\n", i, j, BYTE_TO_BINARY(imageBuffer[i][j]));
        }
    }
    DisplayFrame();
}
