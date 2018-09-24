//
// Auto-start diesel module
// 20171117-1059 Open Source
//
// CAN BE FREELY DISTRIBUTED AND MODIFIED AS LONG AS
// ORIGINAL CREDIT IS INCLUDED AND UNALTERED.
// MAY NOT BE SOLD IN PART OR COMPLETE.
//
// (C)Odd-Jarle Kristoffersen
// https://github.com/amuso/dieselautoglow
// https://www.youtube.com/user/amuso750
//
// Automatic wait for glowing to complete before starter is engaged.
// Program offers a serial console for debugging.
//
// Schematics and details : https://youtu.be/4HXyowKtkj4
//
// Program starts up when ignition is active (position 2 powers device via 7805)
// and blink the status LED fast (5x per second).
//
// When "start" signal is received from ignition (position 3 starter signal)
// we'll blink the LED slowly (1x per second) and wait until glowing finish.
//
// Send start signal to external relay for up to "runfor" ms, or until engine
// RPM is above "MIN_RPM" (and below 2000 RPM). Keep status LED lit solid while
// starter signal is active.
//
// Requires 3 digital input, where 1 supports interrupts.
// Requires 2 digital outputs (1 for status LED).
//

// Set values for vehicle

unsigned long runFor = 10000;   // Max starter run time (10000 ms / 10 s)

char modelType[ ] = "Ford Excursion 7.3";
bool START = HIGH;              // HIGH (+B) = start signal received
bool GLOW = LOW;                // LOW (GND) = glowing in progress
int MIN_RPM = 450;              // Minimum idle RPM for engine running
int rpmTicks = 4;               // 4 pulses per revolution of the crank
                                // 60 divided by this value MUST be an integer !

// char modelType[ ] = "Range Rover 2.5 DSE";
// bool START = HIGH;              // HIGH (+B) = start signal received
// bool GLOW = LOW;                // LOW (GND) = glowing in progress
// int MIN_RPM = 450;              // Minimum idle RPM for engine running
// int rpmTicks = 4;               // 4 pulses per revolution of the crank
                                   // 60 divided by this value MUST be an integer !

// End user configuration

int startin = 2;
int enginein = 3;
int glowin = 4;
int startout = 7;
int led = 13;
unsigned long startedAt = 0;
int rpmTimer = 500;
int rpmCounter = 0;
int rpm = 0;
unsigned long previousTime = 0;
int ledStatus = HIGH;

void setup() {
  pinMode(startin, INPUT);
  pinMode(enginein, INPUT);
  pinMode(glowin, INPUT);
  pinMode(startout, OUTPUT);
  digitalWrite(startout, LOW);
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);
  Serial.begin(9600);
  Serial.print("Auto-Start Diesel Module\nModel: ");
  Serial.println(modelType);
  Serial.print("Firmware: 20171117-1059\n= = DEBUGGING CONSOLE ACTIVE = =\nWaiting for start signal... ");
}

void loop() {
  digitalWrite(led, ledStatus);
  if (digitalRead(startin) == START) {
    Serial.println("Start received.");
    waitGlow();
    attemptStart();
    endProgram();
  }
  delay(100);
  ledStatus = !ledStatus;
}

void waitGlow() {
  Serial.print("Waiting for glow procedure to complete... ");
  while (digitalRead(glowin) == GLOW) {
    digitalWrite(led, ledStatus);
    delay(500);
    ledStatus = !ledStatus;
  }
  Serial.println("Ready to start.");
  digitalWrite(led, HIGH);
}

void attemptStart() {
  delay(500);
  if (digitalRead(glowin) != GLOW) {
    Serial.println("Glow has completed.");
    attachInterrupt(1, countPulses, FALLING);
    startedAt = previousTime = millis();
    Serial.print("Starter enabled: ");
    Serial.println(startedAt);
    while (millis() - startedAt < runFor) {
      digitalWrite(startout, HIGH);
      if (millis() - previousTime == rpmTimer) {
        detachInterrupt(1);
        rpm = rpmCounter * (1000 / rpmTimer) * (60 / rpmTicks);
        Serial.print("Engine RPM: ");
        Serial.println(rpm);
        if (rpm > 2000) {
          Serial.println("WARNING: High RPM probably mis-reading so ignoring it.");
          rpmCounter = 0;
          previousTime = millis();
          attachInterrupt(1, countPulses, FALLING);
        } else if (rpm > MIN_RPM) {
          digitalWrite(startout, LOW);
          break;
        } else {
          rpmCounter = 0;
          previousTime = millis();
          attachInterrupt(1, countPulses, FALLING);
        }
      }
    }
    startedAt = millis();
    digitalWrite(startout, LOW);
    Serial.print("Disengaged starter: ");
    Serial.println(startedAt);
    endProgram();
  } else {
    Serial.println("ERROR: Still glowing. Terminating start!");
    endProgram();
  }
}

void countPulses() {
  rpmCounter++;
}

void endProgram() {
  digitalWrite(led, LOW);
  Serial.println("The end.");
  while ( 1 == 1 ) { delay(1000); }
}


