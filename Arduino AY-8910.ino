#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <SD.h>

#define BC1 6
#define BCDIR 5
#define SS 3
#define pinRES 7

byte Buf[255];
int dobavka;
int rBuf;
int curFileNum,curFileNum2;
int Count;
long int fileSize;

SPISettings spiSettings(16000000, MSBFIRST, SPI_MODE0);
LiquidCrystal_I2C lcd(0x27, 20, 4);
File root;
File curFile,prvFile, nxtFile;





void setup() 
{
Serial.begin(115200);
lcd.begin();
SPI.begin();

//init pins

pinMode(SS, OUTPUT);
pinMode(BC1, OUTPUT);
pinMode(BCDIR, OUTPUT);
pinMode(pinRES, OUTPUT);


    resetAY();
    

if (!SD.begin(10)) 
{
    lcd.setCursor(0,0);
    lcd.println("SD CARD ERROR");
    Serial.println("SD CARD ERROR");
while (1);
}

Serial.println("Welcome back!");

root = SD.open("MUSIC/");

Serial.print ("Total files:");
Serial.println (folderLenght());
root.rewindDirectory();
curFile =  root.openNextFile();
fileSize = curFile.size();
}

void loop() 
{
GoNextFile();
GoNextFile();
GoNextFile();
GoNextFile();
playFile();
GoNextFile();

curFile.close();
prvFile.close();
}





void GoNextFile()
{

prvFile = curFile;
curFile =  root.openNextFile();
curFileNum++;
fileSize = curFile.size();
if (!curFile)
{
curFile = prvFile;
curFileNum--;
}

while (curFile.isDirectory())
{
curFile =  root.openNextFile();
curFileNum++;  

if (!curFile)
{
curFile = prvFile;
curFileNum--;
fileSize = curFile.size();
curFile.close();
prvFile.close();
break;
}
}
fileSize = curFile.size();
curFile.close();
prvFile.close();

}




void GoPrevFile()
{

root.rewindDirectory();
curFile =  root.openNextFile();

if (curFileNum ==0)
{
  curFile.close();
  prvFile.close();
  return;
  }
curFileNum2 = curFileNum;
curFileNum = 0;
for (int chapaem = 1; chapaem < curFileNum2; chapaem++)  
{
GoNextFile();
fileSize = curFile.size();
curFile.close();
}
curFile.close();
prvFile.close();
}



void GoRandFile()
{
root.rewindDirectory();
curFile =  root.openNextFile();
if (curFileNum ==0)
{
  curFile.close();
  prvFile.close();
  return;
  }
curFileNum = 0;
int randCount = random (1,Count);
//Serial.println (randCount);
for (int chapaem = 1; chapaem < randCount; chapaem++)  
{
GoNextFile();
fileSize = curFile.size();
curFile.close();

}
curFile.close();
prvFile.close();
}

int folderLenght()
{
Count=0;
root.rewindDirectory();
while (true) 
{
File entry =  root.openNextFile();
if (!entry) 
{
  root.rewindDirectory();
  entry.close();
  return Count;


}
Count++;
entry.close();
}
}





void playFile()
{
byte Register;
byte Value;
String filePath = curFile.name();
String folder = "MUSIC/";
filePath = folder += filePath;
float Pos = 0;
float songSize = round((curFile.size() - 16)/2);

resetAY();

Serial.print("Now Playing:");
Serial.println(filePath);


lcd.setCursor(0,0);
lcd.print("            ");
lcd.setCursor(0,0);
lcd.print(curFileNum+1);
lcd.print(".");
lcd.print(curFile.name());

lcd.setCursor(17,0);
lcd.print("  %");

lcd.setCursor(0,1);
lcd.print("            ");
lcd.setCursor(0,1);
lcd.print(fileSize);


File dataFile = SD.open(filePath);

    dataFile.seek(16); // Переходим к данным
        
//    while (1)
//                              {
dobavka = -1;

dataFile.read(Buf,256);

for (int plBuf = 0; plBuf < 256;plBuf++)
{

Register = Buf[plBuf];

if (Register != 255 and Register != 254)
{
Register = Buf[plBuf];
Value = Buf[plBuf+1];
}

writeAYRegister(Register, Value);
plBuf++;
if (plBuf = 255){dobavka = 0;}else if (plBuf = 254){dobavka = Register;}
}
else
              {            
              if (Register==255) {delay (20);
                                                  if (plBuf == 254){dobavka = Buf[255];}
              }
              
                else
                {
              Value = Buf[plBuf+1];
              delay(80*Value);
              plBuf++;
                                                  if (plBuf == 254){dobavka = Buf[255]);}

              }


              }

}

//                              }

}            


String TwoDigit (String abba)
{
if (abba.length()==1)
{
abba = "0" + abba;
}
return abba;
}


void resetAY()
{
 digitalWrite(pinRES, LOW); 
 delay (10);
 digitalWrite(pinRES, HIGH);

// Initialize the mixer of the AY
writeAYRegister(0x06, 0x00);
writeAYRegister(0x07, 0x3e);
writeAYRegister(0x08, 0x0f);

}


void writeAYRegister(uint8_t address, uint8_t data) {
  SPI.beginTransaction(spiSettings);

  digitalWrite(BC1, LOW);
  digitalWrite(BCDIR, LOW);
  //write address

  digitalWrite(SS, LOW);
  SPI.transfer(address);
  digitalWrite(SS, HIGH);

  digitalWrite(BC1, HIGH);
  digitalWrite(BCDIR, HIGH);

  digitalWrite(BC1, LOW);
  digitalWrite(BCDIR, LOW);

  //write data
  digitalWrite(BC1, LOW);
  digitalWrite(BCDIR, HIGH);

  digitalWrite(SS, LOW);
  SPI.transfer(data);
  digitalWrite(SS, HIGH);

  digitalWrite(BC1, LOW);
  digitalWrite(BCDIR, LOW);

  SPI.endTransaction();
}



