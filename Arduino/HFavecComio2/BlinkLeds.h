/* pour un clignotement de led synchronisé */
#ifndef __BLINKLEDS__
#define __BLINKLEDS__
class BlinkLeds
{
public:
  BlinkLeds(unsigned int i);
  void run();
  int getLedState() {
    return ledState;
  }
  void blink(int pin);
private:
  unsigned int interval;
  int ledState;
  unsigned long previousMillis;
};
#endif
