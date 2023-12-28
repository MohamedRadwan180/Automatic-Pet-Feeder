#include "HX711.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

#define dirPin 4
#define stepPin 5
#define FstepsPerRevolution 99
#define BstepsPerRevolution 50
// HX711 circuit wiring
const int Load_cell_DOUT_pin = A0;
const int LOad_cell_SCK_pin = A1;
bool first_time = true;
float c;
HX711 scale;
float weight_2;

//interface
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Déclare l'écran LCD I2C avec son adresse et dimensions (16 colonnes et 2 lignes)
int switch_1_food_amount = 12;
int switch_2_duration = 13;
int potint = A3;
int potint_val = 0;
int portion_size = 10;
int food_duration = 10000; 
int food_status;
int timing_status;

long current_time;
long start_program;

//Entertainment
Servo servoX;
Servo servoY;
/////// Pin laser 
const int laserPin = 8; // Laser control pin
int posX = 90; // Initial position on X axis
int posY = 90; // Initial position on Y axis

// Total number of points to draw
int totalPoints = 10;
int laserDelay = 200; // Laser flash delay (in milliseconds)
int moveDelay = 500; // Delay between moves (in milliseconds)
int enter_start_time;
int enter_current_time;

////// Code for the ultrasonic sensor
float distance;
int currentState;
bool previousState = false; // the cat is not detected. Initial status
const int echoPin_ultrasonic = 2;
const int trigPin_ultrasonic = 3;
int entertainment_limit = 400;
//////

void initiate_settings()
{
  food_status = digitalRead(switch_1_food_amount);
  Serial.println(food_status);

  if(food_status == 1)
  {
    lcd.setCursor(0, 0);
    lcd.print("Portion Size?");
       lcd.setCursor(15, 1);
      lcd.print("g");
  while(food_status == 1)
  {
    
    potint_val = analogRead(potint);
    portion_size = map(potint_val, 0, 1023, 0, 200);
    food_status = digitalRead(switch_1_food_amount);
      lcd.setCursor(0, 1);
      lcd.print(portion_size);
   
      delay(100);
  }
  }
  else
  {
    lcd.clear();
  }

    timing_status = digitalRead(switch_2_duration);
  Serial.println(timing_status);

  if(timing_status == 1)
  {
    lcd.setCursor(0, 0);
    lcd.print("Feeding Duration?");
       lcd.setCursor(13, 1);
      lcd.print("sec");
  while(timing_status == 1)
  {
    
    potint_val = analogRead(potint);
    food_duration = map(potint_val, 0, 1023, 0, 30000);
    timing_status = digitalRead(switch_2_duration);
      lcd.setCursor(0, 1);
      lcd.print(food_duration/1000);
      delay(100);
  }
  }
  else
  {
    lcd.clear();
  }
}

void Dispense(){

    // Set the spinning direction counterclockwise:
  digitalWrite(dirPin, LOW);

  // Spin the stepper motor 1 revolution slowly:
  for (int i=0; i < BstepsPerRevolution; i++) {
    // These four lines result in 1 step:
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(800);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(800);
  }

  delay(100);
     // Set the spinning direction clockwise:
  digitalWrite(dirPin, HIGH);

  // Spin the stepper motor 1 revolution slowly:
  for (int i=0; i < FstepsPerRevolution; i++) {
    // These four lines result in 1 step:
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(600);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(600);
  }

  delay(1000);
}

void activ_laser() {

  for (int i = 0; i < totalPoints; ++i) {
    // Generate random coordinates over a 0.25 m² area (0.5 m x 0.5 m)
    int targetX = random(50, 200); // Range 50 to 200 (0.05 to 0.20 m)
    int targetY = random(50, 200); // Range 50 to 200 (0.05 to 0.20 m)

    // Convertir les coordonnées en positions pour les servomoteurs
    int servoPosX = map(targetX, 50, 200, 50, 120); // Convert range to degrees for X (servo)
    int servoPosY = map(targetY, 100, 200, 140, 150); // Convert range to degrees for Y (servo)

    // Move servos to target coordinates
    servoX.write(servoPosX);
    servoY.write(servoPosY);

    // Activate laser (flashing)
    digitalWrite(laserPin, HIGH);
    delay(laserDelay); // Flash delay

    // Deactivate laser
    digitalWrite(laserPin, LOW);

    // Wait before moving on to the next item
    delay(moveDelay);
  }

}

float fun_distance() {
  // Clear the Trigger pin
  digitalWrite(trigPin_ultrasonic, LOW);
  delayMicroseconds(2);
  // Set the Trigger pin high for 10 microseconds
  digitalWrite(trigPin_ultrasonic, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin_ultrasonic, LOW);

  // Read the pulse time on Echo pin
  float duration = pulseIn(echoPin_ultrasonic, HIGH);

  // Calculate the distance
  distance = duration * 0.34 / 2; // Speed of sound: 343 m/s

  // Display the distance on the Serial Monitor
  // Serial.print("Distance: ");
  // Serial.println(distance);
  return distance; //in milimeters
}

void setup() {
    //////// Setup ultrasonic senosr pins 
  pinMode(echoPin_ultrasonic, INPUT);  // Pin 2    Set the Echo pin as an input
  pinMode(trigPin_ultrasonic, OUTPUT); // Pin 3    Set the Trigger pin as an output
  Serial.begin(9600);       // Initialize serial communication
  // Also Vcc + GND
  //////// 
  // put your setup code here, to run once:
  Serial.begin(9600);
  scale.begin(Load_cell_DOUT_pin, LOad_cell_SCK_pin);

  // Declare pins as output:
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);

    //initiating switches
  pinMode(switch_1_food_amount, INPUT_PULLUP);
  pinMode(switch_2_duration, INPUT_PULLUP);
  //initiating LCD
  lcd.init();                      // Initialize the LCD screen
  lcd.backlight();                 // Activate the screen backlight
  start_program = millis();


    /////// Servo-motor code
  servoX.attach(6);  // Pin 6 + Vcc + GND    Attach the first servo (modifiable according to your wiring)
  servoY.attach(9); // Pin 9 + Vcc + GND    Attach the second servo (modifiable according to your wiring)
  ///////

  /////// Laser 
  pinMode(laserPin, OUTPUT); // Pin 8 + Vcc + GND
  ///////
}




void loop() { // Main loop, where all the functions are called. Here starts the WAIT block of the FLow Diagram
  lcd.clear();    
  initiate_settings();
  current_time = millis();
   Serial.println(scale.is_ready());
    if (scale.is_ready()) {         // Take weight measurements from the sensor
    weight_2 = scale.get_units(10); // Average of 10 readings
    if (first_time == true){
      c = -weight_2*0.0026;         // The first time that the system is working, with the tray empty, 
                                    // measurement subsystem is calibrated.
      first_time = false;
    }
    Serial.print("Weight: ");
    Serial.print(weight_2*0.0026 + c); // Two decimal places
    Serial.println(" g");
    lcd.setCursor(0, 0);
    lcd.print("Weight: ");
    lcd.setCursor(9, 0);
    lcd.print((weight_2*0.0026 + c));
    lcd.setCursor(0, 1);
    lcd.print("Feed in: ");
    lcd.setCursor(14, 1);
    lcd.print("s");
    lcd.setCursor(9, 1);
    lcd.print((food_duration/1000) - ((current_time-start_program)/1000));
    
    if(current_time-start_program>=food_duration) // check if it is time to feed
    {
    if(weight_2*0.0026 + c < portion_size)        // check if the tray has enought food
    {
    while(weight_2*0.0026 + c < portion_size)      // dispense food until the limit has been reached
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Time to feed......");
      lcd.setCursor(0, 1);
      lcd.print("Food: ");
      lcd.setCursor(12, 1);
      lcd.print("g");
      Dispense();                                 // function to dispense food
      //tart_program = current_time;
      weight_2 = scale.get_units(10); 
      delay(1000);
      start_program = current_time;               // set the current time as the last time that food has been given
      lcd.setCursor(7, 1);
      lcd.print(weight_2*0.0026 + c);
    }
    }else{
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Tray is full");                  // else, say that the tray is full
      delay(3000);
      lcd.clear();
    }
        start_program = current_time;

    }
  } else {
    Serial.println("HX not working!!");
  }

  if(fun_distance() <= entertainment_limit)     // check if the cat is nearby
  {
    activ_laser();                              // entertain the cat and give food reward
    Dispense();
  }

  delay(1000);                                  // 1s delay to avoid probelms with the weight sensor measurements
}
