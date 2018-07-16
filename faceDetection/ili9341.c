
#include "ili9341.h"
#include <limits.h>
#include "padframe.h"
#include "rt/rt_api.h"
#include "font.c"

#define MADCTL_MY  0x80     ///< Bottom to top
#define MADCTL_MX  0x40     ///< Right to left
#define MADCTL_MV  0x20     ///< Reverse Mode
#define MADCTL_ML  0x10     ///< LCD refresh Bottom to top
#define MADCTL_RGB 0x00     ///< Red-Green-Blue pixel order
#define MADCTL_BGR 0x08     ///< Blue-Green-Red pixel order
#define MADCTL_MH  0x04     ///< LCD refresh right to left

#define SSPI_WRITE(a,v)          spiWrite(a,v)                ///< Software SPI write 8 bits
#define SPI_WRITE16(a,s)         SSPI_WRITE(a,(s) >> 8); SSPI_WRITE(a,s)  ///< Software SPI write 16 bits
#define SSPI_WRITE32(a,l)        SSPI_WRITE(a,(l >> 24)); SSPI_WRITE(a,(l >> 16)); SSPI_WRITE(a,(l >> 8)); SSPI_WRITE(a,l)   ///< Software SPI write 32 bits
#define SSPI_WRITE_PIXELS(a,c,l) for(unsigned int i=0; i<(l); i+=2){ SSPI_WRITE(a,((unsigned char*)(c))[i+1]); SSPI_WRITE(a,((unsigned char*)(c))[i]); }   ///< Software SPI write 'l' pixels (16 bits each)

RT_L2_DATA unsigned int _width=0; 
RT_L2_DATA unsigned int _height=0;
RT_L2_DATA int16_t cursor_x=0;
RT_L2_DATA int16_t cursor_y=0;
RT_L2_DATA int16_t wrap=0;
RT_L2_DATA uint8_t textsize=1;
RT_L2_DATA uint16_t textcolor=ILI9341_GREEN;
RT_L2_DATA uint16_t textbgcolor=ILI9341_WHITE;

void ILI9341_begin(rt_spim_t *spim)
{

    SPI_DC_INIT();

    writeCommand(spim,0xEF);
    spiWrite(spim,0x03);
    spiWrite(spim,0x80);
    spiWrite(spim,0x02);

    writeCommand(spim,0xCF);
    spiWrite(spim,0x00);
    spiWrite(spim,0XC1);
    spiWrite(spim,0X30);

    writeCommand(spim,0xED);
    spiWrite(spim,0x64);
    spiWrite(spim,0x03);
    spiWrite(spim,0X12);
    spiWrite(spim,0X81);

    writeCommand(spim, 0xE8);
    spiWrite(spim,0x85);
    spiWrite(spim,0x00);
    spiWrite(spim,0x78);

    writeCommand(spim,0xCB);
    spiWrite(spim,0x39);
    spiWrite(spim,0x2C);
    spiWrite(spim,0x00);
    spiWrite(spim,0x34);
    spiWrite(spim,0x02);

    writeCommand(spim,0xF7);
    spiWrite(spim,0x20);

    writeCommand(spim,0xEA);
    spiWrite(spim,0x00);
    spiWrite(spim,0x00);

    writeCommand(spim,ILI9341_PWCTR1);    //Power control
    spiWrite(spim,0x23);   //VRH[5:0]

    writeCommand(spim,ILI9341_PWCTR2);    //Power control
    spiWrite(spim,0x10);   //SAP[2:0];BT[3:0]

    writeCommand(spim,ILI9341_VMCTR1);    //VCM control
    spiWrite(spim,0x3e);
    spiWrite(spim,0x28);

    writeCommand(spim,ILI9341_VMCTR2);    //VCM control2
    spiWrite(spim,0x86);  //--

    writeCommand(spim,ILI9341_MADCTL);    // Memory Access Control
    spiWrite(spim,0x48);

    writeCommand(spim,ILI9341_VSCRSADD); // Vertical scroll
    SPI_WRITE16(spim,0);                 // Zero

    writeCommand(spim,ILI9341_PIXFMT);
    spiWrite(spim,0x55);

    writeCommand(spim,ILI9341_FRMCTR1);
    spiWrite(spim,0x00);
    spiWrite(spim,0x18);

    writeCommand(spim,ILI9341_DFUNCTR);    // Display Function Control
    spiWrite(spim,0x08);
    spiWrite(spim,0x82);
    spiWrite(spim,0x27);

    writeCommand(spim,0xF2);    // 3Gamma Function Disable
    spiWrite(spim,0x00);

    writeCommand(spim,ILI9341_GAMMASET);    //Gamma curve selected
    spiWrite(spim,0x01);

    writeCommand(spim,ILI9341_GMCTRP1);    //Set Gamma
    spiWrite(spim,0x0F);
    spiWrite(spim,0x31);
    spiWrite(spim,0x2B);
    spiWrite(spim,0x0C);
    spiWrite(spim,0x0E);
    spiWrite(spim,0x08);
    spiWrite(spim,0x4E);
    spiWrite(spim,0xF1);
    spiWrite(spim,0x37);
    spiWrite(spim,0x07);
    spiWrite(spim,0x10);
    spiWrite(spim,0x03);
    spiWrite(spim,0x0E);
    spiWrite(spim,0x09);
    spiWrite(spim,0x00);

    writeCommand(spim,ILI9341_GMCTRN1);    //Set Gamma
    spiWrite(spim,0x00);
    spiWrite(spim,0x0E);
    spiWrite(spim,0x14);
    spiWrite(spim,0x03);
    spiWrite(spim,0x11);
    spiWrite(spim,0x07);
    spiWrite(spim,0x31);
    spiWrite(spim,0xC1);
    spiWrite(spim,0x48);
    spiWrite(spim,0x08);
    spiWrite(spim,0x0F);
    spiWrite(spim,0x0C);
    spiWrite(spim,0x31);
    spiWrite(spim,0x36);
    spiWrite(spim,0x0F);

    writeCommand(spim,ILI9341_SLPOUT);    //Exit Sleep
    rt_time_wait_us(120000);
    writeCommand(spim,ILI9341_DISPON);    //Display on
    rt_time_wait_us(120000);

    _width  = ILI9341_TFTWIDTH;
    _height = ILI9341_TFTHEIGHT;
    
}


#ifndef pgm_read_byte
 #define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#endif
#ifndef pgm_read_word
 #define pgm_read_word(addr) (*(const unsigned short *)(addr))
#endif
#ifndef pgm_read_dword
 #define pgm_read_dword(addr) (*(const unsigned long *)(addr))
#endif

void setTextWrap(signed short w) {
    wrap = w;
}

void setCursor(signed short x, signed short y) {
    cursor_x = x;
    cursor_y = y;
}

void setTextColor(uint16_t c) {
    // For 'transparent' background, we'll set the bg
    // to the same as fg instead of using a flag
    textcolor = c;
}

void writePixelAtPos(rt_spim_t *spim,int16_t x, int16_t y, uint16_t color) {
    if((x < 0) ||(x >= (int16_t)_width) || (y < 0) || (y >= (int16_t)_height)) return;
    setAddrWindow(spim,x,y,1,1);
    writePixel(spim,color);
}

void drawChar(rt_spim_t *spim,int16_t x, int16_t y, unsigned char c,
  uint16_t color, uint16_t bg, uint8_t size) {

        if((x >= (int16_t)_width)            || // Clip right
           (y >= (int16_t)_height)           || // Clip bottom
           ((x + 6 * size - 1) < 0) || // Clip left
           ((y + 8 * size - 1) < 0))   // Clip top
            return;

        //if(!_cp437 && (c >= 176)) c++; // Handle 'classic' charset behavior

        for(int8_t i=0; i<5; i++ ) { // Char bitmap = 5 columns
            uint8_t line = pgm_read_byte(&font[c * 5 + i]);
            for(int8_t j=0; j<8; j++, line >>= 1) {
                if(line & 1) {
                    if(size == 1)
                        writePixelAtPos(spim,x+i, y+j, color);
                    else
                        writeFillRect(spim,x+i*size, y+j*size, size, size, color);
                } else if(bg != color) {
                    if(size == 1)
                        writePixelAtPos(spim,x+i, y+j, bg);
                    else
                        writeFillRect(spim,x+i*size, y+j*size, size, size, bg);
                }
            }
        }


    } 

 
void writeChar(rt_spim_t *spim,uint8_t c) {

     if(c == '\n') {                        // Newline?
            cursor_x  = 0;                     // Reset x to zero,
            cursor_y += textsize * 8;          // advance y one line
        } else if(c != '\r') {                 // Ignore carriage returns
            if(wrap && ((cursor_x + textsize * 6) > (short int)_width)) { // Off right?
                cursor_x  = 0;                 // Reset x to zero,
                cursor_y += textsize * 8;      // advance y one line
            }
            drawChar(spim,cursor_x, cursor_y, c, textcolor, textbgcolor, textsize);
            cursor_x += textsize * 6;          // Advance x one char
        }
}

void writeText(rt_spim_t *spim,char* str,int fontsize){
    textsize= fontsize;
    int i=0;
    while(str[i] != '\0')
        writeChar(spim,str[i++]);
}

void setRotation(rt_spim_t *spim,unsigned char m) {
    int rotation = m % 4; // can't be higher than 3
    switch (rotation) {
        case 0:
            m = (MADCTL_MX | MADCTL_BGR);
            _width  = ILI9341_TFTWIDTH;
            _height = ILI9341_TFTHEIGHT;
            break;
        case 1:
            m = (MADCTL_MV | MADCTL_BGR);
            _width  = ILI9341_TFTHEIGHT;
            _height = ILI9341_TFTWIDTH;
            break;
        case 2:
            m = (MADCTL_MY | MADCTL_BGR);
            _width  = ILI9341_TFTWIDTH;
            _height = ILI9341_TFTHEIGHT;
            break;
        case 3:
            m = (MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
            _width  = ILI9341_TFTHEIGHT;
            _height = ILI9341_TFTWIDTH;
            break;
    }

    writeCommand(spim, ILI9341_MADCTL);
    spiWrite(spim, m);
}

void writePixel(rt_spim_t *spim,unsigned short color){
    SPI_WRITE16(spim, color);
}

void writeColor(rt_spim_t *spim, unsigned short color, unsigned int len){

    for (uint32_t t=0; t<len; t++){
        writePixel(spim,color);
    }
    return;
}

void writePixels(rt_spim_t *spim,uint16_t * colors, uint32_t len){
    SSPI_WRITE_PIXELS(spim,(uint8_t*)colors , len * 2);
}

void drawRGBBitmap(rt_spim_t *spim,signed short x, signed short y,
  unsigned short *pcolors, signed short w, signed short h) {

    int16_t x2, y2; // Lower-right coord
    if(( x             >= (short int) _width ) ||      // Off-edge right
       ( y             >= (short int) _height) ||      // " top
       ((x2 = (x+w-1)) <  0      ) ||      // " left
       ((y2 = (y+h-1)) <  0)     ) return; // " bottom

    int16_t bx1=0, by1=0, // Clipped top-left within bitmap
            saveW=w;      // Save original bitmap width value
    if(x < 0) { // Clip left
        w  +=  x;
        bx1 = -x;
        x   =  0;
    }
    if(y < 0) { // Clip top
        h  +=  y;
        by1 = -y;
        y   =  0;
    }
    if(x2 >= (short int)_width ) w = _width  - x; // Clip right
    if(y2 >= (short int)_height) h = _height - y; // Clip bottom

    pcolors += by1 * saveW + bx1; // Offset bitmap ptr to clipped top-left
    //startWrite();
    setAddrWindow(spim,x, y, w, h); // Clipped area
    while(h--) { // For each (clipped) scanline...
      writePixels(spim,pcolors, w); // Push one (clipped) row
      pcolors += saveW; // Advance pointer by one full (unclipped) line
    }
    //endWrite();
}

#define LINES_PER_CHUNK 2

void lcd_pushPixels(rt_spim_t *spim,  uint16_t x, uint16_t y,uint16_t w,uint16_t h, uint16_t *p) {

    setAddrWindow(spim,x, y, w, h); // Clipped area
    for(int i=0;i<h;i+=LINES_PER_CHUNK){
    
        if((i+LINES_PER_CHUNK)<h-1)
        rt_spim_send(spim,    p+(w*i), (w)*LINES_PER_CHUNK*16, RT_SPIM_CS_KEEP, NULL);
        else
        rt_spim_send(spim,    p+(w*i), (w)*LINES_PER_CHUNK*16, RT_SPIM_CS_AUTO, NULL);
    }

}


void setAddrWindow(rt_spim_t *spim,unsigned short x, unsigned short y, unsigned short w, unsigned short h) {
    uint32_t xa = ((uint32_t)x << 16) | (x+w-1);
    uint32_t ya = ((uint32_t)y << 16) | (y+h-1);
    writeCommand(spim,ILI9341_CASET); // Column addr set
    SSPI_WRITE32(spim,xa);
    writeCommand(spim,ILI9341_PASET); // Row addr set
    SSPI_WRITE32(spim,ya);
    writeCommand(spim,ILI9341_RAMWR); // write to RAM
}

void writeFillRect(rt_spim_t *spim, unsigned short x, unsigned short y, unsigned short w, unsigned short h, unsigned short color){
    if((x >= _width) || (y >= _height)) return;
    short int x2 = x + w - 1, y2 = y + h - 1;
    if((x2 < 0) || (y2 < 0)) return;

    // Clip right/bottom
    if(x2 >= (short int) _width)  w = _width  - x;
    if(y2 >= (short int) _height) h = _height - y;
    unsigned int len = (int32_t)w * h;

    setAddrWindow(spim,x, y, w, h);
    writeColor(spim,color, len);
}

void SPI_DC_INIT(){
    gap8SetOnePadAlternate(PLP_PAD_TIMER0_CH2, PLP_PAD_GPIO_ALTERNATE1);
    gap8SetGPIODir(gap8GiveGpioNb(PLP_PAD_TIMER0_CH2), GPIO_DIR_OUT);
    gap8WriteGPIO(gap8GiveGpioNb(PLP_PAD_TIMER0_CH2), 0x0);
}

void SPI_DC_LOW(){
  gap8WriteGPIO(gap8GiveGpioNb(PLP_PAD_TIMER0_CH2), 0x0);
}

void SPI_DC_HIGH(){
  gap8WriteGPIO(gap8GiveGpioNb(PLP_PAD_TIMER0_CH2), 0x1);
}


//32 bit L2 Buffer
RT_L2_DATA unsigned char data[4] __attribute__((aligned(32)));


void spiWrite(rt_spim_t *spim, unsigned char  d) 
{
  data[0]=d;
  rt_spim_send(spim, data, 8, RT_SPIM_CS_AUTO, NULL);
}



void writeCommand(rt_spim_t *spim,unsigned char cmd){
    SPI_DC_LOW();
    spiWrite(spim,cmd);
    SPI_DC_HIGH();
}