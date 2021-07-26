#include <ESP8266WiFi.h>                                               
#include <FirebaseArduino.h>                                        
 
#define FIREBASE_HOST "automated-gates.firebaseio.com"              // the project name address from firebase id
#define FIREBASE_AUTH "L5nt25n0U7u8hJde9QXXmoMxF6cg3PMpuGpgmX6A"       // the secret key generated from firebase
#define WIFI_SSID "Nilkanth"                                          //WIFI NAME
#define WIFI_PASSWORD "Moonwalker"                                    //WIFI PASSKEY
String fireStatus = "";


int ECHO=4;
int Trig=5;
int led=12;
int motorPin2  = 13;
int motorPin1  = 12;

int GateClosePin = 10;
int GateOpenPin = 9;

int flag=0;
int dist;
int dist1=15;
int distc=0;
int i;
int gatec=1,gateo=1;    //1="OFF", 0="ON"
bool IsClosed = true;

void setup()
{
  
  Serial.begin(9600);
  
  pinMode(GateClosePin, INPUT_PULLUP);
  pinMode(GateOpenPin, INPUT_PULLUP);
  
  delay(10);                 
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);                               
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) 
  {       
    Serial.print("*");
    delay(500);
  }
  Serial.println();
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.println("Connecting to firebase....");
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH); // connect to firebase
  Serial.println("Connected to firebase...!");
  
  Firebase.setString("OPEN_GATE","OFF");      //updateing firebase values
  Firebase.setString("CLOSE_GATE","OFF");

  
  pinMode (led,OUTPUT);
  pinMode (ECHO,INPUT);
  pinMode (Trig,OUTPUT);
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);

}

bool isGateClosed(){
  return digitalRead(GateClosePin);
}

bool isGateOpen(){
  return digitalRead(GateOpenPin);
}
uint64_t Time1 = 0;               //Unsigned 64 bit integer to prevent overflow
void loop()
{
  updateFirebase();
  if(gateo==0)
  {
    Serial.println("opening gate");
    gateo=1;
    gateOpen();
  }
  if(gatec==0)
  {
    Serial.println("closing gate");
    gateClose(); 
  }

  //Auto off after 5 sec
  if((millis() - Time1) > 5000 && (!IsClosed) ){
    gateClose();
  }
}


void gateClose()
{
  while(!isGateClosed())
  {
    digitalWrite(motorPin1, HIGH);  //anticlockwise-close
    digitalWrite(motorPin2, LOW);
    delay(200);
    digitalWrite(motorPin1, LOW);
    //delay(1000);
    //digitalWrite(motorPin2, LOW);
    ultra();
    if(dist<27)
    {
      Serial.println("Obstacle detected");
      break; 
    }
  }  
  if(!isGateClosed())
  {
    Serial.println("Opening gate again...");
    while(!isGateOpen())
    {
      digitalWrite(motorPin1, LOW);  //clockwise-open
      digitalWrite(motorPin2, HIGH);
      delay(200);
      digitalWrite(motorPin2, LOW);
    }
    Serial.println("Attempting to close again...");
  }
  else
  {
    Serial.println("Gate fully closed");
    IsClosed = true;
    Firebase.setString("CLOSE_GATE","OFF");
    gatec=1;
  }
  digitalWrite(motorPin2, LOW);
  digitalWrite(motorPin1, LOW);
  delay(6000);
}


void gateOpen()
{
  while(!isGateOpen())
  {
    digitalWrite(motorPin1, LOW);  //anticlockwise-close
    digitalWrite(motorPin2, HIGH);
    delay(200);
    digitalWrite(motorPin2, LOW);
  }
  Time1 = millis();
  IsClosed =  false;
  digitalWrite(motorPin2, LOW);
  digitalWrite(motorPin1, LOW);
  Serial.println("Gate fully open");
  Firebase.setString("OPEN_GATE","OFF");
}


void ultra()
{
  digitalWrite(Trig,LOW);
  delay(5);
  digitalWrite(Trig,HIGH);
  delay(50);
  digitalWrite(Trig,LOW);
  int duration= pulseIn(ECHO,HIGH);
  dist= duration/29/2;
  //Serial.println(dist);
}

void updateFirebase()
{
  Serial.println("Updating Firebase");
  fireStatus = Firebase.getString("OPEN_GATE");
  Serial.println("OPEN GATE: "+fireStatus);
  if(fireStatus=="ON")
  {
    gateo=0;
  }
  else if(fireStatus=="OFF")
  {
    gateo=1;
  }
  
  fireStatus = Firebase.getString("CLOSE_GATE");
  Serial.println("CLOSE GATE: "+fireStatus);
  if(fireStatus=="ON")
  {
    gatec=0;
  }
  else if(fireStatus=="OFF")
  {
    gatec=1;
  }
}
