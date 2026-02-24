#include <LiquidCrystal_I2C.h>
#include <Wire.h> 
#include <ESP32Servo.h>

Servo my_servo;
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define LEDS_PIN        32 
#define POT_PIN         34
#define LDR_PIN         35  

#define FLAME_SENSOR    33
#define ALARM_BUZZER    25
#define LED_ALARM       26
#define SENSOR1         27
#define SENSOR2         14
#define SENSOR3         13  
#define SERVO_PIN    12  

#define input1 19
#define input2 23
#define input3 4
#define input4 18

bool previoussensor1 = 0;
bool previoussensor2 = 0;

unsigned long previous_millis = 0;
unsigned long lcd_timer = 0; 

bool alarmled_state = LOW;


enum SystemState {
  IDLE,
  WELCOME,
  BYE,
  FIRE_ALARM
};
SystemState currentState = IDLE;

void Gateclose ()
{
 
  for(int i=0;i<=90;)	
    {
      unsigned long currentmillis = millis();
     if(currentmillis-previous_millis>=1)
     {
        my_servo.write (i);
        i++;
     	previous_millis=currentmillis; 
     }
    }
}

void Gateopen ()
{
 
  for(int i=90;i>=0;)	
  {
    unsigned long currentmillis = millis();
   if(currentmillis-previous_millis>=1)
   {
      my_servo.write (i);
      i--;
      
     	previous_millis=currentmillis; 
    }
  }
}

void showWelcome() {
  lcd.setCursor(0, 0);
  lcd.print("Welcome to the");
  lcd.setCursor(0, 1);
  lcd.print("Smart Garage  ");
}

void showGoodbye() {
  lcd.setCursor(4, 0);
  lcd.print("Goodbye!    ");
  lcd.setCursor(2, 1);
  lcd.print("Drive Safely");
}

void fireAlert() {
  lcd.setCursor(2, 0);
  lcd.print("!!! FIRE !!!");
  lcd.setCursor(2, 1);
  lcd.print("EVACUATE NOW");
}

void garage_places(bool place1, bool place2, bool place3, bool place4) {
  lcd.setCursor(0, 0); lcd.print("S1:");
  lcd.setCursor(3, 0); lcd.print(place1 ? "F" : "E");

  lcd.setCursor(8, 0); lcd.print("S2:");
  lcd.setCursor(11, 0); lcd.print(place2 ? "F" : "E");
  
  lcd.setCursor(0, 1); lcd.print("S3:");
  lcd.setCursor(3, 1); lcd.print(place3 ? "F" : "E");

  lcd.setCursor(8, 1); lcd.print("S4:");
  lcd.setCursor(11, 1); lcd.print(place4 ? "F" : "E");
}

void set_alarm_state() {
  if (alarmled_state == HIGH) {
    tone(ALARM_BUZZER, 1000); 
  } else {
    noTone(ALARM_BUZZER);
  }
  digitalWrite(LED_ALARM, alarmled_state);
}

void setup() {
  Serial.begin(9600);
  Wire.begin(); 
  lcd.init();
  lcd.backlight();
  lcd.clear();

  pinMode(SENSOR1, INPUT);
  pinMode(SENSOR2, INPUT);
  pinMode(SENSOR3, INPUT);
  
  pinMode(FLAME_SENSOR, INPUT);
  pinMode(POT_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);

  pinMode(LEDS_PIN, OUTPUT);
  pinMode(ALARM_BUZZER, OUTPUT);
  pinMode(LED_ALARM, OUTPUT);

  my_servo.attach(SERVO_PIN);

  currentState = IDLE;
  
  lcd.setCursor(0, 0);
  lcd.print("System Starting");
  delay(1000);
  lcd.clear();
}

void loop() {
  unsigned long currentMillis = millis();

  bool flame_reading = digitalRead(FLAME_SENSOR);
  int pot_reading = analogRead(POT_PIN); 
  int brightness = map(pot_reading, 0, 4095, 0, 255);
  int sunlight = analogRead(LDR_PIN);

  bool currentSensor1 = digitalRead(SENSOR1);
  bool currentSensor2 = digitalRead(SENSOR2);

  Serial.println(sunlight);


  if (sunlight <= 3000) { 
    analogWrite(LEDS_PIN, brightness);
  } else {
    analogWrite(LEDS_PIN, 0);
  }


  switch (currentState) {

    case IDLE:
      noTone(ALARM_BUZZER);
      digitalWrite(LED_ALARM, LOW);
      
     
      if (flame_reading == HIGH) {
        currentState = FIRE_ALARM;
        lcd.clear();
      }
    
      else if (previoussensor1 == 1 && currentSensor1 == 0) {
        Gateopen ();
        currentState = WELCOME;
        lcd.clear();
      }
   
      else if (previoussensor2 == 1 && currentSensor2 == 0) {
        Gateopen ();
        currentState = BYE;
        lcd.clear();
      }

      else {
         if (currentMillis - lcd_timer > 500) {
            garage_places(1-digitalRead(input1), 1-digitalRead(input2), 1-digitalRead(input3), 1-digitalRead(input4));
            lcd_timer = currentMillis;
         }
      }
      break;

    case FIRE_ALARM:
      fireAlert();

      if (currentMillis - previous_millis >= 500) {
        alarmled_state = !alarmled_state; 
        set_alarm_state();
        previous_millis = currentMillis;
      }
      my_servo.write(0);
   
      if (flame_reading == LOW) {
        currentState = IDLE;
        lcd.clear();   
        noTone(ALARM_BUZZER);
        digitalWrite(LED_ALARM, LOW);
        my_servo.write(90);
      }
      break;

    case WELCOME:
      showWelcome();
      if ((previoussensor1==1 && currentSensor1==0) || (previoussensor2==1 && currentSensor2==0) )
    {
    
     Gateclose ();
      lcd.clear();
     currentState= IDLE;
	  }
    previoussensor1=currentSensor1;
    previoussensor2=currentSensor2;
    if (flame_reading == HIGH) {
        currentState = FIRE_ALARM;
        lcd.clear();
    }
      break;

    case BYE:
      showGoodbye();
      if ((previoussensor1==1 && currentSensor1==0) || (previoussensor2==1 && currentSensor2==0) )
    {
    
     Gateclose ();
      lcd.clear();
     currentState= IDLE;
    
	  }
    previoussensor1=currentSensor1;
    previoussensor2=currentSensor2;

     if (flame_reading == HIGH) {
        currentState = FIRE_ALARM;
        lcd.clear();
     }
    break;
  }

  previoussensor1 = currentSensor1;
  previoussensor2 = currentSensor2;
    
}