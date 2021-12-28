/* GENERATOR CONTROL MODULE v0.1
Developed by: Tino Herljević
Date: 1.11.2015.
Last modification: 29.12.2021.

Basic controls, emergency switch, automatic and manual start.
TODO: automatic battery recharge, display, wireless controls, */
//includes
#include <SimpleTimer.h>


//devices
int starter = 13;
int fuel = 12;
int choke = 11;
int engine = 10;

//switches
int emergencySw = 8;
int autoSw = 9;
int manualSw = 10;
int oilSw = 11;
int mainsSw = 13;


//LED lights
int maintenanceLed = 14;
int errorLed = 2;
int runningLed = 3;
int oilLed = 4;
int autoLed = 5;
int manualLed = 6;
int beeper = 7;


boolean engineRunning = false;
boolean manualMode = false;
boolean automaticMode = false;
boolean maintenance = false;
boolean emerg = false;
boolean exercising = false;
boolean mains = true; // we assume that mains is available at the time of starting controller

SimpleTimer timer;

void setup() {

  Serial.begin (9600);
  timer.setInterval(6.048e+8, exercise);


  //defining pin outputs
  pinMode(starter, OUTPUT);
  pinMode(fuel, OUTPUT);
  pinMode(choke, OUTPUT);
  pinMode(engine, OUTPUT);

  pinMode(errorLed, OUTPUT);
  pinMode(runningLed, OUTPUT);
  pinMode(oilLed, OUTPUT);
  pinMode(autoLed, OUTPUT);
  pinMode(manualLed, OUTPUT);
  pinMode(beeper, OUTPUT);

  pinMode(autoSw, INPUT);
  pinMode(manualSw, INPUT);
  pinMode(emergencySw, INPUT);
  pinMode(oilSw, INPUT);
  pinMode(mainsSw, INPUT);

  //setting pin states
  starter = LOW;
  fuel = LOW;
  choke = LOW;
  engine = LOW;

  errorLed = LOW;
  autoLed = LOW;
  manualLed = LOW;
  beeper = LOW;
  maintenanceLed = LOW;
}



void loop() {
  timer.run();

  /*When EMERGENCY switch is activated,
   * all functions generator fucntions are stopped + warning led
   * is blinking. If user puts switch to NO mode, all functions are restored. */
  if (digitalRead(emergencySw) == HIGH) {
    emergency();
  }

  /*maintenance mode*/
  if ((digitalRead(manualSw) == LOW) && (digitalRead(autoSw) == LOW) && (digitalRead(emergencySw) == LOW)) {
    maintenance == true;
    confirmationBeep();
    digitalWrite(maintenanceLed, HIGH);
  } else maintenance == false;
  digitalWrite(maintenanceLed, LOW);


  /*When in manualStartStart MODE, engine will start itself and will run until user
       exits the manualStart mode. This can be used for engine and generator testing. */
  if (digitalRead(manualSw) == HIGH) {
    manualStart();
  }

  if (digitalRead(autoSw) == HIGH) {
    automaticStart();
  }

  if ((engineRunning == true) && (digitalRead(oilSw) == HIGH)) {
    lowOil();
  }

  else if (engineRunning == true) {
    stopEngine();
    engineRunning = false;
  }
}

/*PROCEDURES */


/*engine start procedure*/
void startEngine() {
  digitalWrite(engine, HIGH);
  digitalWrite(fuel, HIGH);
  delay(1500);
  digitalWrite(choke, HIGH);
  digitalWrite(starter, HIGH);
  delay (3000);
  digitalWrite(starter, LOW);
  checkEmergency();
  delay(1000);
  digitalWrite(choke, LOW);
  engineRunning = true;
  checkEmergency();

}

/*engine stopping procedure*/
void stopEngine() {
  Serial.println("Stopping engine");
  digitalWrite(engine, LOW);
  digitalWrite(fuel, LOW);
  digitalWrite(starter, LOW);
  digitalWrite(choke, LOW);
  engineRunning = false;
  Serial.println("Engine shut down");
}

void emergency() {

  stopEngine();

  do {
    if (emergencySw == HIGH) {
      emerg = true;
    }
    else emerg = false;
    digitalWrite(errorLed, HIGH);
    errorBeep();
    digitalWrite(errorLed, LOW);
    delay(1000);
  }
  while (emerg == true);
}


void checkEmergency() {
  if (digitalRead(emergencySw) == HIGH) {
    emerg = true;
    emergency();
  }
}

/*manual starting the generator*/
void manualStart() {
  if ((engineRunning == false) && (manualMode == false) && (digitalRead(manualSw) == HIGH) && (digitalRead(emergencySw) == LOW)) {
    confirmationBeep();
    digitalWrite(manualLed, HIGH);
    manualMode = true;
    startEngine();
    do {
      if (digitalRead(autoSw) == LOW && digitalRead(manualSw) == LOW && digitalRead(emergencySw) == LOW) {
        maintenance = true;
        manualMode = false;
      }
      if (digitalRead(emergencySw) == HIGH) {
        emerg = true;
      }
    }
    while (maintenance == false || emerg == false || manualMode == true);
    stopEngine();
    digitalWrite(manualLed, LOW);
  }
}

/*automatic starting generator based on mains availability*/
void automaticStart() {
  if ((engineRunning == false) && (automaticMode == false) && (digitalRead(manualSw) == HIGH) && (digitalRead(emergencySw) == LOW)) {
    confirmationBeep();
    digitalWrite(autoLed, HIGH);
    automaticMode = true;
  }
}




/*beep sounds*/
void errorBeep() {
  for (int i = 0; i < 1000; i++) {
    digitalWrite(beeper, HIGH);
    delayMicroseconds(250);
    digitalWrite(beeper, LOW);
    delayMicroseconds(250);
  }
}

void confirmationBeep() {
  for (int i = 0; i < 2; i++) {
    for (int i = 0; i < 500; i++) {
      digitalWrite(beeper, HIGH);
      delayMicroseconds(250);
      digitalWrite(beeper, LOW);
      delayMicroseconds(250);
    }
    delay(500);
  }

}



/*part of code where controller checks for low oil pressure. If pressure is low,
*it will turn off the generator and sound the alarm.*/
void checkOilPressure() {
  if (oilSw == HIGH) {
    lowOil();
  }
}

void lowOil() {
  stopEngine();
  do {
    if ((manualSw == LOW) && (autoSw == LOW) && (emergencySw == LOW)) {
      maintenance == true;
    }
    digitalWrite(oilLed, HIGH);
    errorBeep();
    digitalWrite(oilLed, LOW);
    delay(1000);
  }
  while (maintenance == false);

}





/* this code will start exercise run every 7 days, since the controller has been powered on.
 *  Test run will only be performed if controller is set to automatic mode.
 *  Power from generator will not be sent to the mains (relay remains open).
 */
void exercise() {
  if (engineRunning == false && maintenance == false && digitalRead(autoSw) == HIGH && digitalRead(manualSw) == LOW && digitalRead(emergencySw) == LOW) {
    startEngine();
    do {
      if (digitalRead(mainsSw) == LOW) {
        mains = false;
      }
      if (digitalRead(autoSw) == LOW && digitalRead(manualSw) == LOW && digitalRead(emergencySw) == LOW) {
        maintenance = true;
      }


    }
    while (mains == false || emerg == false || maintenance == false);
    stopEngine();
  }
}
