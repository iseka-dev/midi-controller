#include <MIDI.h>

MIDI_CREATE_DEFAULT_INSTANCE();

int pot1 = A0;
int pot1Old = 0;
int pot1Delta = pot1-pot1Old;

int fader1 = A1;
int fader1Old = 0;
int fader1Delta = fader1-fader1Old;

void setup() {

  pinMode(pot1, INPUT);
  pinMode(pot1, INPUT);
  pinMode(fader1, INPUT);
 
  //  Set MIDI baud rate:
  MIDI.begin(MIDI_CHANNEL_OMNI);
}
 
void loop() {
  
  pot1 = analogRead(A0);
  fader1 = analogRead(A1);

  if (abs(pot1Delta) >= 3){
    MIDI.sendControlChange(102, round((pot1)/8), 1);
    pot1Old = pot1;
    delay(1);
  }
  if (abs(fader1Delta) >= 3){
    MIDI.sendControlChange(103, round(fader1)/8, 1);
    fader1Old = fader1;
    delay(1);
  }
  else{
  }
}
