#define 	F_CPU 3686400	// Taktfrequenz des myAVR-Boards
#include	<avr\io.h>		// AVR Register und Konstantendefinitionen

/*Custom Includes and defines*/
#include "Grundgerüst.h"
#include <util/delay.h>		//delay-Methode
#define	STARTZEIT_ANZEIGE 1050
#define	LEDZEIT	1000000 			//ein halbes Bit lang leuchtet die IR_DIODE in microseconds --> _delay_us()
#define ZEITNACHSIGNAL	30		//in ms

const int geraeteadresse[3] = {0,0,1};
int togglebit = 1;
int tastenkommando[3][3] = 
{
	{0,0,1},{0,1,0},{1,0,0}
};
int letzteTaste = 0;

void turnON()
{
	//Schaltet die Anzeige optisch an und für eine bestimmte Zeit fürs "hochfahren"
	PORTD = 0b11111111;
	_delay_ms(STARTZEIT_ANZEIGE);
	PORTD = 0b00000000;

	//Hier könnte eine AN_LED eingebaut werden --> Wenn angeschlossen bei sendcommando() auch PORTC mitverändern!!!
	PORTC = 0b000001;
}

void setup()
{
	//LED Ausgänge definieren an Port C0, C1,C2
	//IR-Diode an Port C3
	DDRC = 0b001111;
	//LED mit Strom versorgen (Am Anfang 0)
	PORTC = 0b000000;

	//Schalter als Eingang
	DDRB = 0b000000; 
	//Schalter mit Strom versorgen B0,B1,B2
	PORTB = 0b000111; 

	//7 Segmentanzeige anmachen an Lo an Port D anschließen
	DDRD = 0b11111111;	//Ausgang alle an
	PORTD = 0b00000000;	//Stromversorgung = 0

	turnON();             
}

void diodeAN()
{
	//Strom für IR_DIODE an
	//wenn andere LED angeschlossen entsprechende 1 mitnehmen z.B 0b001001
	PORTC = 0b001001;
	_delay_us(LEDZEIT);
}

void diodeAUS()
{
	//Strom für IR_DIODE aus
	//wenn andere LED angeschlossen entsprechende 1 mitnehmen z.B 0b000001
	PORTC = 0b000001;
	//PORTC = 0b000101;		connect C2 to an LED to watch when Diode is off
	_delay_us(LEDZEIT);
}

void segmentanzeige(int x)
{
	switch (x)
	{
		case 0:
		PORTD = 0b0111111;
		break;
		case 1:
		PORTD = 0b0000110;
		break;
		default:
		PORTD = 0b1000000;
		break;
	}
}

void sendCommando(int aktuellerBefehl[])
{

	for(int i=0; i<9; i++)
	{
		//Zeigt entweder 0 oder 1 an, also aktueller Befehl
		segmentanzeige(aktuellerBefehl[i]);

		if(aktuellerBefehl[i] == 0)
		{
			diodeAUS();
			diodeAN();
		}
	else
		{
			diodeAN();
			diodeAUS();	
		}

	}

	//Diode wieder aus am Schluss
	diodeAUS();

	//Segmentanzeige aus machen
	segmentanzeige(23);

	//warten bis neuer Befehl entgegengenommen werden darf
	//PROBLEM: Nach letztem Bit ist noch mal ein delay!!!
	_delay_ms(ZEITNACHSIGNAL);
}

void assembleCommando(int Taster)
{
	int	aktuellerBefehl[9];	//9 ist die Länge aller Bits mit 2 Startbits, 1 Togglebit,3 Adressbits und 3 Kommandobits

	/*if(tastenkommando[Taster-1][2] == 1){
	turnON();
	} */

	//Startbits sind immer 1
	aktuellerBefehl[0] = 1;
	aktuellerBefehl[1] = 1;

	//Togglebit festlegen
	if(Taster == letzteTaste)
	{
		aktuellerBefehl[2] = 1;		//1 bedeutet gleiche Taste wie davor
	}
else
	{
		aktuellerBefehl[2] = 0;		//0 bedeutet neuer Befehl
	}

	//Geraeteadresse einbinden
	for(int i=0; i<3; i++)
	{
		aktuellerBefehl[3+i] = geraeteadresse[i];
	}

	/*if(aktuellerBefehl[3] == 0 && aktuellerBefehl[4]== 0 && aktuellerBefehl[5] == 1){
	//turnON();
	} */

	//Tastenspezifischen Befehl hinzufügen
	for(int i=0; i<3; i++)
	{
		aktuellerBefehl[6+i] = tastenkommando[Taster-1][i];
	}

	/*if(aktuellerBefehl[6] == 0 && aktuellerBefehl[7]== 0 && aktuellerBefehl[8] == 1){
	//	turnON();
	} */

	sendCommando(aktuellerBefehl);
}

main ()						
{
	setup();

	do 
	{
		int Taster = 0;

		//Taster 1 gedrückt
		if(!(PINB&0b000001))
		{
			Taster = 1;
		}
		//Taster 2 gedrückt
	else if(!(PINB&0b000010))
		{
			Taster = 2;
		}
		//Taster 3 gedrückt
	else if(!(PINB&0b000100))
		{
			Taster = 3;
		}
	else
		{
			Taster = 0;		//nichts gedrückt
		}

		//Calls the assembleCommando-Function to process the pressed key
		if(Taster != 0)
		{
			assembleCommando(Taster);
		}

	}
	while (true);			
}

