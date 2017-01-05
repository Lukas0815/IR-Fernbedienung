
#define 	F_CPU 3686400	// Taktfrequenz des myAVR-Boards
#include	<avr\io.h>		// AVR Register und Konstantendefinitionen

//custom includes
#include <util/delay.h>		//delay-Methode
#define	LEDZEIT	889			//halbes Bit --> Muss mit Sender übereinstimmen!!!!

// Custom variables 
int displayZahl[10]={
0b00111111,0b00000110,0b01011011,0b01001111, 0b01100110,0b01101101,0b01111101,0b00000111,0b01111111,0b01101111
};
//Idlespannung bei keiner Einstrahlung
//TODO: richtigen Wert herausfinden
int vergleichsspannung = 512;
int eigeneAddresse[3] = {0,0,1};
int addresse[3];
int command[3];

void initAD()
{
	ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1);
	ADMUX |= (1<<REFS0); 				// Referenzspannung 5V (Werte von 0 - 1023)
	ADCSRA |= (1<<ADSC);
	while (ADCSRA & (1<<ADSC) ) {} 	// erste Messung zu verwerfen 
}


/*
	Setup AD
*/

void setup(){

// LED für "an"
DDRB = 0b000001;
PORTB = 0b000001;

// C1 als Eingang definiert
DDRC = 0b000000;
// C1 mit Strom versorgt 
PORTC = 0b000010;

//7 Segmentanzeige anmachen an Lo an Port D anschließen
DDRD = 0b11111111;	//Ausgang alle an
PORTD = 0b00000000;	//Stromversorgung = 0

initAD();
} 


int measure() 
{ 
  	ADMUX |= (1<<MUX0);  // AD liegt an C1 an! :3 
	ADCSRA |= (1<<ADSC);
	while(ADCSRA & (1<<ADSC))
    	{}
	int wert=ADCW;
	
	// Wert zwischen 0 - 1023
	return wert;
}

bool checkAddress(){
	for(int i=0; i<3; i++){
		int x = getBit();
		if(x != 5){
			addresse[i] = x;
		}
	}
	for(int i=0; i<3; i++){
		if(addresse[i] == eigeneAddresse[i]){
			continue;
		}else{
			return false;
		}
	}
	
	return true;
}

void getCommand(){
	
}

void showCommandOnDisplay(){
	
}

int getBit()
{
	// prüfe auf 1 also erst Hohe dann Niedrige Spannung 
	if(messwert > vergleichspannung){
		_delay_us(LEDZEIT);
		
		//falls High erfolgreich (erstes if), dann auf Low prüfen
		if(messwert<= vergleichspannung)
		{	
			// warten bis zum nächsten Bit 
			_delay_us(LEDZEIT);
			//Bit "1" erfüllt alle Kriterien also return 1
			return 1;
		} else {
			// Bitfehler : V zu lange zu hoch
			// 5 ist einfach ein Fehlercode den wir überprüfen können
			return 5;  
		}	
	}else if(messwert <= vergleichspannung){
		//prüfe auf 0 also erst Low und dann High Spannung
		_delay_us(LEDZEIT);	
		if(messwert > vergleichspannung){	
			// warten bis zum nächsten Bit 
			_delay_us(LEDZEIT);
			//Bit "0" erfüllt alle Kriterien also return 0
			return 0;	
			
		}else {
			// Bitfehler : V zu lange zu hoch
			// 5 ist einfach ein Fehlercode den wir überprüfen können
			return 5;  
		}		
	}
}


main ()						
{
	setup(); 
	int ZwischenErgebnis;
	
	
	do {					
		int messwert = measure();
		
		//Bei IR-Einstrahlung ist der Messwert, also die anliegende Spannung größer
		if(messwert > vergleichspannung){
			//warten bis zur Hälfte des AN- oder AUS-Zeit
			_delay_us(LEDZEIT/2);
			ZwischenErgebnis= getBit(); 
			if(ZwischenErgebnis==1){
				ZwischenErgebnis= getBit(); 
				if(ZwischenErgebnis==1){
					// zwei 1-Bits detected -> Startbits
					// eigentlichen Befehl kann decodiert werden
					if(checkAddress() == true){
						getCommand();
						showCommandOnDisplay();
					}
										
				}		
			}	
		}
	} while (true);			
}

