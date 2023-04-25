  /*
 * Author: Nikolaos Konstantinou
 * Project: Suzuki Jimny GSM System
 * Date 14/7/2020
 * Code Version: 1.0.0
 * Firmware Version: 1.0.0
 */
 
#include <SoftwareSerial.h> // Library for using serial communication
SoftwareSerial SIM900(7, 8); // RX, TX

#define MainPhoneNumber = //add phone here;
String data[5];

//GPS Strings to get the location
#define DEBUG true
String state,timegps,latitude,longitude,atmsg,msgi,gspeed;
bool bAdmin,bGetloc,bHelp,bClrmsg=0;
bool inverterStatus=0;

String incomingData;   
String message = "";  

int ignition = 2;
int window1 = 3;
int window2 = 4;
int horn = 5;
int lock = 6;    
int unlock = 9; 
int flashlight = 10;
int inverter = 11;
int GSM_Power = 12;


void setup()
{
  Serial.begin(19200); // baudrate for serial monitor

  pinMode(lock, OUTPUT);   // Setting erlay pin as output pin
  pinMode(unlock, OUTPUT); 
  pinMode(flashlight, OUTPUT);   // Setting erlay pin as output pin
  pinMode(horn, OUTPUT);
  pinMode(GSM_Power, OUTPUT);    //Pin to remote start GSM Modem
  pinMode(ignition, OUTPUT);
  pinMode(window1, OUTPUT);
  pinMode(window2, OUTPUT); 
  pinMode(inverter, OUTPUT);

  digitalWrite(ignition, HIGH);
  digitalWrite(inverter, HIGH);
  digitalWrite(window1, HIGH);
  digitalWrite(window2, HIGH);
  digitalWrite(lock, HIGH);
  digitalWrite(unlock, HIGH);
  digitalWrite(flashlight, HIGH);
  digitalWrite(horn, HIGH);
  
  GSM_Start_Up(); // Call the GSM Power Sequence
}

void GSM_Start_Up(){
  digitalWrite(GSM_Power, HIGH);
  delay(2000);
  digitalWrite(GSM_Power, LOW);
  
  delay(5000);
  SIM900.begin(19200); // baudrate for GSM shield
  
  SIM900.print("AT+CMGF=1\r");  // set SMS mode to text mode
  delay(500);
  
  SIM900.print("AT+CNMI=2,2,0,0,0\r"); // set gsm module to tp show the output on serial out
  delay(500);

  sendData("AT+CGNSPWR=1",1000,DEBUG);//Turn on GPS(GNSS - Global Navigation Satellite System)
  delay(500);
  
  sendData("AT+CGNSSEQ=RMC",1000,DEBUG);
  delay(500);
  
  sendData("AT+CGPSSTATUS?",1000,DEBUG);//this will check your GPS status. A working GPS should get either a 2D or 3D fix location
  delay(500);
}

void loop()
{
  
  receive_message();

  if(incomingData.indexOf("Car_doors_lock")>=0)
  {
    delay(150);
    digitalWrite(lock, LOW);
    delay(700);
    digitalWrite(lock, HIGH);
    message = "Doors Are Locked";
    send_message(message);
  }
  
  if(incomingData.indexOf("Car_doors_unlock")>=0)
  {
    delay(150);
    digitalWrite(unlock, LOW);
    delay(700);
    digitalWrite(unlock, HIGH);
    message = "Doors Are Unlocked";
    send_message(message);
  }        

/*==========================LighBar=======================================*/
  if(incomingData.indexOf("Car_flashlight_on")>=0){   
    delay(150);
    digitalWrite(flashlight, LOW);
    message = "FlashLight Activated";
    send_message(message);
   }

    if(incomingData.indexOf("Car_flashlight_off")>=0){   
    delay(150);
    digitalWrite(flashlight, HIGH);
    message = "FlashLight Deactivated";
    send_message(message);
   }

/*==========================Ignition=======================================*/
  if(incomingData.indexOf("Car_ignition_on")>=0){   
    delay(150);
    digitalWrite(ignition, LOW);
    delay(10000);
    digitalWrite(ignition, HIGH);
    message = "Ignition Activated - Systems Online";
    send_message(message);
   }

    if(incomingData.indexOf("Car_ignition_off")>=0){   //when the ability to start the car is added then turn off car manual
    delay(150);
    //digitalWrite(ignition, HIGH);
    message = "Ignition Deactivated - Systems Offline";
    send_message(message);
   }
/*==========================Ignition=======================================*/
   if(incomingData.indexOf("Car_inverter")>=0){   
      delay(150);
        if(inverterStatus == 0){
          digitalWrite(inverter, LOW);
          inverterStatus = 1;
          message = "Inverter Is Online";
          send_message(message);
   }else if (inverterStatus == 1){
           digitalWrite(inverter, HIGH);
           inverterStatus = 0;
           message = "Inverter Is Offline";
           send_message(message);
        }
   }

   /*==========================Windows Down=====================================*/
  if(incomingData.indexOf("Car_windows_down")>=0){ 
    delay(150);
    digitalWrite(ignition, LOW);
    delay(2000);
    digitalWrite(window2, LOW);
    delay(4000);
    digitalWrite(window2, HIGH);
    delay(4000);
    digitalWrite(ignition, HIGH);
    message = "Windows are going down";
    send_message(message);
   }
/*==========================Windows Up=====================================*/
    if(incomingData.indexOf("Car_windows_up")>=0){   
    delay(150);
    digitalWrite(ignition, LOW);
    delay(2000);
    digitalWrite(window1, LOW);
    delay(4000);
    digitalWrite(window1, HIGH);
    delay(4000);
    digitalWrite(ignition, HIGH);
    message = "Windows are going up";
    send_message(message);
   }
/*==========================Car Location=====================================*/
    if(incomingData.indexOf("Car_Location")>=0){   
      sendLocationOfCar();
   }

   if(incomingData.indexOf("Car_functions_list")>=0){   //This function will print all the functions of the car 
    message = "This Service in not yet available";
    send_message(message);
   }
}

void receive_message()
{
  if (SIM900.available() > 0)
  {
    incomingData = SIM900.readString(); // Get the data from the serial port.
    Serial.print(incomingData); 
    delay(10); 
  }
}

/*==========================Reply Back To Confirm The Command===================================*/
void send_message(String message)
{
  SIM900.println("AT+CMGF=1");    //Set the GSM Module in Text Mode
  delay(100);  
  SIM900.println("AT+CMGS=\"add phone here\""); // Replace it with your mobile number
  delay(100);
  SIM900.println(message);   // The SMS text you want to send
  delay(100);
  SIM900.println((char)26);  // ASCII code of CTRL+Z
  delay(100);
  SIM900.println();
  delay(1000);  
}



void sendLocationOfCar(){
  sendTabData("AT+CGNSINF",1000,DEBUG);
 
  //Serial.println("Latitude  :" + latitude);
  //Serial.println("Longitude  :" + longitude);

  message = ("http://maps.google.com/maps?q=loc:"+latitude+","+longitude);
  send_message(message);
  delay(10000);
  SIM900.flush();
  }

void sendTabData(String command , const int timeout , boolean debug){

SIM900.println(command);
long int time = millis();
int i = 0;

while((time+timeout) > millis()){
while(SIM900.available()){
char c = SIM900.read();
if (c != ',') {
data[i] +=c;
delay(300);
} else {
i++;
}
if (i == 5) {
delay(300);
goto exitL;
}
}
}exitL:
if (debug) {

latitude = data[3];
longitude = data[4];
}
}

String sendData (String command , const int timeout ,boolean debug){
String response = "";
SIM900.println(command);
long int time = millis();
int i = 0;

while ( (time+timeout ) > millis()){
while (SIM900.available()){
char c = SIM900.read();
response +=c;
}
}
if (debug) {
//Serial.print(response);
}
return response;
}
