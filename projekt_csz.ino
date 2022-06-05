#include <Wire.h>
#include<TimerOne.h>

long accelX, accelY, accelZ;
float gForceX, gForceY, gForceZ, prevgForceY = 0;

long gyroX, gyroY, gyroZ;
float rotX, rotY, rotZ, prevRotY = 0;

int backLight = 13;
int leftSignal = 12;
int rightSignal = 11;
int frontLight = A2;
int fotoResistor = 0;

void setup() {
  Serial.begin(9600);
  pinMode(backLight, OUTPUT);
  pinMode(leftSignal, OUTPUT);
  pinMode(rightSignal, OUTPUT);
  Timer1.initialize( 200000 );
  Timer1.attachInterrupt(controlSignalLights); 
  Wire.begin();
  setupMPU();
}


void loop() {
  recordAccelRegisters();
  recordGyroRegisters();
  controlBackLight();
  int frontLightBrightness = map(analogRead(fotoResistor), 700, 1023, 0, 255);
  analogWrite(frontLight, frontLightBrightness);
  //printData();
  delay(100);
}

void setupMPU(){
  Wire.beginTransmission(0b1101000); //This is the I2C address of the MPU (b1101000/b1101001 for AC0 low/high datasheet sec. 9.2)
  Wire.write(0x6B); //Accessing the register 6B - Power Management (Sec. 4.28)
  Wire.write(0b00000000); //Setting SLEEP register to 0. (Required; see Note on p. 9)
  Wire.endTransmission();  
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x1B); //Accessing the register 1B - Gyroscope Configuration (Sec. 4.4) 
  Wire.write(0x00000000); //Setting the gyro to full scale +/- 250deg./s 
  Wire.endTransmission(); 
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x1C); //Accessing the register 1C - Acccelerometer Configuration (Sec. 4.5) 
  Wire.write(0b00000000); //Setting the accel to +/- 2g
  Wire.endTransmission(); 
}

void recordAccelRegisters() {
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x3B); //Starting register for Accel Readings
  Wire.endTransmission();
  Wire.requestFrom(0b1101000,6); //Request Accel Registers (3B - 40)
  while(Wire.available() < 6);
  accelX = Wire.read()<<8|Wire.read(); //Store first two bytes into accelX
  accelY = Wire.read()<<8|Wire.read(); //Store middle two bytes into accelY
  accelZ = Wire.read()<<8|Wire.read(); //Store last two bytes into accelZ
  processAccelData();
}

void processAccelData(){
  gForceX = accelX / 16384.0;
  gForceY = accelY / 16384.0; 
  gForceZ = accelZ / 16384.0;
}

void recordGyroRegisters() {
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x43); //Starting register for Gyro Readings
  Wire.endTransmission();
  Wire.requestFrom(0b1101000,6); //Request Gyro Registers (43 - 48)
  while(Wire.available() < 6);
  gyroX = Wire.read()<<8|Wire.read(); //Store first two bytes into accelX
  gyroY = Wire.read()<<8|Wire.read(); //Store middle two bytes into accelY
  gyroZ = Wire.read()<<8|Wire.read(); //Store last two bytes into accelZ
  processGyroData();
}

void processGyroData() {
  rotX = gyroX / 131.0;
  rotY = gyroY / 131.0; 
  rotZ = gyroZ / 131.0;
}

void controlBackLight(){
  if(gForceY < prevgForceY - 0.03){
    digitalWrite(backLight, LOW);
  }
  else{
    digitalWrite(backLight, HIGH);
  }
  prevgForceY = gForceY;
}

void controlSignalLights(){
  if(rotY < -3 && prevRotY < -3){
    digitalWrite(leftSignal, !digitalRead(leftSignal));
  }
  else{
    digitalWrite(leftSignal, HIGH);
  }
  if(rotY > 3 && prevRotY > 3){
    digitalWrite(rightSignal, !digitalRead(rightSignal));
  }
  else{
    digitalWrite(rightSignal, HIGH);
  }
  prevRotY = rotY;
}

void printData() {
  Serial.print("Gyro (deg)");
  Serial.print(" X=");
  Serial.print(rotX);
  Serial.print(" Y=");
  Serial.print(rotY);
  Serial.print(" Z=");
  Serial.print(rotZ);
  Serial.print(" Accel (g)");
  Serial.print(" X=");
  Serial.print(gForceX);
  Serial.print(" Y=");
  Serial.print(gForceY);
  Serial.print(" Z=");
  Serial.println(gForceZ);
}
