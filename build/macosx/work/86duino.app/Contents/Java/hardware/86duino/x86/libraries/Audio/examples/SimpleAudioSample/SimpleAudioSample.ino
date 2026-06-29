#include "Audio.h"
#include "AudioSample.h"

AudioSample *audio;

void setup() {
  Audio.begin(22050, 100);
  Audio.prepare(NULL, 1024, 8);
  audio = new AudioSample();
  audio->load("C:/Faded.mp3");
}

void loop() {
  audio->play();
  Audio.update();
}