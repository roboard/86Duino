/*
  Continue to read A/B pulse by using encoder 0 (PIN 42 43 44). 
  You can also use attachInterrupt() to run ISR when occur certain condition
  (ex. index signal or internal counter is equal to setComparator())
*/

#include <EncoderEX.h>
 
volatile unsigned long num1 = 0L;
volatile unsigned long num2 = 0L;
volatile unsigned long num3 = 0L;
volatile unsigned long num4 = 0L;
volatile unsigned long num5 = 0L;
volatile unsigned long num6 = 0L;
volatile unsigned long num7 = 0L;
volatile unsigned long num8 = 0L;
volatile unsigned long num9 = 0L;
void test0(int flag) { // ISR
  num1++;
  if(flag == INTR_INDEX) {
    num2 = Enc0.readIDXCount(); // if motor rotate one circle, num2 add one
  }
}

void test1(int flag) {
  if(flag == INTR_INDEX) {
    num3 = Enc1.readIDXCount(); // if motor rotate one circle, num2 add one
  }
}

void test2(int flag) {
  if(flag == INTR_INDEX) {
    num4 = Enc2.readIDXCount(); // if motor rotate one circle, num2 add one
  }
}

void test3(int flag) {
  if(flag == INTR_INDEX) {
    num5 = Enc3.readIDXCount(); // if motor rotate one circle, num2 add one
  }
}

void test4(int flag) {
  if(flag == INTR_INDEX) {
    num6 = Enc4.readIDXCount(); // if motor rotate one circle, num2 add one
  }
}

void test5(int flag) {
  if(flag == INTR_INDEX) {
    num7 = Enc5.readIDXCount(); // if motor rotate one circle, num2 add one
  }
}

void test6(int flag) {
  if(flag == INTR_INDEX) {
    num8 = Enc6.readIDXCount(); // if motor rotate one circle, num2 add one
  }
}

void test7(int flag) {
  if(flag == INTR_INDEX) {
    num9 = Enc7.readIDXCount(); // if motor rotate one circle, num2 add one
  }
}
 
void setup() {
  Serial.begin(9600);
  Enc0.begin(MODE_AB_PHASE);
  Enc0.setIndexReset(MCENC_PAB_IDXCOND_ABZ, 1, 0, 1);    // when A, B, Z = 1, 0, 1 => trigger interrupt
  Enc0.attachInterrupt(test0);
  Enc1.begin(MODE_AB_PHASE);
  Enc1.setIndexReset(MCENC_PAB_IDXCOND_ABZ, 1, 0, 1);    // when A, B, Z = 1, 0, 1 => trigger interrupt
  Enc1.attachInterrupt(test1);
  Enc2.begin(MODE_AB_PHASE);
  Enc2.setIndexReset(MCENC_PAB_IDXCOND_ABZ, 1, 0, 1);    // when A, B, Z = 1, 0, 1 => trigger interrupt
  Enc2.attachInterrupt(test2);
  Enc3.begin(MODE_AB_PHASE);
  Enc3.setIndexReset(MCENC_PAB_IDXCOND_ABZ, 1, 0, 1);    // when A, B, Z = 1, 0, 1 => trigger interrupt
  Enc3.attachInterrupt(test3);
  Enc4.begin(MODE_AB_PHASE);
  Enc4.setIndexReset(MCENC_PAB_IDXCOND_ABZ, 1, 0, 1);    // when A, B, Z = 1, 0, 1 => trigger interrupt
  Enc4.attachInterrupt(test4);
  Enc5.begin(MODE_AB_PHASE);
  Enc5.setIndexReset(MCENC_PAB_IDXCOND_ABZ, 1, 0, 1);    // when A, B, Z = 1, 0, 1 => trigger interrupt
  Enc5.attachInterrupt(test5);
  Enc6.begin(MODE_AB_PHASE);
  Enc6.setIndexReset(MCENC_PAB_IDXCOND_ABZ, 1, 0, 1);    // when A, B, Z = 1, 0, 1 => trigger interrupt
  Enc6.attachInterrupt(test6);
  Enc7.begin(MODE_AB_PHASE);
  Enc7.setIndexReset(MCENC_PAB_IDXCOND_ABZ, 1, 0, 1);    // when A, B, Z = 1, 0, 1 => trigger interrupt
  Enc7.attachInterrupt(test7);
}
 
void loop() {
  Serial.print("Pulse number: ");
  Serial.print(Enc0.read()); // read pulse counter
  Serial.print("    ");
  Serial.print("num1: ");
  Serial.print(num1);
  Serial.print("    ");
  Serial.print("num2: ");
  Serial.print(num2);
  Serial.print("    ");
  Serial.print("num3: ");
  Serial.print(num3);
  Serial.print("    ");
  Serial.print("num4: ");
  Serial.print(num4);
  Serial.print("    ");
  Serial.print("num5: ");
  Serial.print(num5);
  Serial.print("    ");
  Serial.print("num6: ");
  Serial.print(num6);
  Serial.print("    ");
  Serial.print("num7: ");
  Serial.print(num7);
  Serial.print("    ");
  Serial.print("num8: ");
  Serial.print(num8);
  Serial.print("    ");
  Serial.print("num9: ");
  Serial.println(num9);
  delay(100);
}