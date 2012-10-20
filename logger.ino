
/*
  loplogger
 
 Logger för temp och fukt 
 
 Ext irq för energimätning
 Pin 2 (irq 0)
 
 Ethernetmodul 
 Pin 10: 
 Pin 11:
 Pin 12:
 Pin 13:
 
 SD-kort
 Pin 4: SS
 
 Tempmoduler
 5
 6
 7
 8
 
 Status LED
 Pin 9
 
 */



// libraries for ethershield
#include <SPI.h>         
#include <Ethernet.h>
#include <EthernetUdp.h>		
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <util.h>


// libraries for realtime clock eller iaf tidkonverteringsfunktion
//#include <Wire.h>
//#include <RTClib.h>


//libraries for temperature sensors
#include "DHT22p.h"

// libraries for storage 
#include <SD.h>
const int chipSelect = 4;




DHT22p dht[4] = { DHT22p(5), DHT22p(6), DHT22p(7), DHT22p(8)};


// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0x00, 0x10, 0x01, 0x23, 0xAB, 0x30 }; // Use your own MAC address
byte ip[] = { 192, 168, 0, 100 };                  // no DHCP so we set our own IP address
byte subnet[] = { 255, 255, 255, 0 };             //subnet mask
byte gateway[] = { 192, 168, 0, 1 };             // internet access via router

unsigned int localPort = 8888;             // local port to listen for UDP packets
// find your local ntp server http://www.pool.ntp.org/zone/europe or 
// http://support.ntp.org/bin/view/Servers/StratumTwoTimeServers
// byte timeServer[] = {192, 43, 244, 18}; // time.nist.gov NTP server
IPAddress timeServer(193, 79, 237, 14);    // ntp1.nl.net NTP server  
//byte timeServer[] = {193, 228, 143, 13};    // ntp1.nl.net NTP server  
const int NTP_PACKET_SIZE= 48;             // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[NTP_PACKET_SIZE];                  // buffer to hold incoming and outgoing packets 

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;	

// Initialize the Ethernet server library
// with the IP address and port you want to use 

//EthernetServer server(80);


byte storageServer[]= {192, 168, 0, 150};
EthernetClient storageServerClient;





// timer inställning
volatile int counter_4_s=0;
volatile boolean time_to_sample;
volatile boolean time_to_save;
volatile byte samples=0;
volatile unsigned long time = 0;
int minutes = 0;


volatile int t_sum[4];
volatile int t_avg[4];

volatile unsigned int h_sum[4];
volatile unsigned int h_avg[4];
String dataString;
byte active_sensors;

volatile unsigned int e_sum = 0;
volatile unsigned int e_counter = 0 ;

ISR(TIMER1_OVF_vect) {
  //timer med en periodtid på 4 s
  TCNT1=0x0BDC; // set initial value to remove time error (16bit counter register)
   
 
  counter_4_s++;
  time+=4;
  if (counter_4_s==150) {  // 150 = var tionde minut
    for (int j=0;j<4;j++){
      t_avg[j]=t_sum[j]/samples;  // obs borde kanske slänga in någon felkoll här, har vi missat att sampla 4 gånger i rad så kan vi få division med 0 här
      h_avg[j]=h_sum[j]/samples;
      t_sum[j]=0;
      h_sum[j]=0;
    }
    counter_4_s=0;
    time_to_save=HIGH;
    samples=0;
    minutes++;
    e_sum= e_counter;
    e_counter = 0;

  } 
  else if ((counter_4_s & 0x1F) == 0x1F){  // 0x1F är 5 ettor längst till höger match på   31 63 95 127, dvs jämt utspridda till 150 s
    time_to_sample=HIGH;    
  } 
}


void setup() {
  pinMode(9, OUTPUT);  
  Serial.begin(9600);
  SPI.begin();
  Serial.println("logger");

  Serial.print("SD");
  pinMode(10, OUTPUT);
  if (!SD.begin(chipSelect)) {
    Serial.println(" fail");
    // don't do anything more:
    return;
  }
  Serial.println(" ok");
  
  
  // start the Ethernet connection and the server:
  Ethernet.begin(mac,ip,subnet,gateway);
  //server.begin();
  Udp.begin(localPort);

  delay(2000);  //the sensors need 2 s warmup after power-on
  
 
 
  
  check_sensors();
  
  // give the sensor and Ethernet shield time to set up:
  delay(1000);

  TIMSK1=0x01; // enabled global and timer overflow interrupt;
  TCCR1A = 0x00; // normal operation page 148 (mode0);
  TCNT1=0x0BDC; // set initial value to remove time error (16bit counter register)
  //time=1327963080;
  time=0;
  while( time == 0) {
    Serial.print("NTP "); 
    time = get_npt_time();
    //Serial.print("GMT: ");
   // PrintDateTime(DateTime(time));
   // Serial.println();
    //Serial.print("Unix t: ");
    Serial.println(time);
  } 
  
  TIMSK1=0x01; // enabled global and timer overflow interrupt;
  TCCR1A = 0x00; // normal operation page 148 (mode0);
  TCNT1=0x0BDC; // set initial value to remove time error (16bit counter register)
  TCCR1B = 0x05; // start timer/ set clock
  
  
  pinMode(2, INPUT_PULLUP);         
  attachInterrupt(0, e_blink, FALLING);
  
  Serial.println("init ok");
  

}



void loop() { 
  // listen for incoming Ethernet connections:
  //listenForClients();
  if (time_to_sample==HIGH){
    time_to_sample=LOW;
    digitalWrite(9, HIGH); 
    read_t_and_h();
    digitalWrite(9, LOW);
  }
  
  if (time_to_save==HIGH){
    time_to_save=LOW;
    //print_t_and_h();
    //Serial.println("saving to sd-card");
    save_to_file();
    transfer_data(0);
    transfer_data(1);
    transfer_data(2);
  
  /*if (minutes == 600) {
    //stäng av irq 
    Serial.print("Uppdaterar tiden: ");
    Serial.println(time);   
    while( time == 0) {
    Serial.println("Trying to get time from a NTP-server"); 
    time = get_npt_time();
    Serial.print("GMT: ");
   // PrintDateTime(DateTime(time));
   // Serial.println();
    Serial.print("Unix timestamp: ");
    Serial.println(time);
    //sätt på irq
  } */
  
  }
}

void e_blink() {
  e_counter++;
}




void check_sensors() {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  DHT22p_ERROR_t errorCode;
  Serial.print("Sens ");
  
  for (int i=0;i<4;i++) {
    errorCode = dht[i].readData();
    
    if (errorCode==DHT_ERROR_NONE) {
      active_sensors += (1 << i);
      Serial.print(i);
    } else {
      Serial.print("e");
      Serial.print(errorCode);
    }
  }
  Serial.println("");
}



void read_t_and_h() {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  double h[4];
  double t[4];
  DHT22p_ERROR_t errorCode;
  boolean readingValid=true;
  
  for (int i=0;i<4;i++) {
    if(active_sensors&(1<<i)) {
      errorCode = dht[i].readData();  
      if(errorCode==DHT_ERROR_NONE) {
        h[i] = dht[i].humidity;
        t[i] = dht[i].temperature;
      } else {
        readingValid=false;
      }
    }
  }
  
  // check if returns are valid, if they are NaN (not a number) then something went wrong!
  if (readingValid) {
    for (int k=0;k<4;k++){
      t_sum[k]=t_sum[k]+int(t[k]*100);
      h_sum[k]=h_sum[k]+int(h[k]*100);
    }
    samples++;
  
  } else {
    Serial.println("Fail rd DHT");
  } 

  
}





void save_to_file() {
  
  File dataFile = SD.open("loplogg.txt", FILE_WRITE);
  if (dataFile) {
    char datastr[50];
    sprintf(datastr, "%lu,%4i,%4i,%4i,%4i,%4i,%4i,%4i,%4i,%5i ", time,t_avg[0],t_avg[1],t_avg[2],t_avg[3],h_avg[0],h_avg[1],h_avg[2],h_avg[3],e_sum);
    dataFile.println(datastr);    
    dataFile.close();
    Serial.println(datastr);
  }  
  // if the file isn't open, pop up an error:
  else {
    Serial.println("SD err");
  } 
}



 

 
unsigned long get_npt_time() {
  // send an NTP packet to a time server
  sendNTPpacket(timeServer);

  // wait to see if a reply is available
  delay(1000);
  if ( Udp.parsePacket() ) {  
    // We've received a packet, read the data from it
    Udp.read(packetBuffer,NTP_PACKET_SIZE);  // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);  
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;  

     // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;     
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;  


    return epoch;
  }
  else
  {
    delay(60000L);
    return 0;
    
  }
}
    

/*void PrintDateTime(DateTime t)
{
    char datestr[24];
    sprintf(datestr, "%04d-%02d-%02d  %02d:%02d:%02d  ", t.year(), t.month(), t.day(), t.hour(), t.minute(), t.second());
    Serial.print(datestr);  
}
*/

// send an NTP request to the time server at the given address 
unsigned long sendNTPpacket(IPAddress address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE); 
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49; 
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:         
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer,NTP_PACKET_SIZE);
  Udp.endPacket(); 
}


void transfer_data(byte set){
  String www = "www";
  String err = " err";
  Serial.print(www);
  
  if (storageServerClient.connect(storageServer, 80)) {
    //Serial.println("ected");
    char getstr[105];   // 24+10+8x(4+5)+1=105   24+10+4x(4+5)+1=71 
    if ( set == 0 ) {
      sprintf(getstr, "GET /update_db.php?Time=%lu&T1=%i&T2=%i&T3=%i&T4=%i ", time,t_avg[0],t_avg[1],t_avg[2],t_avg[3]);
    } else if (set == 1) {
      sprintf(getstr, "GET /update_db.php?Time=%lu&H1=%i&H2=%i&H3=%i&H4=%i ", time,h_avg[0],h_avg[1],h_avg[2],h_avg[3]);
    } else if (set == 2) {
      sprintf(getstr, "GET /update_db.php?Time=%lu&E=%i ", time,e_sum);
    }
    //sprintf(getstr, "GET /update_db.php?Time=%lu&T1=%i&T2=%i&T3=%i&T4=%i&H1=%i&H2=%i&H3=%i&H4=%i ", time,t_avg[0],t_avg[1],t_avg[2],t_avg[3],h_avg[0],h_avg[1],h_avg[2],h_avg[3]);
    storageServerClient.println(getstr);
    storageServerClient.println();
    

    Serial.println(" ok");
    storageServerClient.stop();
  } else {
    Serial.println(err);
    
    File dataFile = SD.open("loplogg.log", FILE_WRITE);
      if (dataFile) {
      char datastr[10];
      sprintf(datastr, "%lu ", time);
      dataFile.print(www);
      dataFile.print(err);
      dataFile.println(datastr);    
      dataFile.close();
      Serial.println(datastr);
  }  
  // if the file isn't open, pop up an error:
  else {
    Serial.println("SD err");
  } 
  }
 
  
}
///////////////////////////////////////////
//
// End of program
//

