#include <AndroidAccessory.h>

#define relay_1 2
#define relay_2 3
#define relay_3 4
#define relay_4 5
#define relay_5 6
#define relay_6 7
#define relay_7 8
#define relay_8 9

#define UNLOCK relay_1
#define LOCK relay_2
#define HORN relay_3
#define CLUCHSENSOR relay_4
#define TRUNK relay_5
#define STARTER relay_6
#define IGNITION1 relay_7
#define IGNITION2 relay_8



char accessoryName[] = "DemoKit"; // your Arduino board
char companyName[] = "Google";

// counters
long timer = millis();

// initialize the accessory:
AndroidAccessory usb(companyName, accessoryName);
typedef enum {
  START_CAR = 1,
  STOP_CAR = 2,
  START_HONKING = 3,
  STOP_HONKING = 4,
  SET_SILENCE = 5,
  UNSET_SILENCE = 6,
  SET_BYPASS_CLUTCH = 7,
  UNSET_BYPASS_CLUTCH = 8,
  SET_HORN_DURATION = 9,
  OPEN_TRUNK = 10,
  UNLOCK_DORS = 11,
  LOCK_DORS = 12,
  SET_RELAYS = 99,

} cmd;

typedef struct {
  bool execute = false;
  int sequence = 0;
  double startTime = 0;
  double duration = 0;
  //bool silent=0;
} Action;
int accesoryComand = 0;
double lastAcessoryRead = 0;
bool bypassCluch = 0, carON = 0;
Action cStopHorn;
Action cStartCar;
Action cStopCar;
Action cStopStarter;
Action cLockDors;
Action cUnlockDors;
Action cOpenTrunk;
bool silent = 0;
int hornDuration = 100;



void setRelays(int val) {

  if ( val & 0x1 ) {
    digitalWrite( relay_1, LOW );
  } else {
    digitalWrite( relay_1, HIGH );
  }

  if ( val & 0x2 ) {
    digitalWrite( relay_2, LOW );
  } else {
    digitalWrite( relay_2, HIGH );
  }

  if ( val & 0x4 ) {
    digitalWrite( relay_3, LOW );
  } else {
    digitalWrite( relay_3, HIGH );
  }

  if ( val & 0x8 ) {
    digitalWrite( relay_4, LOW );
  } else {
    digitalWrite( relay_4, HIGH );
  }


  if ( val & 0x10 ) {
    digitalWrite( relay_5, LOW );
  } else {
    digitalWrite( relay_5, HIGH );
  }

  if ( val & 0x20 ) {
    digitalWrite( relay_6, LOW );
  } else {
    digitalWrite( relay_6, HIGH );
  }


  if ( val & 0x40 ) {
    digitalWrite( relay_7, LOW );
  } else {
    digitalWrite( relay_7, HIGH );
  }


  if ( val & 0x80 ) {
    digitalWrite( relay_8, LOW );
  } else {
    digitalWrite( relay_8, HIGH );
  }
}

void setupPins() {
  pinMode(relay_1, OUTPUT);
  pinMode(relay_2, OUTPUT);
  pinMode(relay_3, OUTPUT);
  pinMode(relay_4, OUTPUT);
  pinMode(relay_5, OUTPUT);
  pinMode(relay_6, OUTPUT);
  pinMode(relay_7, OUTPUT);
  pinMode(relay_8, OUTPUT);

  digitalWrite(relay_1, HIGH);
  digitalWrite(relay_2, HIGH);
  digitalWrite(relay_3, HIGH);
  digitalWrite(relay_4, HIGH);
  digitalWrite(relay_5, HIGH);
  digitalWrite(relay_6, HIGH);
  digitalWrite(relay_7, HIGH);
  digitalWrite(relay_8, HIGH);
}

void  runStarter(double duration) {
  digitalWrite( STARTER, LOW );
  cStopStarter.duration = duration;
  cStopStarter.execute = 1;
  cStopStarter.startTime = millis();
}

void stopStarter() {
  digitalWrite( STARTER, HIGH );
  cStopStarter.execute = 0;
}


void playHorn(double duration) {
  digitalWrite( HORN, LOW );
  cStopHorn.duration = duration;
  cStopHorn.execute = 1;
  cStopHorn.startTime = millis();
}

void stopHorn() {
  digitalWrite( HORN, HIGH );
  cStopHorn.execute = 0;
}

void stopCar() {
  cStopCar.execute = 0;
  if (!silent) {
    playHorn(hornDuration);
  }
  digitalWrite( IGNITION1, HIGH );
  digitalWrite( IGNITION2, HIGH );
  carON = false;
  if (!bypassCluch) {
    digitalWrite( CLUCHSENSOR, HIGH );
  }

}

void startCar() {
  carON = true;
  if (!silent) {
    if ( cStartCar.sequence < 3 ) {
      if ( !cStopHorn.execute ) {
        cStartCar.execute = 1;
        cStartCar.duration = hornDuration * 2;
        cStartCar.startTime = millis();
        playHorn(hornDuration);
        cStartCar.sequence++;
      }
    }
  } else {
    if ( cStartCar.sequence < 3) {
      cStartCar.sequence = 3;
    }
  }
  if ( cStartCar.sequence >= 3 ) {

    if ( cStartCar.sequence == 3 ) {
      digitalWrite( IGNITION1, LOW );
      digitalWrite( IGNITION2, LOW );
      digitalWrite( CLUCHSENSOR, LOW );
      cStartCar.sequence = 4;
      cStartCar.execute = 1;
      cStartCar.duration = 2000;
      cStartCar.startTime = millis();

    } else if ( cStartCar.sequence == 4 ) {
      runStarter(1000);
      cStopCar.execute = 1;
      cStopCar.startTime = millis();
      cStartCar.execute = 0;
      cStartCar.sequence = 0;
    }
  }
}


void lockCar() {
  playHorn(hornDuration/2);
  cLockDors.execute = 1;
  cLockDors.duration = 300;
  cLockDors.startTime = millis();
  digitalWrite( LOCK, LOW );
}

void unlockCar() {
  playHorn(hornDuration/2);
  cUnlockDors.execute = 1;
  cUnlockDors.duration = 500;
  cUnlockDors.startTime = millis();
  //digitalWrite( IGNITION1, LOW );
  //digitalWrite( IGNITION2, LOW );
  digitalWrite( UNLOCK, LOW );
}

void unlockTrunk() {
  playHorn(hornDuration/2);
  cOpenTrunk.execute = 1;
  cOpenTrunk.duration = 200;
  cOpenTrunk.startTime = millis();
  digitalWrite( TRUNK, LOW );
}



void readAndroid() {

  if (usb.isConnected()) { // isConnected makes sure the USB connection is ope
    if (usb.available()) {

      accesoryComand = usb.read();
      Serial.print( accesoryComand );

      switch ( accesoryComand ) {
        case START_CAR:
          if ( !carON ) {
            cStopCar.duration = 300000; //todo make it configurable
            cStartCar.execute = 1;
          }
          break;

        case STOP_CAR:
          stopCar();
          break;

        case START_HONKING:
          digitalWrite( HORN, LOW );
          break;

        case STOP_HONKING:
          digitalWrite( HORN, HIGH );
          break;

        case SET_SILENCE:
          silent = true;
          break;

        case UNSET_SILENCE:
          silent = false;
          break;

        case SET_HORN_DURATION:
          while (!usb.available()) {}
          if (usb.available()) {
            hornDuration = usb.read();
            playHorn(hornDuration);
          }
          break;

        case SET_BYPASS_CLUTCH:
          bypassCluch = true;
          digitalWrite( CLUCHSENSOR, LOW );
          break;

        case UNSET_BYPASS_CLUTCH:
          bypassCluch = false;
          digitalWrite( CLUCHSENSOR, HIGH );
          break;

        case LOCK_DORS:
          lockCar();
          break;

        case UNLOCK_DORS:
          unlockCar();
          break;

        case OPEN_TRUNK:
          unlockTrunk();
          break;

        case SET_RELAYS:
          while (!usb.available()) {}
          if (usb.available()) {
            setRelays(usb.read());
          }
          break;

        default:
          break;
      }
    }
    lastAcessoryRead = millis();

  }
}


void setup() {
  Serial.begin(9600);
  // start the connection to the device over the USB host:
  usb.begin();

  setupPins();
}

void loop() {
  // Print to usb
  timer = millis();

  //STOP HORN
  if ( timer - cStopHorn.startTime >= cStopHorn.duration && cStopHorn.execute) {
    stopHorn();
  }

  //STOP CAR
  if ( timer - cStopCar.startTime >= cStopCar.duration && cStopCar.execute) {
    stopCar();
  }

  //STOP STARTER
  if ( timer - cStopStarter.startTime >= cStopStarter.duration && cStopStarter.execute ) {
    stopStarter();
  }

  //START CAR
  if ( timer - cStartCar.startTime >= cStartCar.duration && cStartCar.execute) {
    startCar();
  }

  //UNLOCK DORS
  if ( timer - cUnlockDors.startTime >= cUnlockDors.duration && cUnlockDors.execute) {
    cUnlockDors.execute = 0;
    digitalWrite( UNLOCK, HIGH );
    //if ( !carON ) {
      //digitalWrite( IGNITION1, HIGH );
      //digitalWrite( IGNITION2, HIGH );
    //}
  }

  //LOCK DORS
  if ( timer - cLockDors.startTime >= cLockDors.duration && cLockDors.execute) {
    cLockDors.execute = 0;
    digitalWrite( LOCK, HIGH );
  }

  //OPEN TRUNK
  if ( timer - cOpenTrunk.startTime >= cOpenTrunk.duration && cOpenTrunk.execute) {
    cOpenTrunk.execute = 0;
    digitalWrite( TRUNK, HIGH );
  }

  //READ USB ACCESSORY
  if (timer - lastAcessoryRead > 100) { // sending 10 times per second
    readAndroid();
  }


}

