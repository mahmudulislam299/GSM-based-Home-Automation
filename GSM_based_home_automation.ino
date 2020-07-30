/* Mahmudul Islam
 * EEE,Bangladesh University of Engineering and Technology (BUET)
 * Email: mahmudulislam299@gmail.com
 * Project: Home Automation with GSM module
 */
 
#include <DFRobot_sim808.h> //sim808 module
#include <SoftwareSerial.h> 
#include "Wire.h" //ds3231 timer
#include <EEPROM.h> // EEPROM is good 100,000 write/erase cycles // 3.3 ms per write

#define buzzer 52
#define DS3231_I2C_ADDRESS 0x68
#define MESSAGE_LENGTH 160
#define PHONE_NUMBER "+88018********"
#define MESSAGE  "hello,world"

byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
char message[MESSAGE_LENGTH];
int messageIndex = 0;
char phone[16];
char datetime[24];

int LIGHT1 = 10 ; // light number 1
int LIGHT2 = 11 ; // light number 2
int buttonON = 7; //switch on button
int buttonOFF = 8; //swtich off button

int ONhour= 19 ,ONminute= 00,ONsecond=00; //time of light on
int OFFhour=5 ,OFFminute=30 ,OFFsecond=00; //time of light off


int lastPress1 = 0 ; // last press ?? on or off?? for eeprom . it will save the state whether light on or off
int lastPress2 = 0 ;
uint8_t EEPROMaddress1 = 130 ; // eeprom address
uint8_t EEPROMaddress2 = 12 ; 

DFRobot_SIM808 sim808(&Serial);
void beep();

void LIGHT_init() {
  pinMode(LIGHT1, OUTPUT) ;
  pinMode(LIGHT2, OUTPUT) ;
  pinMode(buttonON, INPUT_PULLUP) ; 
  pinMode(buttonOFF, INPUT_PULLUP) ;
  /* push button use without using resistor.. internal pull up resistor of arduino is used
   *  in is case.. the connection patter is [digital_pin]---[switch]---[ground]
   *  the pin is always HIGH. When push button is pressed the digital pin will become LOW.
   */
}

void sim_intial(){
 //******** Initialize sim808 module *************
  while(!sim808.init()) {
      Serial.print("Sim808 init error\r\n");
      delay(1000);
  }
  delay(2000);  
  Serial.println("Init Success, please send SMS message to me!"); 
}

byte decToBcd(byte val){
  // Convert normal decimal numbers to binary coded decimal
  return( (val/10*16) + (val%10) );
}

byte bcdToDec(byte val){
// Convert binary coded decimal to normal decimal numbers
  return( (val/16*10) + (val%16) );
}

void setup() {
  Wire.begin();
  Serial.begin(9600);
//  sim_intial();
  LIGHT_init();
  pinMode(buzzer, OUTPUT);
  beep(); delay(50); beep(); delay(50); beep();
  // DS3231 seconds, minutes, hours, day, date, month, year
//  setDS3231time(00,33,11,6,24,8,18); // set the initial time here:
}



void beep() {
  analogWrite(buzzer, 255);
  delay(100);
  analogWrite(buzzer,0);
  delay(100);
  analogWrite(buzzer, 255);
  delay(100);
  analogWrite(buzzer,0);
}

void message_send() {
//  Serial.println("Start to send message ...");
  //******** define phone number and text **********
  sim808.sendSMS(PHONE_NUMBER,MESSAGE); 
}

void message_read() {
  //*********** Detecting unread SMS ************************
   messageIndex = sim808.isSMSunread();
    Serial.print("messageIndex: ");
    Serial.println(messageIndex);

   //*********** At least, there is one UNREAD SMS ***********
   if (messageIndex > 0) { 
      sim808.readSMS(messageIndex, message, MESSAGE_LENGTH, phone, datetime);
                 
      //***********In order not to full SIM Memory, is better to delete it**********
      sim808.deleteSMS(messageIndex);
//      Serial.println("From number: ");
//      Serial.println(phone);  
//      Serial.print("Datetime: ");
//      Serial.println(datetime);        
//      Serial.print("Recieved Message: ");
//      Serial.println(message);
        
      
   }
}

void setDS3231time(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year){
  // sets time and date data to DS3231
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set next input to start at the seconds register
  Wire.write(decToBcd(second)); // set seconds
  Wire.write(decToBcd(minute)); // set minutes
  Wire.write(decToBcd(hour)); // set hours
  Wire.write(decToBcd(dayOfWeek)); // set day of week (1=Sunday, 7=Saturday)
  Wire.write(decToBcd(dayOfMonth)); // set date (1 to 31)
  Wire.write(decToBcd(month)); // set month
  Wire.write(decToBcd(year)); // set year (0 to 99)
  Wire.endTransmission();
}

void readDS3231time(byte *second, byte *minute, byte *hour, byte *dayOfWeek, byte *dayOfMonth, byte *month, byte *year){
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  // request seven bytes of data from DS3231 starting from register 00h
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
  *dayOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read());
}

void Time(){
  
  /***retrieve data from DS3231***/
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
  /*****send it to the serial monitor******/
  Serial.print(hour); Serial.print(":"); Serial.print(minute);
  Serial.print(":"); Serial.print(second); Serial.print(" ");
  Serial.print(dayOfMonth); Serial.print("/");
  Serial.print(month); Serial.print("/"); Serial.println(year);  
}


void loop(){
//  message_read();
  lastPress1 = EEPROM.read(EEPROMaddress1) ;
  lastPress2 = EEPROM.read(EEPROMaddress2) ;
  Serial.print(lastPress1); Serial.print("**");
  Serial.print(lastPress2); Serial.print("**");
  Time();
  
/** BUTTON press ON**/  
  if (digitalRead(buttonON)== LOW){
    beep(); delay(200); beep();  //beep for light on
    lastPress1 = buttonON ;
    lastPress2 = buttonON ;
    EEPROM.write(EEPROMaddress1, lastPress1) ;
    EEPROM.write(EEPROMaddress2, lastPress2) ;
    Serial.println("on button");
    delay(100);
  }

/**BUTTON press OFF**/  
  else if (digitalRead(buttonOFF)==LOW){
    lastPress1 = buttonOFF ;
    lastPress2 = buttonOFF ;
    EEPROM.write(EEPROMaddress1, lastPress1) ;
    EEPROM.write(EEPROMaddress2, lastPress2) ;
    beep(); delay(100); beep(); // beep for light off
    Serial.println("off button");
    delay(100);
  }

  if (hour==ONhour && minute==ONminute && second== ONsecond){
    Serial.println("ON message");
    beep(); delay(200); beep(); //light on beep
    lastPress1 = buttonON ;
    lastPress2 = buttonON ;
    EEPROM.write(EEPROMaddress1, lastPress1) ;
    EEPROM.write(EEPROMaddress2, lastPress2);
    delay(10);
  }
  else if (hour == OFFhour && minute == OFFminute && second == OFFsecond){
    Serial.println("OFF time");
    beep(); delay(100); beep(); //light off beep
    lastPress1 = buttonOFF ;
    lastPress2 = buttonOFF ;
    EEPROM.write(EEPROMaddress1, lastPress1) ;
    EEPROM.write(EEPROMaddress2, lastPress2) ;
   
     delay(10);
  }

   if ((strcmp (message,"on") == 0) && messageIndex>0){
    Serial.println("ON message");
    beep(); delay(200); beep(); //light on beep
    lastPress1 = buttonON ;
    lastPress2 = buttonON ;
    EEPROM.write(EEPROMaddress1, lastPress1) ;
    EEPROM.write(EEPROMaddress2, lastPress2);
    delay(10);
   }

   if ((strcmp (message,"off") == 0) && messageIndex>0){
    Serial.println("OFF time");
    beep(); delay(100); beep(); //light off beep
    lastPress1 = buttonOFF ;
    lastPress2 = buttonOFF ;
    EEPROM.write(EEPROMaddress1, lastPress1) ;
    EEPROM.write(EEPROMaddress2, lastPress2) ;
   
     delay(10);
   }

   if ((strcmp (message,"on1") == 0) && messageIndex>0){
    Serial.println("Light 1 ON message");
    beep(); delay(200); beep(); //light on beep
    lastPress1 = buttonON ;
    EEPROM.write(EEPROMaddress1, lastPress1);
    delay(10);
   }

   if ((strcmp (message,"off1") == 0) && messageIndex>0){
    Serial.println("Light 1 OFF message");
    beep(); delay(200); beep(); //light on beep
    lastPress1 = buttonOFF ;
    EEPROM.write(EEPROMaddress1, lastPress1);
    delay(10);
   }

   if ((strcmp (message,"on2") == 0) && messageIndex>0){
    Serial.println("Light 2 ON message");
    beep(); delay(200); beep(); //light on beep
    lastPress2 = buttonON ;
    EEPROM.write(EEPROMaddress2, lastPress2);
    delay(10);
   }

   if ((strcmp (message,"off2") == 0) && messageIndex>0){
    Serial.println("Light 2 OFF message");
    beep(); delay(200); beep(); //light on beep
    lastPress2 = buttonOFF ;
    EEPROM.write(EEPROMaddress2, lastPress2);
    delay(10);
   }
  
  if (lastPress1 == buttonON){
    digitalWrite(LIGHT1, LOW) ;
  }
  else if (lastPress1 == buttonOFF){
    digitalWrite(LIGHT1, HIGH) ;
  }

  if (lastPress2 == buttonON){
    digitalWrite(LIGHT2, LOW) ;
  }
  else if (lastPress2 == buttonOFF){
    digitalWrite(LIGHT2, HIGH) ;
  }
}
