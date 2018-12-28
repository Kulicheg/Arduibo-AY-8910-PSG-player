////Pin connected to Data in (DS) of 74HC595
//const int dataPin = 10;
////Pin connected to latch pin (ST_CP) of 74HC595
//const int latchPin = 15;
////Pin connected to clock pin (SH_CP) of 74HC595
//const int clockPin = 14;

#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <SD.h>
#include <Button.h>
#include <TimerOne.h>
#include <Encod_er.h>

#define ENC_S1 8
#define ENC_S2 7
#define BUTTON_1_PIN 6
#define SDcard 9
#define SS 14
#define pinRES 15
#define BCDIR 16
#define BC1 17




byte Buf[256];
byte Register, Value;
int dobavka;
int rBuf;
int curFileNum, curFileNum2;
int Count;
int offset;
long int fileSize, songSize,  SizeBl;
String filePath, folder;
byte jdPos;

SPISettings spiSettings(16000000, MSBFIRST, SPI_MODE0);
LiquidCrystal_I2C lcd(0x27, 20, 4);
File root;
File curFile, prvFile, nxtFile;
Button button1(BUTTON_1_PIN, 4);
Encod_er encoder( ENC_S1, ENC_S2, 2);


void setup()
{
  
  digitalWrite(2, HIGH);
  Serial.begin(115200);
  lcd.begin();
  SPI.begin();
  Timer1.initialize(1000); // инициализация таймера 1, период 250 мкс
  Timer1.attachInterrupt(timerInterrupt, 250); // задаем обработчик пр




  //init pins

  pinMode(SS, OUTPUT);
  pinMode(BC1, OUTPUT);
  pinMode(BCDIR, OUTPUT);
  pinMode(pinRES, OUTPUT);
  pinMode(2, OUTPUT);

  resetAY();


  if (!SD.begin(SDcard))
  {
    lcd.setCursor(0, 0);
    lcd.println(F("SD CARD ERROR"));
    Serial.println(F("SD CARD ERROR"));
    while (1);
  }

  root = SD.open("MUSIC/");

  Serial.print (F("Total files:)"));
  Serial.println (folderLenght());
  root.rewindDirectory();
  curFile =  root.openNextFile();
  fileSize = curFile.size();
}




void loop()
{
  playFile();


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
      filePath = curFile.name();
      folder = "MUSIC/";
      filePath = folder += filePath;
      curFile.close();
      prvFile.close();
      break;
    }
  }
  fileSize = curFile.size();

  filePath = curFile.name();
  folder = "MUSIC/";
  filePath = folder += filePath;

  Serial.print (F("filePath = "));
  Serial.println (filePath);

  curFile.close();
  prvFile.close();

}




void GoPrevFile()
{

  root.rewindDirectory();
  curFile =  root.openNextFile();

  if (curFileNum == 0)
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
  if (curFileNum == 0)
  {
    curFile.close();
    prvFile.close();
    return;
  }
  curFileNum = 0;
  int randCount = random (1, Count);
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
  Count = 0;
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



  File dataFile = SD.open(filePath);

  dataFile.seek(16); // Переходим к данным

  Serial.print (F("Now Playing:"));
  Serial.print(curFileNum);
  Serial.print(".");
  Serial.println(dataFile.name());
  fileSize = dataFile.size();
  songSize = round((fileSize - 16) / 2);
  SizeBl = (fileSize - 16) / 256;
  long int LastBl = (fileSize - 16) - SizeBl * 256;


  Serial.print (F("dataFile.size() = "));
  Serial.println (fileSize);

  resetAY();


  lcd.setCursor(0, 0);
  lcd.print("            ");
  lcd.setCursor(0, 0);
  lcd.print(curFileNum + 1);
  lcd.print(".");
  lcd.print(curFile.name());

  lcd.setCursor(17, 0);
  lcd.print("  %");

  lcd.setCursor(0, 1);
  lcd.print("            ");
  lcd.setCursor(0, 1);
  lcd.print(fileSize);





  while (SizeBl >= 0)
  {
    dataFile.read(Buf, 256);

    if (dobavka == -3)
    {
      delay(80 * Buf[0]);
      offset = 1;
    }

    if (dobavka >= 0)
    {
      Register = dobavka;
      byte Value1 = Buf[0];
      writeAYRegister(Register, Value1);
      offset = 1;
    }

    dobavka = -1;
    int playDo = 256;

    if (SizeBl == 0) {
      playDo = LastBl;
    }

    for (int plBuf = 0; plBuf < playDo; plBuf++)
    {


      ////////////////////////////////////////////////
      button1.scanState();  // вызов метода ожидания стабильного состояния для кнопки 2

      if ( button1.flagClick == true ) {
        // был клик кнопки

        button1.flagClick = false;        // сброс признака

        Serial.println(F("Next Song"));
        dataFile.close();
        GoNextFile();
        return;
      }


      if (encoder.timeRight != 0) {
        Serial.println(F("Next Song"));
        encoder.timeRight = 0;

        dataFile.close();
        GoNextFile();
        return;


      }
      if (encoder.timeLeft != 0) {
        Serial.println(F("Previous Song"));
        encoder.timeLeft = 0;

        dataFile.close();
        GoPrevFile();
        return;


      }
      /////////////////////////////////////////////////


      plBuf = plBuf + offset;
      offset = 0;

      Register = Buf[plBuf];

      if (Register != 255 and Register != 254)
      {

        Register = Buf[plBuf];
        Value = Buf[plBuf + 1];
        digitalWrite(2, !digitalRead (2));

        if (plBuf < 255)
        { writeAYRegister(Register, Value);
          plBuf++;

        } else {
          dobavka = Buf[255];
          //plBuf++;

        }

      }
      else
      {
        if (Register == 255)
        {
          delay (20);
          dobavka = -1;
        }

        else
        {
          if (plBuf < 255)
          {
            writeAYRegister(Register, Value);
            Value = Buf[plBuf + 1];
            delay(80 * Value);
            plBuf++;
          } else {
            dobavka = -3;
          }
        }

      }

    }
    SizeBl = SizeBl - 1;
  }

  dataFile.close();
  GoNextFile();
}







String TwoDigit (String abba)
{
  if (abba.length() == 1)
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


void timerInterrupt()
{
  encoder.scanState();
}



