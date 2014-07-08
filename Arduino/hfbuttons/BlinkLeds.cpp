#include <Arduino.h>

#include "BlinkLeds.h"

/*
 * différence "juste" entre deux chronos (tient compte d'un éventuel passage par 0 on est pas a l'abrit ...)
 */
inline unsigned long my_diff_millis(unsigned long chrono, unsigned long now)
{
  return now >= chrono ? now - chrono : 0xFFFFFFFF - chrono + now;
}


/*
 * Création d'un clignotement de led
 */
BlinkLeds::BlinkLeds(unsigned int i)
{
  interval = i;
  ledState = LOW;
  previousMillis = 0;
}


/*
 * mis à jour de l'état d'un clignotement en fonction du temps passé
 */
void BlinkLeds::run() {
  unsigned long currentMillis = millis();
  if(my_diff_millis(previousMillis, currentMillis) > interval) {
    previousMillis = currentMillis;
    ledState = !ledState;
  }
}


void BlinkLeds::blink(int pin)
{
  if(!getLedState())
    digitalWrite(pin,LOW);
  else
    digitalWrite(pin,HIGH);
}

