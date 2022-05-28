#include <MIDI.h>
#include <Thread.h> // Threads library (by Ivan seidel) >> https://github.com/ivanseidel/ArduinoThread
#include <ThreadController.h>   

MIDI_CREATE_DEFAULT_INSTANCE();

// potentiometers
const byte nPots = 2; // *coloque aqui o numero de entradas digitais utilizadas no multiplexer
const byte potPins[nPots] = {A0, A1};
int potCurrentState[nPots] = {0}; // estado atual da porta analogica
int potPrevState[nPots] = {0}; // estado previo da porta analogica
int potVar = 0; // variacion entre estados previo y actual
int lastCcValue[nPots] = {0};

// pot reading
int TIMEOUT = 50; //tiempo de lectura del pot
byte varThreshold = 8; // umbral para variacion de la señal del potenciómetro
boolean potMoving = true;
unsigned long prevTime[nPots] = {0}; // tempo almacenado anteriormente
unsigned long timer[nPots] = {0}; // tiempo que paso desde ultima medicion
byte cc = 1;

// buttons
const byte nButs = 1;
const byte butPins[nButs] = {A2};
int butCurrentState[nButs] = {0};
int butPrevState[nButs] = {0};

unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 1;    // the debounce time; increase if the output flickers

byte butCc = 1;

ThreadController cpu; //thread master
Thread threadReadPots; // thread para controlar los pots
Thread threadReadButtons; // thread para controlar botones

void setup() {

  pinMode(2, INPUT_PULLUP);

  MIDI.begin(MIDI_CHANNEL_OMNI);
  // MIDI.begin(31250);

  pinMode(2, INPUT_PULLUP);
  
  threadReadPots.setInterval(10);
  threadReadPots.onRun(readPots);
  cpu.add(&threadReadPots);

  threadReadButtons.setInterval(20);
  threadReadButtons.onRun(readButtons);
  cpu.add(&threadReadButtons);

}

void loop() {

  cpu.run();
  MIDI.read();


}

void readPots() {
  for (int i = 0; i < nPots; i++) { // le todas entradas analogicas utilizadas, menos a dedicada a troca do canal midi
    potCurrentState[i] = analogRead(i+0);
  }
  for (int i = 0; i < nPots; i++) {
    potVar = abs(potCurrentState[i] - potPrevState[i]); // calcula a variacao da porta analogica
    if (potVar >= varThreshold) {  //sets a threshold for the variance in the pot state, if it varies more than x it sends the cc message
      prevTime[i] = millis(); // armazena o tempo previo
    }
    timer[i] = millis() - prevTime[i]; // reseta o timer
    if (timer[i] < TIMEOUT) { // se o timer for menor que o tempo maximo permitido significa que o potenciometro ainda esta se movendo
      potMoving = true;
    }
    else {
      potMoving = false;
    }
    if (potMoving == true) { // se o potenciometro ainda esta se movendo, mande o control change
      int ccValue = map(potCurrentState[i], 0, 1023, 0, 127);
      if (lastCcValue[i] != ccValue) {
        // MidiUSB.flush();
        MIDI.sendControlChange(cc + i, map(potCurrentState[i], 0, 1023, 0, 127), 11); // envia Control Change (numero do CC, valor do CC, canal midi)
        //Serial.print("CC: "); Serial.print(cc + i); Serial.print(" value:"); Serial.println(map(potCState[i], 0, 1023, 0, 127));
        potPrevState[i] = potCurrentState[i]; // armazena a leitura atual do potenciometro para comparar com a proxima
        lastCcValue[i] = ccValue;
      }
    }
  }
}

void readButtons() {
  for (int i = 0; i < nButs; i++) { //reads buttons
    int buttonReading = digitalRead(i+2);
    if (buttonReading > 100) {
      butCurrentState[i] = HIGH;
    }
    else {
      butCurrentState[i] = LOW;
    }
  }

  int flag = 0;
  for (int i = 0; i < nButs; i++) {
    if ((millis() - lastDebounceTime) > debounceDelay) {
      if (butCurrentState[i] != butPrevState[i]) {
        lastDebounceTime = millis();
        if (butCurrentState[i] == HIGH) {
          if (flag == 0) {
            MIDI.sendControlChange(butCc + i, 127, 10); // envia Control Change (numero do CC, valor do CC, canal midi)
            flag = 127;
          }
          else {
            MIDI.sendControlChange(butCc + i, 0, 10); // envia Control Change (numero do CC, valor do CC, canal midi)
            flag = 0;
          }
        }
        butPrevState[i] = butCurrentState[i];
      }
    }
  }
 
}
