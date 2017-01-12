
#define 	F_CPU 3686400	// Taktfrequenz des myAVR-Boards
#include	<avr\io.h>		// AVR Register und Konstantendefinitionen

//custom includes
#include "Empfänger.h"
#include <util/delay.h>		//delay-Methode
#define	LEDZEIT	889			//halbes Bit --> Muss mit Sender übereinstimmen!!!!

// Custom variables 
int displayZahl[10]={
	0b00111111,0b00000110,0b01011011,0b01001111, 0b01100110,0b01101101,0b01111101,0b00000111,0b01111111,0b01101111
};
//Idlespannung bei keiner Einstrahlung
//TODO: richtigen Wert herausfinden
int messwert = 0;
int vergleichsspannung = 511;	//2.5V
int eigeneAddresse[3] = {0,0,1};
int addresse[3];
int command[3];
int taster[3][3] = 
{
	{0,0,1},{0,1,0},{1,0,0}
};

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

void setup()
{

	// LED für "an" B1 und Diode (nur Testen) an B0
	//DDRB = 0b000011;
	//PORTB = 0b000001;

	//LED für "an" an B1 mit am anfang an machen	
	DDRB = 0b000010;
	PORTB = 0b000010;

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

bool checkAddress()
{
	for(int i=0; i<3; i++)
	{
		int x = getBit();
		if(x != 5)
		{
			addresse[i] = x;
		}
	}
	for(int i=0; i<3; i++)
	{
		if(addresse[i] == eigeneAddresse[i])
		{
			continue;
		}
	else
		{
			return false;
		}
	}

	return true;
}

bool getCommand()
{

	for(int i=0; i<3; i++)
	{
		int bit = getBit();

		if(getBit() != 5)
		{
			command[i] = getBit();
		}
		else
		{
			return false;
		}
	}
	
	return true;
}

void showCommandOnDisplay()
{
	int Taster = 0;
	bool isTastercommandVorhanden = false;

	//Geht alle Tasterbefehle durch Taster[j][k]
	for(int j=0; j<3;j++)
	{
		for(int k=0; k<3; k++)
		{

			// Prüft ob Command der empfangen wurde dem Tasterbefehl entspricht
			if(command[k] == taster[j][k])
			{

				//wenn nach 3. Prüfen noch korrekt ist k schon 2
				if(k == 2)
				{
					//Das gesamte Kommando stimmt mit aktuellem Taster (j) überein
					//Also kann man aufhören zu prüfen: j = 3 und damit erste Forschleife beenden
					Taster = j+1;
					isTastercommandVorhanden = true;
					j=3;
				}
			}
			else
			{
				//Stimmt der Command an einer Stelle nicht mit dem aktuellen Taster überein dann springe
				//zum nächsten Taster
				k=3;

			}
		}
	}

	//Zahl Taster auf Display anzeigen
	if(isTastercommandVorhanden){
		segmentanzeige(Taster);
	}else{
		segmentanzeige(404);
	}
}

int getBit()
{
	// prüfe auf 1 also erst niedrige Spannung (Diode AN) dann hohe Spannung (Diode AUS)
	if(messwert < vergleichsspannung)
	{
		_delay_us(LEDZEIT);

		//falls High erfolgreich (erstes if), dann auf Low prüfen
		if(messwert > vergleichsspannung)
		{

			// warten bis zum nächsten Bit 
			_delay_us(LEDZEIT);
			//Bit "1" erfüllt alle Kriterien also return 1
			return 1;
		}
	else 
		{
			// Bitfehler : V zu lange zu hoch
			// 5 ist einfach ein Fehlercode den wir überprüfen können
			return 5;  
		}

	}
else if(messwert > vergleichsspannung)
	{
		//prüfe auf 0 also erst hohe Spannung (Diode AUS) und dann niedrige Spannung (Diode AN)
		_delay_us(LEDZEIT);	
		if(messwert < vergleichsspannung)
		{

			// warten bis zum nächsten Bit 
			_delay_us(LEDZEIT);
			//Bit "0" erfüllt alle Kriterien also return 0
			return 0;	

		}
	else 
		{
			// Bitfehler : V zu lange zu hoch
			// 5 ist einfach ein Fehlercode den wir überprüfen können
			return 5;  
		}

	}
}

void segmentanzeige(int x){
	
	switch (x)
	{
		case 0:
			PORTD = 0b00111111;
			break;
		case 1:
			PORTD = 0b00000110;
			break;
		case 2:
			PORTD = 0b01011011;
			break;
		case 3:
			PORTD = 0b01001111;
			break;
		case 4:
			PORTD = 0b01100110;
			break;
		case 5:
			PORTD = 0b01101101;
			break;
		case 6:
			PORTD = 0b01111101;
			break;
		case 404:
			PORTD = 0b01110001;
			break;
		default:
			PORTD = 0b01000000;
			break;
	}
}


main ()						
{
	setup(); 
	int ZwischenErgebnis;

	do 
	{

		messwert = measure();

		//Bei IR-Einstrahlung ist der Messwert, also die anliegende Spannung kleiner
		if(messwert < vergleichsspannung)
		{
			//warten bis zur Hälfte des AN- oder AUS-Zeit
			_delay_us(LEDZEIT/2);
			ZwischenErgebnis= getBit(); 
			if(ZwischenErgebnis == 1)
			{
				ZwischenErgebnis = getBit(); 
				if(ZwischenErgebnis == 1)
				{
					// zwei 1-Bits detected -> Startbits
					// eigentlichen Befehl kann decodiert werden
					if(checkAddress() == true)
					{
						getCommand();
						showCommandOnDisplay();
						
					}

				}

			}

		}

		/*
		if(messwert > vergleichsspannung){
		PORTB = 0b000001;
		}
	else if(messwert < vergleichsspannung){
	PORTB = 0b000011;
	}

	*/
	
	}
	while (true);			
}

