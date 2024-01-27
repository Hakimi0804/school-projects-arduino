#include <assert.h>

#define DELAY_US 400

const int driver_in1_pin=10;
const int driver_in2_pin=11;
const int enA_pin=6;

bool forward_done=false;
bool backward_done=false;

void setup() {
  Serial.begin(9600);
  pinMode(driver_in1_pin, OUTPUT);
  pinMode(driver_in2_pin, OUTPUT);
  pinMode(enA_pin, OUTPUT);
  digitalWrite(driver_in1_pin, HIGH);
  digitalWrite(driver_in2_pin, LOW);
}

// void turnOn()
// {
//   digitalWrite(driver_in1_pin, HIGH);
//   digitalWrite(driver_in2_pin, LOW);
// }

// void turnOff()
// {
//   digitalWrite(driver_in1_pin, LOW);
//   digitalWrite(driver_in2_pin, LOW);
// }

int getBalance(int time)
{
  assert(time <= 1000);
  return 1000 - time;
}

void loop() {
  analogWrite(driver_in1_pin, 38);
}
