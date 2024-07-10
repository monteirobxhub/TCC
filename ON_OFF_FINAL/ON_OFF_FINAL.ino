#include <SimpleModbusSlave.h>
#include <math.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define Temperatura 3
const int RelePin = 5;
const int RelePinLuz = 7;  

OneWire oneWire(Temperatura);

DallasTemperature sensors(&oneWire);


float mediaTemp = 0;
int maxima;
int minima;
int ParteInteiraMedidaM1, ParteInteiraMedidaM2, ParteInteiraT1, ParteDecimalT1;
int ParteDecimalMedidaM1, ParteDecimalMedidaM2, ParteInteiraT2, ParteDecimalT2;
int mediaSet = 39;
int mediaScada = 0;

enum {
  VALOR_REAL_TEMP1,
  VALOR_REAL_TEMP2,
  VALOR_DECIMAL_TEMP1,
  VALOR_DECIMAL_TEMP2,
  VALOR_REAL_MEDIA,
  VALOR_DECIMAL_MEDIA,
  VALOR_MEDIA_SET,
  VALOR_MEDIA_SCADA,
  HOLDING_REGS_SIZE
};


unsigned int holdingRegs[HOLDING_REGS_SIZE];

void setup() {
  Serial.begin(9600);  // Inicialize a comunicação serial

  modbus_configure(&Serial, 9600, SERIAL_8N1, 1, 2, HOLDING_REGS_SIZE, holdingRegs);

  modbus_update_comms(9600, SERIAL_8N1, 1);

  pinMode(RelePin, OUTPUT);    // seta o pino como saída
  digitalWrite(RelePin, HIGH);  // seta o pino com nivel logico baixo
  pinMode(RelePinLuz, OUTPUT);  // seta o pino como saída
  digitalWrite(RelePinLuz, HIGH);

  sensors.begin(); /*inicia biblioteca*/

  pinMode(2, INPUT_PULLUP);
}

void loop() {

  Serial.println("***************************");
  digitalWrite(RelePin, HIGH);
  Serial.println("Cooler Desligado");
  digitalWrite(RelePinLuz, LOW);
  Serial.println("Lampada Ligada");


  Temperatura1();
  
  delay(200);

  maxima = mediaSet + 1;
  int D = 1;
  if (mediaTemp >= maxima && D == 1) {

    digitalWrite(RelePin, LOW);
    Serial.println("Cooler ligado");
    digitalWrite(RelePinLuz, HIGH);
    Serial.println("Lampada Desligada");
    Temperatura1();


    while (D == 1) {

      Temperatura1();
      Serial.println("Resfriando");

      Serial.println(" ");
      delay(200);

      minima = mediaSet - 1;
      if (mediaTemp <= minima) {
        Temperatura1();

        D = 0;
      }
    }

  }  //fim do while


}  // loop principal


void Temperatura1() {
  sensors.requestTemperatures();

  float tempSensor1 = sensors.getTempCByIndex(0);
  Serial.print("Temperatura do sensor 1 é: "); /* Printa "A temperatura é:" */
  Serial.print(tempSensor1);
  Serial.println(" °C");
  ParteInteiraT1 = floor(tempSensor1);
  ParteDecimalT1 = (tempSensor1 - ParteInteiraT1) * 100;


  //////////////////////////////////////////////////////////////////
  float tempSensor2 = sensors.getTempCByIndex(1);
  Serial.print("Temperatura do Sensor 2: ");
  Serial.print(tempSensor2);
  Serial.println(" °C");
  ParteInteiraT2 = floor(tempSensor2);
  ParteDecimalT2 = (tempSensor2 - ParteInteiraT2) * 100;

  //////////////////////////////////////////////////////////////////
  mediaTemp = ((tempSensor1 + tempSensor2) / 2);
  Serial.print("MEDIA DE TEMPERATURA: ");
  Serial.print(mediaTemp);
  Serial.println(" °C");
  ParteInteiraMedidaM1 = floor(mediaTemp);
  ParteDecimalMedidaM1 = (mediaTemp - ParteInteiraMedidaM1) * 100;
  Serial.println("------------------------------");

  //////////////////////////////////////////////////////////////////
  if (mediaScada != 0) 
  {
    mediaTemp = mediaScada;
  }
  modbus_update();

  //////////////////////////////////////////////////////////////////
  modbus_update();
  holdingRegs[VALOR_REAL_TEMP1] = ParteInteiraT1;
  holdingRegs[VALOR_REAL_TEMP2] = ParteInteiraT2;
  holdingRegs[VALOR_DECIMAL_TEMP1] = ParteDecimalT1;
  holdingRegs[VALOR_DECIMAL_TEMP2] = ParteDecimalT2;
  holdingRegs[VALOR_REAL_MEDIA] = ParteInteiraMedidaM1;
  holdingRegs[VALOR_DECIMAL_MEDIA] = ParteDecimalMedidaM1;
  holdingRegs[VALOR_MEDIA_SET] = mediaSet;
  holdingRegs[VALOR_MEDIA_SCADA] = mediaScada;
}
