#include<SoftwareSerial.h>
SoftwareSerial mySerial(10,11);
String ownerID;
String sendTo;
String data;
void setup() {
  Serial.begin(115200);
  mySerial.begin(57600);
  Serial.print("Enter ID:");
  while(!Serial.available());
  ownerID = Serial.readStringUntil("\n");
  Serial.print("\nMy ID is : ");
  Serial.println(ownerID[0]);
}
void flushRx() {
  while(mySerial.available())
    uint8_t temp = mySerial.read();
}
int checkSum(String a) {
  int x = a[0];
  for(int i=1;i<a.length()-1;i++) {
    if(a[i] == '\n') continue;
    x = x + a[i];
    //Serial.println(a[i]);
    if(x>=128) {
      x-=127;
    }
  }
  //Serial.print("Master (check sum): ");
  //Serial.println(x);
  return x;
}
String P = "";
int frameNow = 1;
char prevData = '|';
void loop () {
  if(Serial.available()) {
    Serial.print("Reciver is : ");
    while(!Serial.available());
    sendTo = Serial.readStringUntil("\n");
    Serial.println(sendTo[0]);
    Serial.print("Enter Data : ");
    while(!Serial.available());
    data = Serial.readStringUntil("\n");
    Serial.println(String(ownerID[0]) + String(sendTo[0]) + data);
    for(int i=0;i<data.length();i++) {
      bool sended = false;
      bool first_send = true;
      while(!sended) {
        if(first_send) {
          first_send = false;
        }
        else {
          Serial.println("Timeout");
          Serial.println("Retransmit frame : " + String((frameNow+1)%2));
        }
        Serial.println("Send frame : " + String((frameNow+1)%2));
        Serial.println("Data : " + String(data[i]));
        mySerial.print("*" + String(ownerID[0]) + String(sendTo[0])+ String((frameNow+1)%2) + String(data[i]) + "#");
        Serial.println("Data : *" + String(ownerID[0]) + String(sendTo[0])+ String((frameNow+1)%2) + String(data[i]) + "#");
        unsigned long long now = millis();
        while(millis() - now < 1000 and !sended) {
          if(mySerial.available()) {
            int temp = mySerial.read();
            //Serial.println("YYYYYYYYYYYYYYY" + String(temp));
            if(temp == (frameNow+2)%2) {
              sended = true;
            }
          }
        }    
      }
      frameNow = (frameNow+1)%2;
    }
    mySerial.print(127-checkSum(data));
  }
  //Serial.print("wait");
  if(mySerial.available()) {
    char temp = mySerial.read();
    Serial.println("\0");
    int frameNo = -1;
    char data_now;
    if(temp == '*') {
      char header[10];
      header[0] = mySerial.read();
      delay(10);
      header[1] = mySerial.read();
      delay(10);
      //Serial.println(String(char(ownerID[0])));
      if(header[1] == ownerID[0]) {
        frameNo = mySerial.read()-'0';
        Serial.print(frameNow);
        Serial.print(" ");
        Serial.println(frameNo);
        if (frameNow != frameNo) {
          //Serial.println("Change frame" + String(frameNow));
          Serial.println("Receive frame");
          Serial.print("Header     : " + String(char(header[1])));
          Serial.println(char(header[0]));
          Serial.println("Frame No. : " + String(frameNo));
          data_now = char(mySerial.read());
          Serial.println("Data : " + String(data_now));
          Serial.println("Received");
          mySerial.read();
          Serial.println("Send ACK " + String((frameNo+1)%2));
          frameNow = (frameNo);
          mySerial.write((frameNow+1)%2);
          
          if(data_now == '\n') {
            P += prevData;
            P += '\n';
            Serial.print("Complete : " + P);
            String masterKey = mySerial.readStringUntil("\n");
            //String Key = String(checkSum(P));
            //Serial.println(masterKey);
            //Serial.println(P);
            //Serial.print(checkSum(P));
            if(127-(checkSum(P)+masterKey.toInt()) == 0) {
              Serial.println("Data Correct");
            }
            else {
              Serial.println("Data not Correct");
            }
            P = "";
            prevData = '|';
          }
          else if(prevData != '|') {
            P += prevData;
            prevData = data_now;
            //Serial.println("Save DATAAAAAAAAAAAA");
          }
          else {
            prevData = data_now;
          }
        }
        else {
          //Flush Data
          Serial.println("Flush RO DAH!!!");
          mySerial.write((frameNow+1)%2);
          //prevData = '|';
        }
      }
      else 
      {
        if(('A' <= header[0] and header[0] <= 'Z') and ('A' <= header[1] and header[1] <= 'Z')) {
          frameNo = mySerial.read()-'0';
          data_now = char(mySerial.read());
          mySerial.read();
          frameNow = (frameNo+2)%2;
          Serial.println("~" + String(char(header[0])) + String(char(header[1])) + String(frameNow));
        }
      }
    }
  }
}
