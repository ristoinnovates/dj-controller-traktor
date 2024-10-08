#include <Arduino.h>
#include "MIDIUSB.h"
#include <Wire.h>
#include "MCP23017Encoders.h"
#include <ResponsiveAnalogRead.h>

int startingCCPots = 1;      // Starting CC for pots
int startingCCButtons = 50;  // Starting CC for buttons
int startingCCEncoders = 100; // Starting CC for encoders




int muxChannel1 = 4;
int muxChannel2 = 5;
int muxChannel3 = 6;
int muxChannel4 = 8;

int muxSwitchesInput1 = 9; // 16 channel digital
int muxSwitchesInput2 = 10; // 16 channel digital
int muxSwitchesInput3 = 16; // 16 channel digital


int muxPotsInput1 = A0; // 16 channel analog
int muxPotsInput2 = A1; // 8 channel analog

// MULTIPLEXER 1 BUTTONS - 16 CH
const int NUMBER_MUX_1_BUTTONS = 16;
bool muxButtons1CurrentState[NUMBER_MUX_1_BUTTONS] = {0};
bool muxButtons1PreviousState[NUMBER_MUX_1_BUTTONS] = {0};

unsigned long lastDebounceTimeMUX1[NUMBER_MUX_1_BUTTONS] = {0};
unsigned long debounceDelayMUX1 = 20;

// MULTIPLEXER 2 BUTTONS - 16 CH
const int NUMBER_MUX_2_BUTTONS = 16;
bool muxButtons2CurrentState[NUMBER_MUX_2_BUTTONS] = {0};
bool muxButtons2PreviousState[NUMBER_MUX_2_BUTTONS] = {0};

unsigned long lastDebounceTimeMUX2[NUMBER_MUX_2_BUTTONS] = {0};
unsigned long debounceDelayMUX2 = 20;

// MULTIPLEXER 3 BUTTONS - 16 CH
const int NUMBER_MUX_3_BUTTONS = 16;
bool muxButtons3CurrentState[NUMBER_MUX_3_BUTTONS] = {0};
bool muxButtons3PreviousState[NUMBER_MUX_3_BUTTONS] = {0};

unsigned long lastDebounceTimeMUX3[NUMBER_MUX_3_BUTTONS] = {0};
unsigned long debounceDelayMUX3 = 20;


// MULTIPLEXER 1 POTS - 16 CH
const int NUMBER_MUX_1_POTS = 16;
int muxPots1CurrentValue[NUMBER_MUX_1_POTS] = {0};
int muxPots1PreviousValue[NUMBER_MUX_1_POTS] = {0};

ResponsiveAnalogRead potsMux1[NUMBER_MUX_1_POTS] = {
  ResponsiveAnalogRead(A0, true), 
  ResponsiveAnalogRead(A0, true), 
  ResponsiveAnalogRead(A0, true), 
  ResponsiveAnalogRead(A0, true), 
  ResponsiveAnalogRead(A0, true), 
  ResponsiveAnalogRead(A0, true), 
  ResponsiveAnalogRead(A0, true), 
  ResponsiveAnalogRead(A0, true), 
  ResponsiveAnalogRead(A0, true), 
  ResponsiveAnalogRead(A0, true), 
  ResponsiveAnalogRead(A0, true), 
  ResponsiveAnalogRead(A0, true), 
  ResponsiveAnalogRead(A0, true), 
  ResponsiveAnalogRead(A0, true), 
  ResponsiveAnalogRead(A0, true), 
  ResponsiveAnalogRead(A0, true)
};


// MULTIPLEXER 2 POTS - 8 CH
const int NUMBER_MUX_2_POTS = 7;
int muxPots2CurrentValue[NUMBER_MUX_2_POTS] = {0};
int muxPots2PreviousValue[NUMBER_MUX_2_POTS] = {0};



ResponsiveAnalogRead potsMux2[NUMBER_MUX_2_POTS] = {
  ResponsiveAnalogRead(A1, true), 
  ResponsiveAnalogRead(A1, true), 
  ResponsiveAnalogRead(A1, true),
  ResponsiveAnalogRead(A1, true), 
  ResponsiveAnalogRead(A1, true), 
  ResponsiveAnalogRead(A1, true), 
  ResponsiveAnalogRead(A1, true),
};




// Encoders
const int numberOfEncoders = 8;
MCP23017Encoders myMCP23017Encoders(7); // Parameter is the Arduino interrupt pin that the MCP23017 is connected to

int previousValues[numberOfEncoders] = {0}; // Array to store previous values of each encoder

void setup()
{
  Serial.begin(9600);

  pinMode(muxChannel1, OUTPUT);
  pinMode(muxChannel2, OUTPUT);
  pinMode(muxChannel3, OUTPUT);
  pinMode(muxChannel4, OUTPUT);
  digitalWrite(muxChannel1, LOW);
  digitalWrite(muxChannel2, LOW);
  digitalWrite(muxChannel3, LOW);
  digitalWrite(muxChannel4, LOW);

  pinMode(muxSwitchesInput1, INPUT_PULLUP); // Digital input for the first set of switches
  pinMode(muxSwitchesInput2, INPUT_PULLUP); // Digital input for the second set of switches
  pinMode(muxSwitchesInput3, INPUT_PULLUP); // Digital input for the second set of switches

  pinMode(muxPotsInput1, INPUT); // Analog input for the first set of pots
  pinMode(muxPotsInput2, INPUT); // Analog input for the second set of pots

  myMCP23017Encoders.begin();
  myMCP23017Encoders.setAccel(0, 100);
}

void loop()
{
  updateEncoders();
  updateMUXButtons(muxSwitchesInput1, NUMBER_MUX_1_BUTTONS, muxButtons1CurrentState, muxButtons1PreviousState, lastDebounceTimeMUX1, debounceDelayMUX1, 1);
  updateMUXButtons(muxSwitchesInput2, NUMBER_MUX_2_BUTTONS, muxButtons2CurrentState, muxButtons2PreviousState, lastDebounceTimeMUX2, debounceDelayMUX2, 2);
  updateMUXButtons(muxSwitchesInput3, NUMBER_MUX_3_BUTTONS, muxButtons3CurrentState, muxButtons3PreviousState, lastDebounceTimeMUX3, debounceDelayMUX3, 3);

  updateMUXPots(muxPotsInput1, NUMBER_MUX_1_POTS, muxPots1CurrentValue, muxPots1PreviousValue, 1);
  updateMUXPots(muxPotsInput2, NUMBER_MUX_2_POTS, muxPots2CurrentValue, muxPots2PreviousValue, 2);

  
}

void updateEncoders() {
  for (int i = 0; i < numberOfEncoders; i++)
  {
    int currentValue = myMCP23017Encoders.read(i);

    if (currentValue != previousValues[i])
    {
      Serial.print("Encoder ");
      Serial.print(i);
      Serial.print(": ");
      Serial.println(currentValue);
      byte ccNumber = startingCCEncoders + i;  // Use the encoder index to assign a CC number
      int delta = currentValue - previousValues[i];
      byte ccValue = (delta > 0) ? 1 : 127;  // Send 1 for clockwise, 127 for counterclockwise

      midiControlChange(0, ccNumber, ccValue);  // Send MIDI CC message
      
      previousValues[i] = currentValue;
    }
  }
}

void updateMUXButtons(int muxInputPin, int numberOfButtons, bool *currentState, bool *previousState, unsigned long *lastDebounceTime, unsigned long debounceDelay, int muxNumber) {
  for (int i = 0; i < numberOfButtons; i++) {
    int A = (i >> 0) & 1;  // Extract the least significant bit
    int B = (i >> 1) & 1;  // Extract the second bit
    int C = (i >> 2) & 1;  // Extract the third bit
    int D = (i >> 3) & 1;  // Extract the most significant bit
    
    digitalWrite(muxChannel1, A);  // A corresponds to the least significant bit
    digitalWrite(muxChannel2, B);  // B corresponds to the second bit
    digitalWrite(muxChannel3, C);  // C corresponds to the third bit
    digitalWrite(muxChannel4, D);  // D corresponds to the most significant bit
    
    //delay(1);  // Add a longer delay for debugging

    currentState[i] = digitalRead(muxInputPin);

    if ((millis() - lastDebounceTime[i]) > debounceDelay) {
      if (currentState[i] != previousState[i]) {
        lastDebounceTime[i] = millis();

        if (currentState[i] == LOW) {
          Serial.print("MUX: ");
          Serial.print(muxNumber);
          Serial.print(", Button: ");
          Serial.println(i);
          pressButton(muxNumber, i, 1);
        } else {
          pressButton(muxNumber, i, 0);
        }

        previousState[i] = currentState[i];
      }
    }
  }
}

void pressButton(int muxNumber, int buttonNumber, int state) {
  byte ccNumber = startingCCButtons + (muxNumber - 1) * NUMBER_MUX_1_BUTTONS + buttonNumber;
  byte ccValue = (state == 1) ? 127 : 0;  // Send 127 for button press, 0 for release

  midiControlChange(0, ccNumber, ccValue);  // Send MIDI CC message
}



void updateMUXPots(int muxInputPin, int numberOfPots, int *currentValue, int *previousValue, int muxNumber) {
  for (int i = 0; i < numberOfPots; i++) {
    int A = (i >> 0) & 1;
    int B = (i >> 1) & 1;
    int C = (i >> 2) & 1;
    int D = (i >> 3) & 1;

    // Set the channel selection for the multiplexer
    digitalWrite(muxChannel1, A);
    digitalWrite(muxChannel2, B);
    digitalWrite(muxChannel3, C);

    if (muxNumber == 1) { // 16-channel multiplexer
      digitalWrite(muxChannel4, D);  // D is only used for the 16-channel mux
    }

    //delay(1);  // Small delay for stable channel switching

    if (muxNumber == 1) {
      potsMux1[i].update();  // Update the analog value reading for Mux 1
      currentValue[i] = potsMux1[i].getValue();  // Handle responsive analog read
    } else if (muxNumber == 2) {
      potsMux2[i].update();
      currentValue[i] = potsMux2[i].getValue();
    }

    if (currentValue[i] != previousValue[i]) {
      Serial.print("MUX: ");
      Serial.print(muxNumber);
      Serial.print(", Pot: ");
      Serial.print(i);
      Serial.print(" Value: ");
      Serial.println(currentValue[i]);
      handlePots(muxNumber, i, currentValue[i]);
      previousValue[i] = currentValue[i];  // Update the previous value
    }
  }
}



void handlePots(int muxNumber, int potNumber, int value) {
  byte ccNumber = startingCCPots;

  // Dynamically calculate the CC number based on the multiplexer
  if (muxNumber == 1) {
    ccNumber += potNumber;  // MUX 1 pots
  } else if (muxNumber == 2) {
    ccNumber += NUMBER_MUX_1_POTS + potNumber;  // MUX 2 starts after MUX 1's pots
  }

  byte ccValue = map(value, 0, 1023, 0, 127);  // Map analog value (0-1023) to MIDI CC value (0-127)
  
  midiControlChange(0, ccNumber, ccValue);  // Send MIDI CC message
}




void midiNoteOn(byte channel, unsigned short note) {
  midiEventPacket_t midiNoteOn = { 0x09, 0x90 | channel, note, 100 };
  MidiUSB.sendMIDI(midiNoteOn);
  MidiUSB.flush();  // Ensure the message is sent immediately
}

void midiNoteOff(byte channel, unsigned short note) {
  midiEventPacket_t midiNoteOff = { 0x08, 0x80 | channel, note, 0 };
  MidiUSB.sendMIDI(midiNoteOff);
  MidiUSB.flush();  // Ensure the message is sent immediately
}

void midiControlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = { 0x0B, 0xB0 | channel, control, value };
  MidiUSB.sendMIDI(event);
  MidiUSB.flush();  // Ensure the message is sent immediately
}
