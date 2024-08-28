#define  CLOCK  9
#define  DATA   8
#define  MCLR   7

//#define DEBUG

#ifdef DEBUG
#define  CLOCK_ANALYZER  13
#define  DATA_ANALYZER   12
#define  MCLR_ANALYZER   11
#endif

#define  VPP1   2
#define  VPP2   3
#define  VPP3   4

#define TENTH 250
#define TDLY 5
#define TERAB 250
#define TPINT 5


void setup() {
 Serial.begin(115200);

 pinMode(CLOCK, OUTPUT);    
 pinMode(DATA, OUTPUT);   
 pinMode(VPP1, OUTPUT);
 pinMode(VPP2, OUTPUT); 
 pinMode(VPP3, OUTPUT);
 pinMode(MCLR, OUTPUT);

#ifdef DEBUG
 pinMode(CLOCK_ANALYZER, OUTPUT);    
 pinMode(DATA_ANALYZER, OUTPUT);   
 pinMode(MCLR_ANALYZER, OUTPUT);
#endif

 Reset();
}

void WriteBit(char obit)
{
 digitalWrite(CLOCK, HIGH);
 #ifdef DEBUG
 digitalWrite(CLOCK_ANALYZER, HIGH);
 #endif
 if(obit)
 {
   digitalWrite(DATA,HIGH);
   #ifdef DEBUG
   digitalWrite(DATA_ANALYZER,HIGH);
   #endif
 }else{
   digitalWrite(DATA,LOW); 
   #ifdef DEBUG
   digitalWrite(DATA_ANALYZER,LOW); 
   #endif
 }
 digitalWrite(CLOCK, LOW);
 #ifdef DEBUG
 digitalWrite(CLOCK_ANALYZER, LOW);
 #endif
 digitalWrite(DATA,LOW); 
 #ifdef DEBUG
 digitalWrite(DATA_ANALYZER,LOW); 
 #endif
}

unsigned char ReadBit()
{
 unsigned char val = 0;
 digitalWrite(CLOCK, HIGH);
 #ifdef DEBUG
 digitalWrite(CLOCK_ANALYZER, HIGH);
 #endif
 if(digitalRead(DATA))
 {
  #ifdef DEBUG
  digitalWrite(DATA_ANALYZER, HIGH);
  #endif
  val = 1; 
 }
 #ifdef DEBUG
 else {
  digitalWrite(DATA_ANALYZER, LOW);
 }
 #endif
 digitalWrite(CLOCK, LOW);
 #ifdef DEBUG
 digitalWrite(CLOCK_ANALYZER, LOW);
 #endif
 return val;
}

void WriteByteLSB(unsigned char _byte, char nbits){
  for(unsigned char i=0; i<nbits; i++){
    WriteBit((_byte&(1<<i))>>i);
  }
}

void WriteShortLSB(unsigned short _short, char nbits){
  for(unsigned char i=0; i<nbits; i++){
    WriteBit((_short&(1<<i))>>i);
  }
}

void EnterProg(){
  WriteByteLSB('P', 8);
  WriteByteLSB('H', 8);
  WriteByteLSB('C', 8);
  WriteByteLSB('M', 8);
  WriteBit(0);
  delayMicroseconds(TDLY);
}

void LoadConf(unsigned short data){
 unsigned char cmd = 0x00; 
 WriteByteLSB(cmd, 6);
 delayMicroseconds(TDLY);
 WriteShortLSB(data<<1, 16);
 delayMicroseconds(TDLY);
}

void LoadProg(unsigned short data)
{
 unsigned char cmd = 0x02;
 WriteByteLSB(cmd, 6);
 delayMicroseconds(TDLY);
 WriteShortLSB(data<<1, 16);
 delayMicroseconds(TDLY);
}

unsigned short ReadProgMem(){
 unsigned char cmd = 0x04;
 unsigned short out = 0;
 WriteByteLSB(cmd, 6);
 pinMode(DATA, INPUT);
 digitalWrite(DATA, LOW);
 #ifdef DEBUG
 digitalWrite(DATA_ANALYZER, LOW);
 #endif
 delayMicroseconds(TDLY);
 ReadBit();
 for(int i=0;i<14;i++)
 {
   out |= ReadBit()<<i;
 }
 ReadBit();
 pinMode(DATA, OUTPUT);  
 delayMicroseconds(TDLY);
 return out;
}

void IncrementAddress()
{
 unsigned char cmd = 0x06;
 WriteByteLSB(cmd, 6);
 delayMicroseconds(TDLY);
}

void ResetAddress(){
  unsigned char cmd = 0x16;
  WriteByteLSB(cmd, 6);
  delayMicroseconds(TDLY);
}

void WriteProg()
{
 unsigned char cmd = 0x08; 
 WriteByteLSB(cmd, 6);
 delay(TPINT);
}

void BulkErase()
{
 unsigned char cmd = 0x09;
 WriteByteLSB(cmd, 6);
 delay(TERAB);
}

void PrintWordHex(unsigned short data){
    if(data<0x1000){
      Serial.print("0");
    }
    if(data<0x100){
      Serial.print("0");
    }
    if(data<0x10){
      Serial.print("0");
    }    
    Serial.print(data, HEX);
}

void ReadProgAll(){
  ResetAddress();
  for(int addr = 0; addr<0x2000; addr++){
    if(addr%32==0){
      Serial.println(" ");
      Serial.print("#");
      PrintWordHex(addr);
      Serial.print(":\t");
    }
    unsigned short data = ReadProgMem();
    IncrementAddress();
    PrintWordHex(data);
    Serial.print(" ");
    Serial.flush();
  }
}

void ReadProgAllRaw(){
  ResetAddress();
  for(int addr = 0; addr<0x2000; addr++){
    unsigned short data = ReadProgMem();
    IncrementAddress();
    PrintWordHex(data);
    Serial.flush();
  }
}

void ReadConfAll(){
  LoadConf(0);
  Serial.print("User ID = ");
  unsigned short cnt, data;
  for(cnt = 0; cnt<4; cnt++){
    data = ReadProgMem();
    IncrementAddress();
    PrintWordHex(data);
    Serial.print(" ");
  }
  IncrementAddress();

  data = ReadProgMem();
  IncrementAddress();
  Serial.print(" Revision ID = ");
  PrintWordHex(data);  
  data = ReadProgMem();
  IncrementAddress();
  Serial.print(" Device ID = ");
  PrintWordHex(data);

  Serial.print(" Configuration = ");
  data = ReadProgMem();
  IncrementAddress();
  PrintWordHex(data);
  Serial.print(" ");
  data = ReadProgMem();
  IncrementAddress();
  PrintWordHex(data);

  Serial.print(" Calibration = ");
  data = ReadProgMem();
  IncrementAddress();
  PrintWordHex(data);
  Serial.print(" ");
  data = ReadProgMem();
  IncrementAddress();
  PrintWordHex(data);
  Serial.println("");
}

void Reset(){
 pinMode(MCLR, OUTPUT);
 digitalWrite(MCLR, LOW);
 digitalWrite(VPP1, LOW);
 digitalWrite(VPP2, LOW);
 digitalWrite(VPP3, LOW);
 digitalWrite(CLOCK, LOW);
 digitalWrite(DATA, LOW);
#ifdef DEBUG
 pinMode(MCLR_ANALYZER, OUTPUT);
 digitalWrite(MCLR_ANALYZER, HIGH);
 digitalWrite(CLOCK_ANALYZER, LOW);
 digitalWrite(DATA_ANALYZER, LOW);
#endif
 delayMicroseconds(TENTH);
 digitalWrite(VPP1, HIGH);
 digitalWrite(VPP2, HIGH);
 digitalWrite(VPP3, HIGH);
 delayMicroseconds(TENTH);

 EnterProg();
}

void ExitProg(){
 pinMode(MCLR, INPUT);
 digitalWrite(VPP1, LOW);
 digitalWrite(VPP2, LOW);
 digitalWrite(VPP3, LOW);
 digitalWrite(CLOCK, LOW);
 digitalWrite(DATA, LOW);
#ifdef DEBUG
 digitalWrite(CLOCK_ANALYZER, LOW);
 digitalWrite(DATA_ANALYZER, LOW);
#endif
 delayMicroseconds(TENTH);
 digitalWrite(VPP1, HIGH);
 digitalWrite(VPP2, HIGH);
 digitalWrite(VPP3, HIGH);
 delayMicroseconds(TENTH);
}

int hexToInt(unsigned char hex){
  if(hex>='0' && hex<='9'){
    return hex-'0';
  }
  if(hex>='a' && hex<='f'){
    return 10+hex-'a';
  }
  if(hex>='A' && hex<='F'){
    return 10+hex-'A';
  }
}

void loop() {
  delayMicroseconds(10);
  if (Serial.available() > 0){
    char cmd =  Serial.read();
    switch(cmd){
    case 'r':
      ReadProgAll();
      break;
    case 'c':
      ReadConfAll();
      break;
    case 'q':
      Reset();
      break;
    case '1':
      ResetAddress();
      break;
    case '2':
      BulkErase();
      break;
    case '3':{
      while(Serial.available() < 4){}
      unsigned int data = 0;
      unsigned char hex =  Serial.read();
      data |= hexToInt(hex)<<12;
      hex =  Serial.read();
      data |= hexToInt(hex)<<8;
      hex =  Serial.read();
      data |= hexToInt(hex)<<4;
      hex =  Serial.read();
      data |= hexToInt(hex);
      LoadProg(data); 
      WriteProg();       
      break;
    }
    case '4':
      WriteProg();
      break;
    case '5':
      IncrementAddress();
      break;
    case '6':
      ReadProgAllRaw();
      break;
    case '7':
      ExitProg();
      break;
    case '8':{
      while(Serial.available() < 5){}
      unsigned int data = 0;
      unsigned int offset = 0;
      unsigned char hex =  Serial.read();
      data |= hexToInt(hex)<<12;
      hex =  Serial.read();
      data |= hexToInt(hex)<<8;
      hex =  Serial.read();
      data |= hexToInt(hex)<<4;
      hex =  Serial.read();
      data |= hexToInt(hex);
      hex =  Serial.read();
      offset = hexToInt(hex);
      LoadConf(data);
      while(offset--){
        IncrementAddress();
      }
      WriteProg();
      data = ReadProgMem();
      if(data<0x1000){
        Serial.print("0");
      }
      if(data<0x100){
        Serial.print("0");
      }
      if(data<0x10){
        Serial.print("0");
      }    
      Serial.print(data, HEX);      
      break;
    }
    default:
      break;
    }
    Serial.print(">"); 
    Serial.flush();
  }
}
