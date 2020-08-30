/*
  Timing program that uses IR sensors of RollBot to time a passing object.

  Time stamp is made when at least one sensor changes.
  Both LEDs are turned on when sensors are reading.
  Left LED is turned off when sensor reading is paused to let timed object pass sensors.

  Author of calibration and normalization functions: Gudny Björk Odinsdottir
  Date: 2019-09-26
  Author of timing function: Mattias Ahle
  Date: 2019-10-16
*/



//Define the LED pins on the robot, used to control the calibration:
#define   LEFT_LED      3
#define   RIGHT_LED     2

//Global variables (arrays) used for the Normalization:
char sensors[5] = {A0, A1, A2, A3, A7}; /*Used to be able to loop through the 5 different sensors*/
int sensorvalues[5];    /*Used to store the raw sensor values*/
int normalizedsensorvalues[5];    /*Used to store the normalized sensor values*/
int maxsensorvalues[5] = {0, 0, 0, 0, 0};   /*Used to find the maximum value (over white surface) for each sensor*/
int minsensorvalues[5] = {1023, 1023, 1023, 1023, 1023};    /*Used to find the minimum value (over black surface) for each sensor*/
//int maxsensorvalues[5] = {335, 338, 345, 356, 333};
//int minsensorvalues[5] = {48, 48, 57, 54, 47};

int threshold = 512;
int lapCounter = 0;
unsigned long timeStamp;
unsigned long previousTimeStamp = 0;
unsigned long bestLapTime = 0;
int lapTimeMinutes;
int lapTimeSeconds;
int lapTimeSecondsTens;
int lapTimeSecondsOnes;
int lapTimeFrac;
int lapTimeTenths;
int lapTimeHundredths;
int lapTimeThousandths;



void setup() {

  for (int i = 0; i < 5; i++) {
    pinMode(sensors[i], INPUT);
  }

  pinMode(LEFT_LED, OUTPUT);
  pinMode(RIGHT_LED, OUTPUT);

  sensorCalibration();

  Serial.begin(9600);

  Serial.println("--- WAITING FOR OBJECT TO PASS SENSORS ---");

}



void loop() {

  digitalWrite(LEFT_LED, HIGH);
  digitalWrite(RIGHT_LED, HIGH);

  for (int i = 0; i < 5; i++) {
    sensorvalues[i] = analogRead(sensors[i]);
  }

  normalizeSensorValues();

  //Uncomment to print sensor values
  //      for (int i = 0; i < 5; i++) {
  //        Serial.print(normalizedsensorvalues[i]);
  //        Serial.print("\t");
  //      }
  //      Serial.println();

  if (normalizedsensorvalues[0] > threshold ||
      normalizedsensorvalues[1] > threshold ||
      normalizedsensorvalues[2] > threshold ||
      normalizedsensorvalues[3] > threshold ||
      normalizedsensorvalues[4] > threshold) {
    timeStamp = millis();
    digitalWrite(LEFT_LED, LOW);
    digitalWrite(RIGHT_LED, HIGH);
    if (previousTimeStamp != 0) { //Don't run until second timestamp (completion of lap 1)
      lapCounter++;
      printLapTime(timeStamp - previousTimeStamp);
    } else {
      Serial.println("Lap 1 started");
    }
    delay(3000); //Give timed object time to pass sensors
    previousTimeStamp = timeStamp;
  }

}



void printLapTime(unsigned long lapTimeInMillis) {
  lapTimeMinutes = lapTimeInMillis / 60000;
  lapTimeSeconds = lapTimeInMillis / 1000 - lapTimeMinutes * 60;
  lapTimeSecondsTens = lapTimeSeconds / 10;
  lapTimeSecondsOnes = lapTimeSeconds % 10;
  lapTimeFrac = lapTimeInMillis - (lapTimeInMillis / 1000) * 1000;
  lapTimeTenths = lapTimeFrac / 100;
  lapTimeHundredths = lapTimeFrac / 10 % 10;
  lapTimeThousandths = lapTimeFrac % 10;

  Serial.print("Lap ");
  Serial.print(lapCounter);
  Serial.print(": ");
  Serial.print(lapTimeMinutes);
  Serial.print(":");
  Serial.print(lapTimeSecondsTens);
  Serial.print(lapTimeSecondsOnes);
  Serial.print(".");
  Serial.print(lapTimeTenths);
  Serial.print(lapTimeHundredths);
  Serial.print(lapTimeThousandths);
  if (lapCounter == 1) {
    bestLapTime = lapTimeInMillis;
  }
  if (lapTimeInMillis < bestLapTime) {
    Serial.print(" <-- NEW BEST!");
    bestLapTime = lapTimeInMillis;
  }
  Serial.println();
}



//Calibration Function Used To Find Out Max- And Min Reading Value For Each Sensor:
void sensorCalibration() {

  Serial.begin(9600);
  Serial.println("Starting The Calibration!");
  Serial.println("Move The Robot Back And Forth Over The Black Line");
  Serial.println("Press Start Button To Start To Calibrate");
  digitalWrite(LEFT_LED, HIGH);   /*Turn on the left LED on the robot*/
  while (digitalRead(10) == HIGH) {
    /*Wait Until Button is Pressed*/
  }
  digitalWrite(RIGHT_LED, HIGH);  /*Turn on the right LED on the robot*/

  //This functionality reads 10000 values for each sensor while you move the
  //robot back and forth over the black line. Every time the new value is either
  //less than the min value or bigger than the max value for that sensor it stores
  //that value as the new min- respectively max value.
  //Move the robot back and forth while both LED's on the robot are turned on!
  for (int i = 0; i < 20000 ; i++) {
    for (int j = 0 ; j < 5; j++) {
      if (analogRead(sensors[j]) > maxsensorvalues[j]) {
        maxsensorvalues[j] = analogRead(sensors[j]);
        Serial.print("New max value: ");
        Serial.println(maxsensorvalues[j]);
      }
      if (analogRead(sensors[j]) < minsensorvalues[j]) {
        minsensorvalues[j] = analogRead(sensors[j]);
        Serial.print("New min value: ");
        Serial.println(minsensorvalues[j]);
      }
    }
  }

  digitalWrite(LEFT_LED, LOW);    /*Turn off the left LED on the robot*/
  Serial.println();
  Serial.println("The obtained maximum and minimum sensor values for each sensor:");
  for (int i = 0; i < 5; i++) {
    if (i == 4) {
      Serial.print("A7: ");
    } else {
      Serial.print("A");
      Serial.print(i);
      Serial.print(": ");
    }
    Serial.print(maxsensorvalues[i]);
    Serial.print("  ");
    Serial.println(minsensorvalues[i]);
  }
  Serial.println();
  Serial.println("Calibration Is Over!");
  Serial.println("Press Start When You Have Placed The Robot On The Racing Track");
  while (digitalRead(10) == HIGH) {
    /*Wait Until Button is Pressed*/
  }
  digitalWrite(RIGHT_LED, LOW);   /*Turn off the right LED on the robot*/
  Serial.println("Start");

}



//Normalizes the Sensor Values:
void normalizeSensorValues() {

  //This functionality will take your sensorvalue and will map it to a value between 0 and 1023
  //depending on the minimum- respectively maximum value obtained from the Calibration Function.
  //In case the sensor value becomes less or greater than the obtained minimum- and maximum value
  //you will get a normalized value which is <0 or >1023. In this case you set your value to 0 respectively 1023
  for (int i = 0; i < 5; i++) {
    normalizedsensorvalues[i] = map(sensorvalues[i], minsensorvalues[i], maxsensorvalues[i], 0, 1023);
    if (normalizedsensorvalues[i] < 0) {
      normalizedsensorvalues[i] = 0;
    } else if (normalizedsensorvalues[i] > 1023) {
      normalizedsensorvalues[i] = 1023;
    }
  }
}
