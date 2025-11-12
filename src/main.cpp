/*
 * Morse Code Receiver (No Display)
 * Reads Morse key input and prints decoded characters via Serial
 */
#include <Arduino.h>
#include <TimerOne.h>
#include <Adafruit_SSD1306.h>
// === Pin and timing variables ===
const byte inputPin = 9;       // Analog input from telegraph key or radio output

int signalOnTime = 0;           // Duration key is pressed (in ms)
int signalOffTime = 0;          // Duration key is released (in ms)
int lastPressDuration = 0;      // Last press duration
int lastPauseDuration = 0;      // Last pause duration
int dotDuration = 0;            // Base time for one dot
int wordsPerMinute = 0;         // Estimated speed (WPM)

// === Morse decoding ===
String currentSymbol = "";      // "." or "-"
String currentMorse = "";       // Full Morse sequence for one character
String decodedText = "";        // Accumulated decoded output

// === Morse code lookup table ===
const byte numSymbols = 48;

const char* letters[numSymbols] = {
  "a","b","c","d","e","f","g","h","i","j","k","l","m",
  "n","o","p","q","r","s","t","u","v","w","x","y","z",
  "0","1","2","3","4","5","6","7","8","9",
  "!","(",")","+",",","-",".","/","=","?","_"
};

const String morse[numSymbols] = {
  ".-","-...","-.-.","-..",".","..-.","--.","....","..",".---","-.-",".-..","--",
  "-.","---",".--.","--.-",".-.","...","-","..-","...-",".--","-..-","-.--","--..",
  "-----",".----","..---","...--","....-",".....","-....","--...","---..","----.",
  "-.-.--","-.--.","-.--.-",".-.-.","--..--","-....-",".-.-.-","-..-.","-...-","..--..","..--.-"
};

void setup() {
  pinMode(inputPin, INPUT_PULLUP);
  
  Serial.begin(9600);
  Timer1.initialize(1000);        // 1ms interval
  Timer1.attachInterrupt(trackSignal); // Runs every millisecond
  Serial.println("Morse Decoder Ready...");
}

void loop() {
  // The decoding runs in the timer interrupt.
  // We just print when a character or word is completed.
}

// === Interrupt Service Routine (ISR) ===
void trackSignal() {
  int inputValue = digitalRead(inputPin);

  // --- Key pressed ---
  if (inputValue > 10) {
    signalOnTime++;
    if (signalOnTime == 1) {
      lastPauseDuration = signalOffTime;
      signalOffTime = 0;
    }
  }

  // --- Key released ---
  if (inputValue <= 10) {
    signalOffTime++;
    if (signalOffTime == 1) {
      lastPressDuration = signalOnTime;

      // Establish dot duration if not set yet
      if (dotDuration == 0) {
        dotDuration = lastPressDuration;
      }

      // Determine if dot or dash
      if (lastPressDuration <= 1.5 * dotDuration) {
        currentSymbol = ".";
        dotDuration = lastPressDuration; // refine dot duration
      } else if (lastPressDuration > 2 * dotDuration) {
        currentSymbol = "-";
      }

      // Add to current Morse character
      if (lastPauseDuration < 2 * dotDuration) {
        currentMorse += currentSymbol;
      }

      signalOnTime = 0;
    }

    // --- End of character ---
    if (signalOffTime == 2 * dotDuration) {
      wordsPerMinute = 1200 / dotDuration;
      if (wordsPerMinute > 90) {
        wordsPerMinute = 0;
        signalOnTime = 0;
      }

      if (currentMorse.length() > 0) {
        String decodedChar = decodeMorse(currentMorse);
        Serial.print(decodedChar);
        decodedText += decodedChar;
        currentMorse = "";
      }
    }

    // --- Long pause: new word ---
    if (signalOffTime == 100 * dotDuration && decodedText.length() > 0) {
      Serial.print(" ");
      decodedText += " ";
    }
  }
}

// === Morse lookup ===
String decodeMorse(String code) {
  for (int i = 0; i < numSymbols; i++) {
    if (morse[i] == code) {
      return String(letters[i]);
    }
  }
  return "?"; // Unknown symbol
}
