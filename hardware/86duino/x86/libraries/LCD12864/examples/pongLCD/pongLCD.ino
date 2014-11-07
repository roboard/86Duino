#include <LCD12864.h>


// gameplay
int score = 0;
boolean lost = false;

// Joystick (Atari Joystick, commonly used in 2600 
int up = 1; // Yellow on the Joystick
int down = 5; // Brown on the Joystick


// BALL
int Xpos = 0;
int Ypos = 133;
int momentumX = 2;
int momentumY = 1;
long dumptimer = 0;
boolean momr = true;
boolean momd = true;

// Paddle
int PaddlePos = 144;




void setup() {

  LCDA.Initialise(); // INIT SCREEN

delay(500); // Some delay
LCDA.Render(); // Start the Rendering process
Title(); // Write PONG on screen
delay(500); // Some Delay

Serial.begin(9600); // In case you want to use the terminal in order to guide the paddle
}



void loop() { 

  // I connected an Atari joystick to analog pins 1 and 5
  // You DON't have to do this, you can use 'u' and 'd' in
  // the terminal if you simplky want to test your screen
  // (I'm sorry, I love my Atari 2600) =D
  
int getread = 0;
getread = analogRead(up);
if (getread == 0) {
if (PaddlePos > 130)
PaddlePos--;
}
getread = analogRead(down);
if (getread == 0) {
if (PaddlePos < 148)
PaddlePos++;
}


run();
//  delay(100);

}

void AI() { // BALL MOVEMENT
  
  
  
if (Xpos+momentumX < 57 && momr == true) {
Xpos+=momentumX;
}
else if (Xpos+momentumX >= 57 && momr == true) {
Xpos-=momentumX;
momr = false;
}
else if (Xpos-momentumX > 0 && momr == false){
Xpos-=momentumX;
}
else if (Xpos-momentumX <= 0 && momr == false){
if (Ypos+1 > PaddlePos && Ypos+1 < PaddlePos+10) {
  Xpos+=momentumX;
  momr = true;
int oldY = momentumY;
scored();
if (Ypos < PaddlePos+3 || Ypos+2 > PaddlePos+7) {
if (momentumY < 4)
  momentumY +=2;
}
else {
if (momentumY > 1)
  momentumY -=1;
}


}
else
{
lost = true;
}
}


if (Ypos+momentumY < 156 && momd == true) {
Ypos+=momentumY;
}
else if (Ypos+momentumY >= 156 && momd == true) {
Ypos-=momentumY;
momd = false;
}
else if (Ypos-momentumY > 128 && momd == false){
Ypos-=momentumY;
}
else if (Ypos-momentumY <= 128 && momd == false){
Ypos+=momentumY;
  momd = true;
}


}

void run() {
if (lost == false) {
  AI();  
LCDA.DumpScreenBuffer(0,0,8,32);

// Draw Room
LCDA.FillRectangle(60,128,2,32); 
LCDA.FillRectangle(0,128,61,2); 
LCDA.FillRectangle(0,158,61,2); 

// Draw Ball
LCDA.FillRectangle(Xpos,Ypos,2,3); 
// Draw Paddle
LCDA.FillRectangle(1,PaddlePos,2,10); 

LCDA.RenderScreenBuffer(0,0,8,32); // Render the court ONLY (no need to render anything else, it will simply slow everything down)
}
}


void numbertobinary(char num) { // Convert the number to binary for the scores
if (num == 0)
LCDA.setPins(1,0,0,0,1,1,0,0,0,0); 
else if (num == 1)
LCDA.setPins(1,0,0,0,1,1,0,0,0,1); 
else if (num == 2)
LCDA.setPins(1,0,0,0,1,1,0,0,1,0); 
else if (num == 3)
LCDA.setPins(1,0,0,0,1,1,0,0,1,1); 
else if (num == 4)
LCDA.setPins(1,0,0,0,1,1,0,1,0,0); 
else if (num == 5)
LCDA.setPins(1,0,0,0,1,1,0,1,0,1); 
else if (num == 6)
LCDA.setPins(1,0,0,0,1,1,0,1,1,0); 
else if (num == 7)
LCDA.setPins(1,0,0,0,1,1,0,1,1,1); 
else if (num == 8)
LCDA.setPins(1,0,0,0,1,1,1,0,0,0); 
else if (num == 9)
LCDA.setPins(1,0,0,0,1,1,1,0,0,1); 
}

void Title() { // Write PONG (ASCII MODE)
  LCDA.Draw(false,8,20);
delay(1);

byte title[] = {0,1,0,1,0,0,0,0,0,1,1,0,1,1,1,1,0,1,1,0,1,1,1,0,0,1,1,0,0,1,1,1};

int count = 0;
int counter = 0;
while (count < sizeof(title)/sizeof(byte)) {
for (int i = count; i < count+8; i++) {
  LCDA.temp[counter] = title[i];
  counter++;
}
LCDA.setPins(1,0,LCDA.temp[0],LCDA.temp[1],LCDA.temp[2],LCDA.temp[3],LCDA.temp[4],LCDA.temp[5],LCDA.temp[6],LCDA.temp[7]); 

count += 8;
counter = 0;
}

LCDA.Draw(true,0,0);
}


void scored() { //  Whenever you hit the ball, the score incremements
LCDA.Draw(false,12,20);
delay(1);
score++;
momentumX = random(3, 6);



if (score < 10) {
numbertobinary('0');
numbertobinary('0');
numbertobinary(score);
}
else if (score < 100) {
int res = score/10;
int lef = score%10;
numbertobinary('0');
numbertobinary(res);
numbertobinary(lef);
}
else if (score < 1000) {
int res = score/100;
int lef = score%100;

  int res2 = lef/10;
int lef2 = lef%10;
numbertobinary(res);
numbertobinary(res2);
numbertobinary(lef2);
}
LCDA.Draw(true,0,0);
}
