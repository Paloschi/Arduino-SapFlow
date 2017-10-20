#include <SPI.h>
#include "Nanoshield_Termopar.h"
#include<stdlib.h>

//Nanoshield_Termopar tc(2, TC_TYPE_T, TC_AVG_OFF);
Nanoshield_Termopar tc(2, TC_TYPE_VOLTAGE_GAIN_32, TC_AVG_OFF);

#include <DS1307.h>
#include <SD.h>
DS1307 rtc(A4, A5);

const int chipSelect = 10;
const int tempo_leitura_segundos = 600;

void printErrorsTc() {
  // Essa função foi criada para mostrar os erros do termopar
  if (tc.isOpen()) {
    Serial.println(" Open circuit");
  } else if (tc.isOverUnderVoltage()) {
    Serial.println(" Overvoltage/Undervoltage");
  } else if (tc.isInternalOutOfRange()) {
    Serial.println(" Internal temperature (cold junction) out of range)");
  } else if (tc.isExternalOutOfRange()) {
    Serial.println(" External temperature (hot junction) out of range");
  }
}


void setup() {
  tc.begin(); // Inicia termopar
  
  // Configuração do modulo de tempo ------------
  rtc.halt(false);  // Set the clock to run-mode
  rtc.setSQWRate(SQW_RATE_1); // Set SQW/Out rate to 1Hz
  rtc.enableSQW(true); // and enable SQW

  // Inicia comunicação serial ------------
  Serial.begin(9600);
  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  while (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    delay(500);
  }
  Serial.println("card initialized.");

  // Checa erros no termopar, se não tiver erro ele escreve o cabeçalho no arquivo de log
  if (tc.hasError()) {
    printErrorsTc();
  } else {
    File dataFile = SD.open("datalog.txt", FILE_WRITE);
    dataFile.println("Data\tLI (ºC)\tLE (mV)");
     Serial.println("Data\tLI (ºC)\tLE (mV)");
    dataFile.close();
  }
}

void loop() {
  // Inicia variáveis: ------------------
  String data = "";
  char internal[20] = "";
  char external[20] = "";

  // Lê termopar -----------------------
  tc.read(); 
  tc.getExternal(); // Read the date and time

  // Verifica erro no termopar ---------
  if (tc.hasError()) {
    data += String(rtc.getDateStr(FORMAT_LONG, FORMAT_LITTLEENDIAN, '/')) + " " + String(rtc.getTimeStr())+"\tProblema na leitura";
    printErrorsTc();
    } 
    
  // Se estiver tudo ok continua ------- 
  else {   
  
    // Checa se é multiplo de 10 minutos
    while (rtc.getTime().min%10 != 0){
      delay(60000);
    }

    // Lê o dia e a hora e coloca na string
    String date_ = String(rtc.getDateStr(FORMAT_LONG, FORMAT_LITTLEENDIAN, '/'));
    String hour_ = String(rtc.getTimeStr());

    // Lê a temperatura em graus (internal) e a sonda em milivots (external)
    dtostrf(tc.getInternal(), 6, 6, internal);
    dtostrf(tc.getExternal()*1000, 10, 9, external);

    // Junta tudo pra gravar no arquivo de log
    data +=  date_ + " " + hour_ + "\t" + internal + "\t" + external;
  }

  // see if the card is present and can be initialized:
  
  if (SD.begin(chipSelect)) 
  {
    File dataFile = SD.open("datalog.txt", FILE_WRITE);
    Serial.println(data);
    if (dataFile) 
    {
      dataFile.println(data);
      dataFile.close();
    }
    else 
      Serial.println("Error opening datalog.txt");
  }
  else 
    Serial.println("Card failed, or not present");
  
  delay(tempo_leitura_segundos * 1000);
}




