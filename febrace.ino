/* Trabalho Engenharia - Febrace 2016
 * (MP2E) - Monitoramento de Parâmetros de Equipamentos Eletroeletrônicos
 * Aluna: Vitória Carolina Ferreira Pereira 
 * Coorientador: Yhan Christian SouzaSilva - versão 4.0 - Data: 20/03/2015
 */    

// --- Bibliotecas Auxiliares ---

#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include <DHT.h>
#include "EmonLib.h"
#include <Wire.h>
#include "RTClib.h"
#include "Timer.h"

// --- Definições Hardware ---

#define upButton 12
#define downButton 11
#define defaultButton 10
#define onButton 9
#define voltageButton 8
#define carga A0
#define pinoSensor A3
#define modeloSensor DHT11
#define sensorCorrente A1
#define buzzer A2
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);
RTC_DS1307 clockRTC;
DHT dht(pinoSensor, modeloSensor);
SoftwareSerial bluetooth(0, 1);
EnergyMonitor emon1;
Timer timer;

// --- Variáveis ---

char menu = 1;
char set1 = 0, set2 = 0, var = 0; 
boolean  flagUP, flagDown, flagDefault, flagOn, flagVoltage;
float temp, irms;
int tensao;
byte grau[8] = {0x0C, 0x12, 0x12, 0x0C, 0x00, 0x00, 0x00, 0x00};

// --- Escopo das funções ---

void changeMenu();
void exibirMenu();
void febrace();
void horaData();
void acionarCarga();
int  voltage();
float current();
float temperatura();
void menuDefault();
void leituraBluetooth();

// --- Configurações Iniciais ---

void setup() {
	timer.every(1000,leituraBluetooth);
        bluetooth.begin(115200);
        emon1.current(sensorCorrente, 60);
        Wire.begin();
        clockRTC.begin();
        lcd.begin(16, 2);
        if(!clockRTC.isrunning()) clockRTC.adjust(DateTime(__DATE__, __TIME__));
        for(char x = 8; x <= 12; x++) pinMode(x, INPUT_PULLUP);
       	pinMode(carga, OUTPUT);
       	pinMode(buzzer, OUTPUT);
       	digitalWrite(carga, HIGH);
    	flagUP = false;
    	flagDown = false;
    	flagDefault = false;
	flagOn = false;
    	flagVoltage = false;
}

// --- Loop ---

void loop() {
	changeMenu();
    	exibirMenu();	
    	menuDefault();
    	timer.update();
}

// --- Função p/ troca de menus ---

void changeMenu() {
	if(!digitalRead(upButton)) {
		flagUP = true;
		digitalWrite(buzzer, HIGH);
	}

	if(!digitalRead(downButton)) {
		flagDown = true;
		digitalWrite(buzzer, HIGH);
	}

	if(digitalRead(upButton) && flagUP) {
		flagUP = false;
		lcd.clear();
		menu++;
		digitalWrite(buzzer, LOW);
		if(menu > 6) menu = 1;
	}

	if(digitalRead(downButton) && flagDown) {
		flagDown = false;
		lcd.clear();
		menu--;
		digitalWrite(buzzer, LOW);
		if(menu < 1) menu = 6;
	}
}

// --- Função p/ definir ordem de menus  ---

void exibirMenu() {
	switch(menu) {
		case 1:
		febrace();
		break;
		case 2:
		horaData();
		break;
		case 3:
		acionarCarga();
		break;
		case 4:
		voltage();
		break;
		case 5: 
		current();
		break;
		case 6:
		temperatura();
		break;
	}
}

// --- Função Febrace - Exibição de mensagem LCD ---

void febrace() {
	lcd.setCursor(4,0);
	lcd.print("Febrace");
	lcd.setCursor(3,1);
 	lcd.print("Bem-Vindo ");
 	delay(25);
}

// --- Função p/ exibir hora e data ---

void horaData() {
	DateTime now = clockRTC.now();
	lcd.setCursor(0, 0);
    	lcd.print("Data: ");
	lcd.print(now.day(), DEC);
	lcd.print("/");
	lcd.print(now.month(), DEC);
	lcd.print("/");
	lcd.print(now.year(), DEC);
	lcd.print(" ");
	lcd.setCursor(0, 1);
	lcd.print("Hora: ");
  
	if(now.hour() < 10) lcd.print("0");
	lcd.print(now.hour(), DEC);
	lcd.print(":");

	if(now.minute() < 10) lcd.print("0");
	lcd.print(now.minute(), DEC);
	lcd.print(":");

	if(now.second() < 10) lcd.print("0");
	lcd.print(now.second(), DEC);
	 delay(25);
}

// --- Acionamento carga por botão ou bluetooth ---

void acionarCarga() {	
	var = bluetooth.read();
	lcd.setCursor(5, 0);
	lcd.print("Status");
	if((!digitalRead(onButton)) || (var == 'L')) {
		flagOn = true;
		digitalWrite(buzzer, HIGH);
	}

	if((digitalRead(onButton) && flagOn)|| (var == 'D')) {
		flagOn = false;
		set1++;
		digitalWrite(buzzer, LOW);
		if(set1 > 2) set1 = 1;
	}
  switch (set1) {
      case 1:
        lcd.setCursor(0, 1);
        lcd.print("Equipamento ON ");
        digitalWrite(carga, LOW);
        break;
      case 2:
        lcd.setCursor(0, 1);
        lcd.print("Equipamento OFF");
        digitalWrite(carga, HIGH);
        break;
  }
}

// --- Função para definir tensão de entrada ---

int voltage() {
	lcd.setCursor(0, 0);
	lcd.print("Tensao Carga");
	if(!digitalRead(voltageButton)) {
	    flagVoltage =  true;
	    digitalWrite(buzzer, HIGH);
	}

	if(digitalRead(voltageButton) && flagVoltage) {
	    flagVoltage = false;
	    set2++;
	    digitalWrite(buzzer, LOW);
	    if(set2 > 2) set2 = 1;
	}

	if(set2 == 1) {
		 tensao = 110;
		lcd.setCursor(0, 1);
		lcd.print("Tensao (V): ");
    		lcd.print(tensao);
    		return tensao;
	} 

	else if(set2 == 2) {
		tensao = 220;
		lcd.setCursor(0, 1);
		lcd.print("Tensao (V): ");
    		lcd.print(tensao);
		 return tensao;
	}   	
}

// --- Função p/ exibição da corrente elétrica e calculo de potência ---

float current() {
	irms = emon1.calcIrms(1480); 
	int tensaoRede = voltage();
	if (irms < 0.2) irms = 0;
	lcd.setCursor(0, 0);
	lcd.print("Corr(A): ");
	lcd.print(irms);
	lcd.setCursor(0, 1);
	lcd.print("Pot (W): ");
	lcd.print(irms * tensaoRede, 3); 
	delay(50);
	return irms;
}

// --- Função p/ exibição da temperatura - utilizo DHT11 ---

float temperatura() {
	lcd.createChar(0, grau);
 	temp = dht.readTemperature();
 	lcd.setCursor(2,0);
 	lcd.print("Temperatura");
 	lcd.setCursor(5,1);
 	lcd.print(temp, 1);
 	lcd.setCursor(9,1);
 	lcd.write((byte)0);
 	delay(25);
 	return temp;
}

// --- Função default, passa informações no LCD 10 vezes ---

void menuDefault() {
	if(!digitalRead(defaultButton)) {
	flagDefault = true;
	digitalWrite(buzzer, HIGH);	    
	}
 
 	if(digitalRead(defaultButton) && flagDefault) {
		for(char i = 0; i < 10; i++) {
			digitalWrite(buzzer, LOW);
			lcd.clear();
			flagDefault = false;
			horaData();
			delay(1000);
			lcd.clear();
			current();
			delay(1000);
			lcd.clear();
			temperatura();
			delay(1000);
		} 	
		lcd.clear();
	}
}

// --- Função p/ leitura de sensores pelo módulo bluetooh HC-05 ---

void leituraBluetooth() {
	float leituraTemperatura = temp;
    	int leituraTensao = tensao;
    	float leituraCorrente = irms;
    	bluetooth.print("Temperatura (ºC): ");
    	bluetooth.println(leituraTemperatura);
    	bluetooth.print("Tensão (V): ");
    	bluetooth.println(leituraTensao);
    	bluetooth.print("Corrente (A): ");
    	if (leituraCorrente < 0.2) leituraCorrente = 0;
    	bluetooth.println(leituraCorrente);
    	bluetooth.print("Potência (W): ");
    	bluetooth.println(leituraTensao * leituraCorrente, 3);
}














 
