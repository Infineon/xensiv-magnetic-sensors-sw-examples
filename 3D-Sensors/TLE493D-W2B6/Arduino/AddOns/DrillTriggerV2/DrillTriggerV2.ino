#include <Tle493d.h>

#define DIR_THR_DEGREE 11 // range between 0° and 14° as the stroke is between -28° and 28°
#define PRESS_THR_PERCENT 10 // the edges of the press range get an additional space so that the limits are always reached

Tle493d sensor = Tle493d();

//startingvalue for the limits they get be updated during opperation
double minDir = -0.1;
double maxDir = 0.1;
double minPress = -0.7;
double maxPress = 0.7;

int dir = 0;
int pwr = 0;

void setup() {
    Serial.begin(1000000);
    delay(50);
    /* Definition of the power pin to power up the sensor. 
       The sensor is connected to LED2 pin. */
    pinMode(LED2, OUTPUT);
    digitalWrite(LED2, HIGH);
    delayMicroseconds(50);
    sensor.begin();
}

void loop() {
  double x = 0, y = 0, z = 0;
  double avgx=0, avgy=0, avgz = 0;
  for(int i = 0; i < 10; i++)
  {
    sensor.updateData();
    avgx += sensor.getX();
    avgy += sensor.getY();
    avgz += sensor.getZ();
  }
  x = avgx/10;
  y = avgy/10;
  z = avgz/10;
  
  //update and compute press value
  double anglePress = x<0 ? atan2(-y, -x) : atan2(y, x);
  minPress = anglePress<minPress ? anglePress : minPress;
  maxPress = anglePress>maxPress ? anglePress : maxPress;
  double press = map(anglePress, maxPress, minPress, (0-PRESS_THR_PERCENT), (100+PRESS_THR_PERCENT));
  if(press < 0)         {pwr = 0;}
  else if(press > 100)  {pwr = 100;}
  else                  {pwr = (int)(round(press));}

  //update and compute direction value only if trigger level is zero
  if(pwr == 0) {
    double angleDir = x<0 ? atan2(-z, -x) : atan2(z, x);
    minDir = angleDir<minDir ? angleDir : minDir;
    maxDir = angleDir>maxDir ? angleDir : maxDir;
    double direction = map(angleDir, minDir, maxDir, -28, 28);
    if(dir == -1) {
      if(direction > (-DIR_THR_DEGREE)) 
        dir = 0;
    }
    else if(dir == 0) {
      if(direction < (-28+DIR_THR_DEGREE)) 
        dir = -1;
      else if(direction > (28-DIR_THR_DEGREE))
        dir = 1;
    }
    else if(dir == 1) {
      if(direction < (DIR_THR_DEGREE))
        dir = 0;
    }
  }
  
  Serial.print(pwr);    Serial.print(",");     Serial.print(dir);
  Serial.println();
  delay(20);   
}