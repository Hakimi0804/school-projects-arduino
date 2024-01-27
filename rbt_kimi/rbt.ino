#include <Servo.h>
#include <stdbool.h>

#ifndef PERSON_TRIGGER_TRESHOLD
#define PERSON_TRIGGER_TRESHOLD 38
#endif

#ifndef RECHECK_VALUE
#define RECHECK_VALUE 1
#endif

// normally a  continuos rotation servo can be stopped by writing (servo.write())
// a value of 90. However, on some cheap servo, these values might be off. in
// my case it is off by +4.
#ifndef SERVO_HALT_OFFSET
#define SERVO_HALT_OFFSET 4
#endif

#ifndef SERVO_MAXSPEED_OFFSET
#define SERVO_MAXSPEED_OFFSET 0
#endif

#ifndef SERVO_MAXSPEED_REVERSED_OFFSET
#define SERVO_MAXSPEED_REVERSED_OFFSET 0
#endif

const int usSensor2_trigPin=9;
const int isSensor2_echoPin=8;

const int usSensor1_trigPin=7;
const int usSensor1_echoPin=6;

const int outerServoPin=3;
const int innerServoPin=5;

const int ledPin=4;

long int duration;
long int distance;

static int recheck_hasPerson_count = 0;
static int recheck_noPerson_count = 0;

static bool noperson_written = false;
static bool hasperson_written = false;

char strbuf[50];

Servo outerServo;
Servo innerServo;

void setup()
{
  Serial.begin(31250);
  outerServo.attach(outerServoPin, 600, 2300);
  innerServo.attach(innerServoPin, 600, 2300);
  outerServo.write(90 + SERVO_HALT_OFFSET);
  innerServo.write(90 + SERVO_HALT_OFFSET);
  noperson_written = true;
  Serial.println("Started");
  pinMode(usSensor1_trigPin, OUTPUT);
  pinMode(usSensor1_echoPin, INPUT);
  pinMode(innerServoPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
}

void moveServo(int angle)
{
  assert(angle >= 0);
  assert(angle <= 360);
  assert(outerServo.attached());
  outerServo.write(angle);
}

void ledToggle(bool state)
{
  if (state) {
    digitalWrite(ledPin, HIGH);
  } else if (!state) {
    digitalWrite(ledPin, LOW);
  }
}

void servo_setpos(Servo *servo, int position, bool reversed, int customDelay = NULL)
{
  const float speedForOneDeg = (0.1/60)*1000; // in millisecs
  const int speedNorm = 0 + SERVO_MAXSPEED_OFFSET; // max speed
  const int speedReversed = 180 + SERVO_MAXSPEED_REVERSED_OFFSET; // max speed reversed direction
  const int speedHalt = 90 + SERVO_HALT_OFFSET; // static

  int delayVal;

  if (customDelay != NULL)
    delayVal = customDelay;
  else
    delayVal = speedForOneDeg * position;

  assert(servo->attached());
  if (reversed)
    servo->write(speedNorm);
  else
    servo->write(speedReversed);

  delay(delayVal);
  servo->write(speedHalt);
}

void loop()
{
  Serial.flush();
  digitalWrite(usSensor1_trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(usSensor1_trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(usSensor1_trigPin, LOW);

  duration = pulseIn(usSensor1_echoPin, HIGH);

  distance = duration * 0.034 / 2;
  if (distance == 0)
    return;
  memset(strbuf, 0, sizeof(strbuf));
  snprintf(strbuf, sizeof(long int), "%ld", distance); // cast distance to char[], store in strbuf

  // deadlock prevention
  if (noperson_written && hasperson_written) {
    Serial.println("Deadlock!");
    noperson_written = false;
    hasperson_written = false;
  } else if (!noperson_written && !hasperson_written) {
    Serial.println("Deadlock!");
    noperson_written = false;
    hasperson_written = false;
  }

  // person detected, open the door
  if (distance <= PERSON_TRIGGER_TRESHOLD) {
    if (recheck_hasPerson_count < RECHECK_VALUE) {
      recheck_hasPerson_count++;
    } else {
      Serial.print("Person is detected !! distance=");
      Serial.println(strbuf);
      ledToggle(true);
      memset(strbuf, 0, sizeof(strbuf));

      if (!hasperson_written) {
        servo_setpos(&outerServo, 90, false);
        servo_setpos(&innerServo, 180, false, (int)((0.1/60)*1000+500));
        hasperson_written = true;
        noperson_written = false;
      }
      recheck_hasPerson_count = 0;
    }

  // no person, close the door
  } else if (distance > PERSON_TRIGGER_TRESHOLD) {
    if (recheck_noPerson_count < RECHECK_VALUE) {
      recheck_noPerson_count++;
    } else {
      Serial.print("No person is detected. distance=");
      Serial.println(strbuf);
      ledToggle(false);
      memset(strbuf, 0, sizeof(strbuf));

      if (!noperson_written) {
        servo_setpos(&innerServo, 10, true);
        servo_setpos(&outerServo, 180, true, (int)((0.1/60)*1000+1000));
        noperson_written = true;
        hasperson_written = false;
      }
      recheck_noPerson_count = 0;
    }
  }
}
