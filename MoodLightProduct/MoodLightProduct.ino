#include <SoftwareSerial.h>
#include <Adafruit_NeoPixel.h>

#define DEBUG_MODE_ENABLE   1
#define BT_RENAME_ENABLE    0

// 네오픽셀 led개수 설정
#define NUM_OF_PIXEL 4
#define CONTROL_PIN 9

// 블루투스 모듈 11, 12번핀으로 사용
SoftwareSerial BT_Serial(11, 12);

//rgbneo 객체 생성
Adafruit_NeoPixel rgbneo = Adafruit_NeoPixel(NUM_OF_PIXEL, CONTROL_PIN, NEO_GRB + NEO_KHZ800);

//전역 변수 선언
int interval = 10; //ms
int sequence_Cycles = 0;
int rainbowCycle = 0;
int cmdMode,sig_setColorNeoPixel,sig_setDisco, sig_setSeq_color_change, sig_setRainbow;
int redColor, greenColor, blueColor;

//길이 제한이 없는 string 설정
unsigned long disco_PreviousMillis = 0;
unsigned long seq_Color_Change_PreviousMillis = 0;
unsigned long rainbow_PreviousMillis = 0;

//packet type 설정
String RGBString;

//data 전송 설정
void setup() {
  BT_Serial.begin(9600);
  
  #if(DEBUG_MODE_ENABLE)
    Serial.begin(9600);
    Serial.println("Setup initialized");
  #endif
  
  rgbneo.begin();
  rgbneo.show();
}

void loop(){
  color_control();
}

//main 함수
void color_control(void){
    
    //BlueTooth 통신 상태 확인
    if(BT_Serial.available()) {
          RGBString = BT_Serial.readStringUntil('\n');

          // 통신 성공 시 packet 반환
          #if(DEBUG_MODE_ENABLE)
            Serial.println(RGBString);
          #endif

          // 연결 혹은 해제 시 led off 상태로 만듦
          if((RGBString == "OK+LOST" || RGBString == "OK+CONN")) RGBString = "0200000000003";

          // packet_handler에 packet 정보 전송
          packet_handler(RGBString);

      // packet의 앞 두자리를 통해 모드를 바꿔줌
      switch(cmdMode){
        case 0: break;
        case 2: sig_setColorNeoPixel = 1; sig_setDisco = 0; sig_setSeq_color_change = 0; sig_setRainbow = 0; break; 
        case 3: sig_setColorNeoPixel = 0; sig_setDisco = 1; sig_setSeq_color_change = 0; sig_setRainbow = 0; break; 
        case 4: sig_setColorNeoPixel = 0; sig_setDisco = 0; sig_setSeq_color_change = 1; sig_setRainbow = 0; break; 
        case 6: sig_setColorNeoPixel = 0; sig_setDisco = 0; sig_setSeq_color_change = 0; sig_setRainbow = 1; break; 
        default :
        break;
        }
     }    

    //사용자가 지정한 색상을 받아 표기
    if(sig_setColorNeoPixel){setcontrolneopixel(NUM_OF_PIXEL);}

    //DISCO MODE
    if(sig_setDisco){if((unsigned long)millis()- disco_PreviousMillis >= interval){
        disco_PreviousMillis = millis();
        DiscoMode(NUM_OF_PIXEL);
      }
    }

    //순차적인 색상 변경
    if(sig_setSeq_color_change){
      if((unsigned long)millis()- seq_Color_Change_PreviousMillis >= interval){
        seq_Color_Change_PreviousMillis = millis();
        seqColorChangeEffect();
      }
    }

    //무지개 색상 출력 + 조도 조절
    if(sig_setRainbow){
      if((unsigned long)millis()-rainbow_PreviousMillis >= interval){
        rainbow_PreviousMillis = millis();
        rainbowEffect();
      }
    }
  }


  //string type으로 입력받은 packet data를 추출해 각 int type으로 변환하고 그 결과를 출력함
  void packet_handler(String rawPacketData){
    cmdMode = rawPacketData.substring(0,2).toInt();
    redColor = rawPacketData.substring(2,5).toInt();
    greenColor = rawPacketData.substring(5,8).toInt();
    blueColor = rawPacketData.substring(8,11).toInt();
    #if(DEBUG_MODE_ENABLE)
       Serial.print("cmdMode: ");      Serial.println(cmdMode);
       Serial.print("Red Value: ");    Serial.println(redColor);
       Serial.print("Green Value: ");  Serial.println(greenColor);
       Serial.print("Blue Value: ");   Serial.println(blueColor);
     #endif
  }


  //사용자가 설정한 색상 RGB값을 받아와 출력하는 함수
  void setcontrolneopixel(int num_of_pixel){
    for(int i = 0; i < num_of_pixel; i++){
      rgbneo.setPixelColor(i,redColor,greenColor,blueColor);
    }
    rgbneo.show();
  }

  // 랜덤으로 RGB값을 받아와 출력하는 함수
  void DiscoMode(int num_of_pixel){
    
    int x = random(0, 255);
    int y = random(0, 255);
    int z = random(0, 255);
    
    for(int i = 0; i < num_of_pixel; i++){
      rgbneo.setPixelColor(i, x, y, z);
    }
    rgbneo.show();

  }

  //순차적으로 RGB 값을 변경하여 출력하는 함수
  void seqColorChangeEffect(){
    for(int i = 0; i < rgbneo.numPixels(); i++){
      rgbneo.setPixelColor(i,Wheel((i+sequence_Cycles)&255));
    }
    rgbneo.show();
    sequence_Cycles++;

    //Cycle이 255가 넘어가면 0으로 초기화
    if(sequence_Cycles >= 256) {
      sequence_Cycles = 0;
    }
  }

  // 4개의 LED 모두 다른 색상을 출력하고 조도를 순서대로 바꿔주는 함수
  void rainbowEffect(){
    for(int i = 0; i < rgbneo.numPixels(); i++){
      rgbneo.setPixelColor(i,Wheel(((i * 256 / rgbneo.numPixels()) + rainbowCycle) & 255));
    }
    rgbneo.show();
    rainbowCycle++;
    
    //Cycle이 255가 넘어가면 0으로 초기화
    if(rainbowCycle >= 256*5) {
      rainbowCycle = 0;
    }
 }
 
//Wheel 함수
 uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return rgbneo.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return rgbneo.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return rgbneo.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
