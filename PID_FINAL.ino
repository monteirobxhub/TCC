// --- INCLUSÃO DAS BIBLIOTECAS USADAS ---
#include <SimpleModbusSlave.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// --- DEFINIÇÃO DOS PINOS A SEREM USADOS ---
#define pinZC 2
#define pinDIM 4 
#define Temperatura 3

// --- PARAMETRIZAÇÃO DA APRESENTAÇÃO DE DADOS DO PLX-DAQ ---
int linha = 0;  
int LABEL = 1;


#define periodo 8333 // MICROSSEGUNDOS

// --- CRIAÇÃO DE OBJETOS PARA AQUISIÇÃO DE DADOS DE TEMPERATURA ---
OneWire oneWire(Temperatura);
DallasTemperature sensors(&oneWire);

// --- DECLARAÇÃO DE VARIÁVEIS USADAS NO PROCESSO ---
float kp= 0.012, ki = (0.0000026), kd= 0.02;        // GANHOS DO CONTROLADOR OBTIDOS EXPERIMENTALMENTE
float p = 0,  i = 0, d = 0, pid = 0, tdecorrido = 0;
float erro = 0;
float setpoint = 40;      // TEMPERATURA DE REFERÊNCIA       
float temperatura;              
float intensidade;

void setup(){
  Serial.begin(9600);

  // --- INICIALIZA A BIBLIOTECA COM AS FUNÇÕES DE LEITURA DOS SENSORES ---
  sensors.begin();

  // --- SETA OS PINOS E INICIALIZA A INTERRUPÇÃO DA FUNÇÃO ZEROCROSS ---
  pinMode(3, INPUT_PULLUP);
  pinMode(pinDIM, OUTPUT);
  pinMode(pinZC, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pinZC), sinalZC, RISING);

  // --- SETA CABEÇALHO DO PLXDAQ
  Serial.println("CLEARDATA");
  Serial.println("LABEL,Hora,Temperatura,Setpoint,linha");
}

 
void loop(){

  mediaTemperatura();     // RETORNA O VALOR DA TEMPERATURA LIDA

  delay(100);

  erro = setpoint - temperatura;     // CALCULA O ERRO, TEMPERATURA ATUAL SUBTRAIDA DO SETPOINT
  
  float delta = ((millis() - tdecorrido))/1000.0;       // CALCULA A DOFERENÇA ENTRE LOOPS

  tdecorrido = millis();            //ARMAZENA O VALOR DE CADA LOOP

  // CALCULO DA PARCELA PROPORCIONAL
  p = kp*erro;

  // CALCULO DA PARCELA DERIVAIVA
  i = (ki*erro)*delta;

  // CALCULO DA PARCELA DERIVAIVA
  d = erro*kd/delta;

  // CALCULO DO PID
  pid = p+i+d;
  linha++;

  // --- FUNÇÃO PARA CONTROLAR A TENSÃO FORNECIDA AO DIMMER ---
  if (pid > 0.20) {
    intensidade = 90; // INTENSIDADE MINIMA DO DIMMER
  } else if (pid < -0.01) {
    intensidade = 0; // INTENSIDADE MAXIMA DO DIMMER
  } else {
    // CONVERTE O RANGE DE VALORES COLETADOS DO PID PARA VALORES DE INTENSIDADES FORNECIDOS AO DIMMER
    intensidade = map(pid * 100, 0, 20, 20, 90);  
  }                                            

  // --- PLOTAGEM NO PLX-DAQ ---
  Serial.print("DATA,TIME,");  
  Serial.print(temperatura);
  Serial.print(",");
  Serial.print(setpoint);
  Serial.print(",");
  Serial.println(linha);
  delay(500);

}//fim do loop

// --- FUNÇÃO PARA ACIONAR O PINO DE INDENTIFICAÇÃO DE PASSAGEM PELO ZERO (ZEROCROSS) ---
void sinalZC() {
  if ( intensidade < 20) return;
  if ( intensidade > 90) intensidade = 90;

  int delayInt = periodo - (intensidade * (periodo / 100) );

  delayMicroseconds(delayInt);
  digitalWrite(pinDIM, HIGH);
  delayMicroseconds(6);
  digitalWrite(pinDIM, LOW);
}

// --- FUNÇÃO PARA AQUISIÇÃO DA TEMPERATURA ---
void mediaTemperatura(){
  sensors.requestTemperatures();
  temperatura = ((sensors.getTempCByIndex(0)+sensors.getTempCByIndex(1))/2);
}

