#include <Arduino.h>

#define WHEEL_PITCH 0.003 // example pitch in meters
#define WHEEL_TEETH 60
#define INTERRUPT_PIN 2

volatile bool new_pulse = true;
volatile bool wheel_stopped = false;

volatile uint32_t last_time = 0;
volatile uint32_t current_time = 0;
volatile uint32_t time_diff = 0;
volatile uint32_t stop_time = 0;

void isr_handler()
{
  current_time = micros();

	if (last_time != 0)
	{	time_diff = current_time - last_time;
		new_pulse = true;
	}
	last_time = current_time;
}

void setup() 
{
  // put your setup code here, to run once:
  Serial.begin (115200);
  pinMode(INTERRUPT_PIN, INPUT);

  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), isr_handler, RISING);
}

void loop() 
{
  // put your main code here, to run repeatedly:
  static uint32_t total_time = 0;
  static uint32_t pulse_count = 0;
  static float total_distance = 0.0;

  if (new_pulse)
  {
    new_pulse = false;
    float speed = WHEEL_PITCH / (time_diff / 1000000.0);
  	Serial.print("Time difference: ");
    Serial.print(time_diff);
    Serial.print("us, Speed: ");
    Serial.print(speed);
    Serial.print(" m/s");

  	total_time += time_diff;
  	pulse_count++;
  	total_distance += WHEEL_PITCH;

  	stop_time = micros();
    wheel_stopped = false;
  }

  if (!wheel_stopped && (micros() - stop_time)	> 2000000)
  {
  	Serial.println ("wheel stopped \r\n");
    wheel_stopped = true;
    if (pulse_count >0)
    {
    	float average_speed = total_distance / (total_time/1000000.0);
    	float rpm = (60.0 / (total_time/1000000.0))* WHEEL_TEETH;
    	Serial.print("Average speed: ");
      Serial.print(average_speed);
      Serial.print(" m/s, RPM");
      Serial.println(rpm);
    }
    total_time = 0;
    pulse_count = 0;
    total_distance = 0.0;
  }
}
