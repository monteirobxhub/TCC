// --- INCLUSÃO DAS BIBLIOTECAS USADAS ---
#include <SimpleModbusSlave.h>
#include <math.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// --- DEFINIÇÃO DOS PINOS A SEREM USADOS ---
#define Temperatura 3
const int RelePin = 5;
const int RelePinLuz = 7;  

// --- CRIAÇÃO DE OBJETOS PARA AQUISIÇÃO DE DADOS DE TEMPERATURA ---
OneWire oneWire(Temperatura);
DallasTemperature sensors(&oneWire);

// --- CRIAÇÃO DAS VARIÁVEIS A SEREM USADAS NO SCADA ---
float mediaTemp = 0;
int maxima;
int minima;
int ParteInteiraMedidaM1, ParteInteiraMedidaM2, ParteInteiraT1, ParteDecimalT1;
int ParteDecimalMedidaM1, ParteDecimalMedidaM2, ParteInteiraT2, ParteDecimalT2;
int mediaSet = 39;
int mediaScada = 0;

// --- CRIAÇÃO DAS VARIAVEIS DE COMUNICAÇÃO COM O SCADA ---
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

// --- REGISTRADORES DO SCADA ---
unsigned int holdingRegs[HOLDING_REGS_SIZE];

void setup() {
  Serial.begin(9600);  // Inicialize a comunicação serial

  // --- SET DO MODBUS NO ARDUINO ---
  modbus_configure(&Serial, 9600, SERIAL_8N1, 1, 2, HOLDING_REGS_SIZE, holdingRegs);
  modbus_update_comms(9600, SERIAL_8N1, 1);

  // --- DEFINIÇÃO DAS FUNCIONALIDADES DOS PINOS ---
  pinMode(RelePin, OUTPUT);    
  digitalWrite(RelePin, HIGH);  
  pinMode(RelePinLuz, OUTPUT);  
  digitalWrite(RelePinLuz, HIGH);

  // --- INICIALIZA A BIBLIOTECA COM AS FUNÇÕES DE LEITURA DOS SENSORES ---
  sensors.begin();

}

void loop() {

  // --- SETA A POSIÇÃO INICIAL DO COOLER E FONTE DE CALOR ---
  digitalWrite(RelePin, HIGH);
  digitalWrite(RelePinLuz, LOW);

  // ---COLETA OS VALORES DE TEMPERATURA ---
  Temperatura1();
  delay(200);


  maxima = mediaSet + 1; // SETA O VALOR DA TEMPERATURA MÉXIMA
  int D = 1; // VARIÁVEL DE VERIFICAÇÃO

  // --- CONDIÇÃO PARA RESFRIAR O SISTEMA ---
  if (mediaTemp >= maxima && D == 1) {
    
    // --- COMANDO DO RELE PARA RESFRIAR O SISTEMA ---
    digitalWrite(RelePin, LOW);
    Serial.println("Cooler ligado");
    digitalWrite(RelePinLuz, HIGH);
    Serial.println("Lampada Desligada");
    Temperatura1();

    // --- LAÇO PARA MANTER O SISTEMA RESFRIANDO ATÉ A TEMPERATURA MÍNIMA ---
    while (D == 1) {

      Temperatura1();
      delay(200);

      minima = mediaSet - 1;
      // --- FUNÇÃO PARA FAZER O SISTEMA VOLTAR A AQUECER ---
      if (mediaTemp <= minima) {
        Temperatura1();

        D = 0;
      }
    }

  }  //fim do while


}  // loop principal

// --- FUNÇÃO PARA COLETAR A TEMPERATURA E PUBLICAR NO MODBUS ---
void Temperatura1() {

  sensors.requestTemperatures();    //COLETA OS VALORES LIDOS PELO SENSOR

  // --- BLOCO PARA COLETAR A TEMPERATURA DO SENSOR 1 E SEPARA AS PARTES DECIMAL E INTEIRO ---
  float tempSensor1 = sensors.getTempCByIndex(0);
  Serial.print("Temperatura do sensor 1 é: "); 
  Serial.print(tempSensor1);
  Serial.println(" °C");
  ParteInteiraT1 = floor(tempSensor1);
  ParteDecimalT1 = (tempSensor1 - ParteInteiraT1) * 100;


  // --- BLOCO PARA COLETAR A TEMPERATURA DO SENSOR 2 E SEPARA AS PARTES DECIMAL E INTEIRO ---
  float tempSensor2 = sensors.getTempCByIndex(1);
  Serial.print("Temperatura do Sensor 2: ");
  Serial.print(tempSensor2);
  Serial.println(" °C");
  ParteInteiraT2 = floor(tempSensor2);
  ParteDecimalT2 = (tempSensor2 - ParteInteiraT2) * 100;

  // --- CALCULA A MÉDIA DAS TEMPERATURAS E SEPARA AS PARTES DECIMAL E INTEIRO --- 
  mediaTemp = ((tempSensor1 + tempSensor2) / 2);
  Serial.print("MEDIA DE TEMPERATURA: ");
  Serial.print(mediaTemp);
  Serial.println(" °C");
  ParteInteiraMedidaM1 = floor(mediaTemp);
  ParteDecimalMedidaM1 = (mediaTemp - ParteInteiraMedidaM1) * 100;
  Serial.println("------------------------------");

  // --- BLOCO PARA ENVIAR OS VALORES SEPARADAMENTE PARA O SUPERVISÓRIO, SEPARANDO PARTE DECIMAL E INTERIA ---
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
