// UTFT_Rotate_Bitmap (C)2012 Henning Karlsen
// web: http://www.henningkarlsen.com/electronics
//
// This program is a demo of the drawBitmap()-function.
//
// This program requires the UTFT library.
//

#include <UTFT.h>

// Uncomment the next line for 86Duino One/EduCake
//UTFT myGLCD(ITDB32S,16,17,18,19);   // Remember to change the model parameter to suit your display module!

// Uncomment the next line for 86Duino Zero
UTFT myGLCD(ITDB32S,8,9,10,11);   // Remember to change the model parameter to suit your display module!

extern unsigned short tux[0x400];

void setup()
{
  myGLCD.InitLCD(LANDSCAPE);
  myGLCD.fillScr(255, 255, 255);
  myGLCD.setColor(0, 0, 0);
}

void loop()
{
    for (int i=0; i<360; i+=5)
    {
      myGLCD.drawBitmap (10, 10, 32, 32, tux, i, 16, 16);
    }
}

