#include   <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>         // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#include "FastLED.h"
//led pin
#define DATA_PIN 5
/////////////////////
#define short_get_high_byte(x) ((HIGH_BYTE & x) >> 8)
#define short_get_low_byte(x)  (LOW_BYTE & x)
#define bytes_to_short(h,l) ( ((h << 8) & 0xff00) | (l & 0x00FF) );

byte mac[] = {0x90, 0xA2, 0xDA, 0x0D, 0x4C, 0x8C} ; //the mac adress in HEX of ethernet shield or uno shield board
byte ip[] = {192, 168, 0, 100}; // the IP adress of your device, that should be in same universe of the network you are using

// the next two variables are set when a packet is received
byte remoteIp[4];        // holds received packet's originating IP
unsigned int remotePort; // holds received packet's originating port

//customisation: Artnet SubnetID + UniverseID
//edit this with SubnetID + UniverseID you want to receive 
byte SubnetID = {1};
byte UniverseID = {1};
byte UniverseID2 = {2};
byte UniverseID3 = {3};

short select_universe= ((SubnetID*16)+UniverseID);
short select_universe2= ((SubnetID*16)+UniverseID2);
short select_universe3= ((SubnetID*16)+UniverseID3);
//customisation: edit this if you want for example read and copy only 4 or 6 channels from channel 12 or 48 or whatever.
const int number_of_channels=512; //512 for 512 channels
const int start_address=0; // 0 if you want to read from channel 1

//buffers
const int MAX_BUFFER_UDP=730;
char packetBuffer[MAX_BUFFER_UDP]; //buffer to store incoming data


// art net parameters
     // artnet UDP port is by default 6454
const int art_net_header_size=17;
const int max_packet_size=576;
char ArtNetHead[8]="Art-Net";
char OpHbyteReceive=0;
char OpLbyteReceive=0;
//short is_artnet_version_1=0;
//short is_artnet_version_2=0;
//short seq_artnet=0;
//short artnet_physical=0;
short incoming_universe=0;
boolean is_opcode_is_dmx=0;
boolean is_opcode_is_artpoll=0;
boolean match_artnet=1;
short Opcode=0;
EthernetUDP Udp;

//leds
#define NUM_LEDS 290
CRGB leds[NUM_LEDS];



void setup() {
      Serial.begin(115200);
        //setup pins as PWM output
      
        //setup ethernet and udp socket
        Ethernet.begin(mac,ip);
        Udp.begin(6454);
      
      Serial.print("server is at ");
      Serial.println(Ethernet.localIP());
        FastLED.addLeds<WS2811, DATA_PIN, BRG>(leds, NUM_LEDS);
}

void loop() {
  Serial.println(Ethernet.localIP());
  int packetSize = Udp.parsePacket();
  
  //FIXME: test/debug check
  if(packetSize>art_net_header_size && packetSize<=max_packet_size) {//check size to avoid unneeded checks

        
    IPAddress remote = Udp.remoteIP();    
    remotePort = Udp.remotePort();
    Udp.read(packetBuffer,MAX_BUFFER_UDP);
    
    //read header
    match_artnet=1;
    for (int i=0;i<7;i++) {
      //if not corresponding, this is not an artnet packet, so we stop reading
      if(char(packetBuffer[i])!=ArtNetHead[i]) {
        match_artnet=0;break;
      } 
    } 
       
     //if its an artnet header
    if(match_artnet==1) { 
        //artnet protocole revision, not really needed
        //is_artnet_version_1=packetBuffer[10]; 
        //is_artnet_version_2=packetBuffer[11];*/
      
        //sequence of data, to avoid lost packets on routeurs
        //seq_artnet=packetBuffer[12];*/
          
        //physical port of  dmx N°
        //artnet_physical=packetBuffer[13];*/
        
      //operator code enables to know wich type of message Art-Net it is
      Opcode=bytes_to_short(packetBuffer[9],packetBuffer[8]);
       
      //if opcode is DMX type
      if(Opcode==0x5000) {
        is_opcode_is_dmx=1;is_opcode_is_artpoll=0;
      }   
       
      //if opcode is artpoll 
      else if(Opcode==0x2000) {
         is_opcode_is_artpoll=1;is_opcode_is_dmx=0;
         //( we should normally reply to it, giving ip adress of the device)
      } 
       
      //if its DMX data we will read it now
      if(is_opcode_is_dmx=1) {
         
         //read incoming universe
         incoming_universe= bytes_to_short(packetBuffer[15],packetBuffer[14])
         //if it is selected universe DMX will be read
        if(incoming_universe==select_universe) setLEDS(0);
        if(incoming_universe==select_universe2) setLEDS(172);
        if(incoming_universe==select_universe3) setLEDS(341);

      
         
      }
    }//end of sniffing


 


     
  }  
     FastLED.show();
}

void setLEDS(int startled){
  int j=startled;
            for(int i=start_address;i< number_of_channels;i=i+3) {
              if (j<=NUM_LEDS){
                leds[j].r = byte(packetBuffer[i+art_net_header_size+1]);
                leds[j].g = byte(packetBuffer[i+art_net_header_size+2]);
                leds[j].b = byte(packetBuffer[i+art_net_header_size+3]);
              }
              j++;
           // buffer_channel_arduino[i-start_address]= byte(packetBuffer[i+art_net_header_size+1]);
          }
  
}
