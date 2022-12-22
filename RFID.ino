#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <EEPROM.h>
 
#define RST_PIN   9
#define SS_PIN    10
#define LED_GREEN 4
#define LED_RED   5
int button = 3;

Servo SG90; 
MFRC522 rc522(SS_PIN, RST_PIN);

int sg90 = 6; // 서보
int key =0;
int key_array[5] = {0,0,0,0};
int member_uid[50][4] = {};
int mod = 0;  //0 : 확인 모드/ 1 : 등록 모드
int mod_flag = 0;
int member_count = 0;
void setup(){
  Serial.begin(9600);
  SPI.begin();
  rc522.PCD_Init();
  
  SG90.attach(sg90);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(button, INPUT_PULLUP);
  pinMode(2,OUTPUT);//릴레이
  //EEPROM 값 불러오기
  member_count = EEPROM.read(0);//0번째 주소는 member_count 저장됨
  int EEPROM_counter = 1;
  for(int l = 1; l<50; l++){
    for(int n = 0; n < 4; n++){
      member_uid[l][n] = EEPROM.read(EEPROM_counter);
      EEPROM_counter++;
    }
  }            
}
//카드 확인 함수  / 등록된 카드 : 리턴값 1, 미등록 카드 : 리턴값 0
int check_f(int n1, int n2, int n3, int n4){
  int uid_array[5] ={n1,n2,n3,n4};
  for(int i = 0; i < 30; i++){
    for(int j = 0; j < 4; j++){
      key_array[j] = uid_array[j] == member_uid[i][j] ? 1 : 0;
    }
    for(int k = 0; k < 4; k++){
      if(key_array[k]==0){key=0;break;}
      else{key=1;}
    }
    if(key == 1){return 1;}
  }
  if(key == 0){return 0;}
}
//카드 저장 함수
int card_save(int n1, int n2, int n3, int n4){
  int address_counter = member_count * 4 + 1;
  member_uid[member_count][0] = n1;
  EEPROM.write(address_counter,n1);
  address_counter++;

  member_uid[member_count][1] = n2;
  EEPROM.write(address_counter,n2);
  address_counter++;

  member_uid[member_count][2] = n3;
  EEPROM.write(address_counter,n3);
  address_counter++;

  member_uid[member_count][3] = n4;
  EEPROM.write(address_counter,n4);
  address_counter++;
  
  member_count++;
  EEPROM.write(0,member_count);
}
void loop(){
  //버튼 토글 관리
  if(digitalRead(button)==0){
    if(mod_flag==0){
      mod_flag = 1;
      mod = !mod;
    }
  }
  else{
    if(mod_flag==1){
      mod_flag = 0;
    }
  }
  //LED 제어
  if(mod==0){
    digitalWrite(LED_RED,HIGH);
    digitalWrite(LED_GREEN,LOW);
  }
  else{
    digitalWrite(LED_RED,LOW);
    digitalWrite(LED_GREEN,HIGH);
  }
  //카드 읽어오기
  if ( !rc522.PICC_IsNewCardPresent() || !rc522.PICC_ReadCardSerial() ) { 
    //카드 또는 ID 가 읽히지 않으면 return을 통해 다시 시작하게 됩니다.
    delay(500);
    return;
  }

  Serial.print("Card UID:");
  //card UID 출력
  for (byte i = 0; i < 4; i++) {
    Serial.print(rc522.uid.uidByte[i]);
    Serial.print(" ");
  }
  Serial.println(" ");
  
  //스피커 소리내기
  tone(7,300,250);

  //mod로 인한 분기
  if(mod == 0){
    //키 만들기
    key = check_f(rc522.uid.uidByte[0], rc522.uid.uidByte[1], rc522.uid.uidByte[2], rc522.uid.uidByte[3]);

    //key 확인
    if(key==1){
      Serial.println("<< OK >>  Registered card...");
      digitalWrite(2,HIGH);
      delay(500);
      SG90.write(30);
      delay(500); 
      key = 0;
      SG90.write(0);
      digitalWrite(2,LOW);
    }
    else{
      delay(100);
      tone(7,300,250);
      Serial.println("<<!>>  This card is not registered");
      delay(500);
      key =0;
      tone(7,300,250);
    }
    delay(100);
  }
  else{
    key = check_f(rc522.uid.uidByte[0], rc522.uid.uidByte[1], rc522.uid.uidByte[2], rc522.uid.uidByte[3]);
    if(key == 1){
      Serial.println("<<!>>This Card already registered...");
      delay(100);
      tone(7,300,250);
    }
    else{
      card_save(rc522.uid.uidByte[0], rc522.uid.uidByte[1], rc522.uid.uidByte[2], rc522.uid.uidByte[3]);
      Serial.println("<< OK >> Card Saved!");
      Serial.print("Saved cards : ");
      Serial.print(member_count);
      tone(7,300,250);
      delay(100);
    }
  }
}