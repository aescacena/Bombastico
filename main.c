#include <8051.h>
#include "sdcc_reg420.h"
#include <stdlib.h>
/* #include <conio.h> */
/* #include <iostream.h> */

#define lcd_data_pin P1 //Pines para mostrar datos en LCD
#define rs P3_1  //Register select (RS) pin
#define rw P3_0  // Read write (RW) pin
#define en P3_5  //Enable (EN) pin
#define startGame P3_2 //Start game (startGame) pin
#define exitGame P3_3 //Exit game (exitGame) pin
#define levelGame P3_4 //Select level game (levelGame) pin

void delay(unsigned int count);
void lcd_command(unsigned char comm);
void lcd_data(unsigned char disp);
void lcd_ini(void);
void character(void);
void writeCharacterLCD(unsigned char character);
void movCharacter(unsigned char direccion,unsigned char character);
unsigned int numAleatorio(unsigned int num);
void config(void);//Configuración general, interrupciones, temporizador... etc
void escreve4x7(void);
void runLevel(unsigned int);
void selectLevel(void);
void writePharse(unsigned char*);//Función para escribir una frase en LCD(Bienvenida, Leves, etc...)
void clearLCD(void);
void configInicial(void);//Configura las interrupciones para boton startGame(P3_2(INT0)) y boton exitGame(P3_3(INT1))
void onButtonSelectLevel(void); //Activa bits de TMOD, TRO y TFO necesarios para avilitar boton selectLevel(T0=3_4)
void offButtonSelectLevel(void);//Desactiva bits de TMOD, TRO y TFO necesarios para avilitar boton selectLevel(T0=3_4)
void resetDisplay(void);//Reinicia las variables en memoria que gestionan el Display
bit dead (void);//Devuelve 1 = dead, 0 != dead
bit victory(void);//Devuelve 1 = Win or 0 != Win
void resetPosition(void);//Resetea las posiciones de los muñecos
void positionBomba(void);//Coloca la Bomba en un arriba o abajo aleatoriamente, cogiendo de semilla positionBranco

__data unsigned char posicionActualBoneco=0x80;//Posicion inicial de muñeco
__data unsigned char posicionActualBomba=0x90;
__data unsigned char posicionActualFlecha=0x90;
__data unsigned char posicionActualFlechaBoneco=0x90;
__data unsigned char boneco=0;
__data unsigned char bomba=1;
__data unsigned char flecha=2;
__data unsigned char flechaBoneco=3;
__data unsigned char branco=7;
__data unsigned char dereita=0;
__data unsigned char esquerda=1;
__data unsigned char acima=2;
__data unsigned char abaixo=3;
unsigned int num;
unsigned char posicionBranco;
bit fin=0;
bit select=0;
unsigned char Tab[]={0b00111111,0b00000110,0b01011011,0b01001111,0b01100110,0b01101101,0b01111101,0b00000111,0b01111111,0b01100111};

__data __at(0x30) unsigned char digito1;
__data __at(0x36) unsigned char digito2;
__data __at(0x42) unsigned char digito3;
__data __at(0x48) unsigned char digito4;
int posTab1=0;
int posTab2=0;
int posTab3=0;
int posTab4=0;

void config(void)
{
	TMOD=0b000100010; //gate=0, C/T=0 e modo 2
	TR0=1;
	TF0=0;
	TL0=256-92;
	TH0=256-92;

	TL0=256-20;
	TH0=256-20;
	//EA=1;
	IT1=1;//CONFIGURA A EXT1 PARA SER ACTIVADA POR TRANSICION 0->1
	EX1=1;//Activa interrupción externa boton exitGame(P3_3)
	//IP=0b00010100;
}
void configInicial(void)
{
	IE0=0;//LIMPIA EL FLAG EXTERNO 0
	IT0=1;//CONFIGURA A EXT0 PARA SER ACTIVADA POR TRANSICION 0->1
	//IE=0b10000100;
	EA=1;//Activa interrupciones
	EX0=1;//Activa interrupción externa de boton startGame(P3_2)
}
void EX0startGame(void) __interrupt(0)
{
	//IE0=0;
	select=1;
	//selectLevel();
}
void EX1exitGame(void) __interrupt(2)
{
	clearLCD();
	lcd_command(0x85);
	writePharse("Bye bye");
	//EA=0;
	fin=1;
}
void onButtonSelectLevel(void)
{
	TMOD=0b00000110; //gate=0, C/T=1 pois o imput do timer é externo
	TR0=1;
	TF0=0;
	TL0=0;
	TH0=0;
}
void offButtonSelectLevel(void)
{
	TR0=0;
}
void delay(unsigned int count)  // Function to provide time delay in msec.
{
	int i;
	for(i=0;i<count;i++);
}
void lcd_command(unsigned char comm)  //Function to send command to LCD.
{
	lcd_data_pin = comm;
	en=1;
	rs=0;
	rw=0;
	//delay(1);
	en=0;
}
void lcd_data(unsigned char disp)  //Function to send data to LCD.
{
	lcd_data_pin=disp;
	en=1;
	rs=1;
	rw=0;
	//delay(1);
	en=0;
}
void lcd_ini()  //Function to initialize the LCD
{
	lcd_command(0x38);//00111000 D=1->8 bits, N=1->2 Lineas, F=0-> Caracter 5x10
	//delay(200);
	lcd_command(0x0C); //00001100 D=1-> Encendido LCD, C=0-> Cursor off, B=0-> Parpadeo off
	//delay(200);
}
void character()
{
	lcd_command(64); //1000000 Address where character MUÑECO is to be stored
	lcd_data(14);//01110
	lcd_data(14);//01110
	lcd_data(14);//01110
	lcd_data(4);//00100
	lcd_data(7);//00111
	lcd_data(4);//00100
	lcd_data(10);//01010
	lcd_data(17);//10001
	//delay(1);

	lcd_command(72); //1001000 Address where character NAVE is to be stored
	lcd_data(0);//00000
	lcd_data(4);//00100
	lcd_data(10);//01010
	lcd_data(10);//01010
	lcd_data(31);//11111
	lcd_data(31);//11111
	lcd_data(4);//00100
	lcd_data(0);//00000
	//delay(1);

	lcd_command(80); //1010000 Address where character FLECHA is to be stored
	lcd_data(0);//00000
	lcd_data(0);//00000
	lcd_data(0);//00000
	lcd_data(14);//01110
	lcd_data(0);//00000
	lcd_data(0);//00000
	lcd_data(0);//00000
	lcd_data(0);//00000
	//delay(1);

	lcd_command(88); //1011000 Address where character FLECHA_MUÑECO is to be stored
	lcd_data(0);//00000
	lcd_data(4);//00100
	lcd_data(2);//00010
	lcd_data(15);//01111
	lcd_data(2);//00010
	lcd_data(4);//00100
	lcd_data(0);//00000
	lcd_data(0);//00000
	//delay(1);
}
void writeCharacterLCD(unsigned char character)
{
	if(character==boneco)
		lcd_command(posicionActualBoneco);/*Ajusta la dirección de la DDRAM.
	  			La dirección es enviado y recibido después de este ajuste.
	  			"Dice donde colocar el caracter enviado posteriormente"*/
	if(character==bomba)
		lcd_command(posicionActualBomba);

	if(character==flecha || (TF0==1 && character==branco))
		lcd_command(posicionActualFlecha);

	if(character==branco)
		lcd_command(posicionBranco);

	if(character==flechaBoneco)
		lcd_command(posicionActualFlechaBoneco);

	lcd_data(character);	/*Caracter de posición Character CGRAM a colocar en dirección
				anteriormente indicado*/

	//delay(1);3
}
void resetPosition(void)
{
	posicionActualBoneco=0x80;
	posicionActualFlecha=0x90;
	posicionActualFlechaBoneco=0x90;
	posicionActualBomba=0x90;
}
void movCharacter(unsigned char direccion,unsigned char character){

	unsigned char posicion;//Posición a escribir en LCD

	if(boneco==character)
	{
		posicionBranco=posicionActualBoneco;
		//mueve arriba
		if(direccion==2)
		{
			if(posicionActualBoneco >= 0xC0 && posicionActualBoneco <= 0xCF)
			{
				/*Escribe en posicion  de LCD, la posicion 7 de CGRAM (blanco)*/
				writeCharacterLCD(branco);
				posicionActualBoneco=posicionActualBoneco-0x40;
				posicion=posicionActualBoneco;
			}
		}
		//mueve derecha
		if(direccion==0)
		{
			if((posicionActualBoneco >= 0x80 && posicionActualBoneco < 0x8E) ||
			(posicionActualBoneco >= 0xC0 && posicionActualBoneco < 0xCE))
			{
				writeCharacterLCD(branco);
				posicionActualBoneco=posicionActualBoneco+0x01;
				posicion=posicionActualBoneco;
			}
		}
		//mueve abajo
		if(direccion==3)
		{
			if(posicionActualBoneco >= 0x80 && posicionActualBoneco <= 0x8F)
			{
				writeCharacterLCD(branco);
				posicionActualBoneco=posicionActualBoneco+0x40;
				posicion=posicionActualBoneco;
			}
		}
		//mueve izquierda
		if(direccion==1)
		{
			if((posicionActualBoneco > 0x80 && posicionActualBoneco <= 0x8E) ||
			(posicionActualBoneco > 0xC0 && posicionActualBoneco <= 0xCE))
			{
				writeCharacterLCD(branco);
				posicionActualBoneco=posicionActualBoneco-0x01;
				posicion=posicionActualBoneco;
			}
		}
	}
	if(flecha==character)
	{
		posicionBranco=posicionActualFlecha;
		if(posicionActualFlecha==0x90)
		{
			posicionActualFlecha=posicionActualBomba-0x01;
			posicion=posicionActualFlecha;
		}
		else if(posicionActualFlecha==0xC0 || posicionActualFlecha==0x80)
		{
			writeCharacterLCD(branco);
			posicionActualFlecha=0x90;
			posicion=posicionActualFlecha;
			//characterr=0x01b;
		}
		else
		{
			writeCharacterLCD(branco);
			posicionActualFlecha=posicionActualFlecha-0x01;
			posicion=posicionActualFlecha;
		}
	}
	if(flechaBoneco==character)
	{
		posicionBranco=posicionActualFlechaBoneco;
		if(posicionActualFlechaBoneco==0x90)
		{
			posicionActualFlechaBoneco=posicionActualBoneco+0x01;
			posicion=posicionActualFlechaBoneco;
		}
		else if(posicionActualFlechaBoneco==0xCF || posicionActualFlechaBoneco==0x8F)
		{
			writeCharacterLCD(branco);
			posicionActualFlechaBoneco=0x90;
			posicion=posicionActualFlecha;
			//characterr=0x01b;
		}
		else
		{
			writeCharacterLCD(branco);
			posicionActualFlechaBoneco=posicionActualFlechaBoneco+0x01;
			posicion=posicionActualFlechaBoneco;
		}
	}
	if(bomba==character)
	{
		posicionBranco=posicionActualBomba;
		writeCharacterLCD(branco);
		num=numAleatorio(2);
		positionBomba();
	}
	writeCharacterLCD(character);
}
unsigned int numAleatorio(unsigned int num)//Buscar manera de conseguir numero totalmente aleatorio
{
	srand(posicionActualBoneco); //Utilizo la hr del sistema como semilla
	return rand()%num;//Esto saca un num aleatorio de 0 hasta num-1
}

void escreve4x7(void)
{
	digito1=Tab[posTab1];
	digito2=Tab[posTab2];
	digito3=Tab[posTab3];
	digito4=Tab[posTab4];

	__asm
		;*************************************************************************************
		;0x33 es la poscion de memoria interna donde vamos a guardar 1 numero de los 4 digitos
		;los numeros de los otros digitos se pueden guardar en posiciones consecutivas
		;los numero del 0 al 9 están almacenados en un array Tab, de donde en principio mvamos 
		;a ir cogiendo los numeros que nos van haciendo falta para mostarlos en el display
		;**************************************************************************************

		MOV r1,0x30
		MOV R2, 0x36
		MOV r3,0x42
		;MOV R4, 0x48

		mov P0,r1
		setb _P2_7
		clr _P2_7
		clr _P2_6
		clr _P2_5
		clr _P2_4
		MOV P0,R2
		setb _P2_6
		CLR _P2_6
		clr _P2_7
		clr _P2_5
		clr _P2_4
		mov P0,r3
		setb _P2_5
		clr _P2_7
		clr _P2_6
		clr _P2_5
		clr _P2_4
		;MOV P0,R4
		;setb _P2_4
		;CLR _P2_6
		;clr _P2_7
		;clr _P2_5
		;clr _P2_4
	__endasm;

}
void runLevel(unsigned int level)
{
	int moveBomba=0;//Cuando cuente 3 mueve bomba level basic, 2 mueve bomba level avanced
	fin=0;
	resetDisplay();
	clearLCD();
	writeCharacterLCD(boneco);//Escribre caracter en LCD, posicion enviada como parametro

	positionBomba();

	writeCharacterLCD(bomba);

	config();

	while(fin!=1)
	{
		if(TF0==1)
		{
			movCharacter(esquerda,flecha);
			/* "Si" esta en level Avanced
				"Si" pulsa P3_4 y posicion FlechaBoneco es pos reset "O" posicion no es pos reset
					Mueve flechaBoneco */
			if(level)
			{
				if((!P3_4 && posicionActualFlechaBoneco==0x90) || posicionActualFlechaBoneco!=0x90)
				{
					movCharacter(dereita,flechaBoneco);
				}
				if(posicionActualBomba==posicionActualFlechaBoneco)
				{
					posTab2++;
					if(posTab2>=10)
					{
						posTab2=0;
						posTab3++;
					}
					if(posTab3>=10)
					{
						posTab3=0;
						posTab4++;
					}
					if(posTab4>=10)
					{
						posTab4=0;
					}
					posicionActualFlechaBoneco=0x90;
					movCharacter(0,bomba);
				}
				if(moveBomba==2)
				{
					moveBomba=0;
					movCharacter(0,bomba);
				}
			}
			if(moveBomba==3)
			{
				moveBomba=0;
				movCharacter(0,bomba);
			}
			moveBomba++;
			TF0=0;
			//fin=dead();
			if(dead())
			{
				int aux=0;
				clearLCD();
				lcd_command(0x85);
				writePharse("Morto");
				lcd_command(0xC4);
				writePharse("The end");
				while(aux<10)
				{
					escreve4x7();
					aux++;
				}
				fin=1;
			}
			if(victory())
			{
				int aux=0;
				clearLCD();
				lcd_command(0x85);
				writePharse("Parabens !!");
				lcd_command(0xC5);
				writePharse("Voce ganhou !!");
				while(aux<10)
				{
					escreve4x7();
					aux++;
				}
				fin=1;
			}
			if(!fin)
			{
				if((posicionActualFlecha==0x80)||(posicionActualFlecha==0xC0))
				{
					posTab1=posTab1+5;
					if(posTab1>=10)
					{
						posTab1=0;
						posTab2++;
					}
					if(posTab2>=10)
					{
						posTab2=0;
						posTab3++;
					}
					if(posTab3>=10)
					{
						posTab3=0;
						posTab4++;
					}
					if(posTab4>=10)
					{
						posTab4=0;
					}
				}
			}
		}
		if(TF1=1)
		{
			TF1=0;
			escreve4x7();
		}
		if(!P2_0)
		{
			movCharacter(dereita,boneco);//Mueve derecha
		}
		if(!P2_1)
		{
			movCharacter(esquerda,boneco);//Mueve izquierda
		}
		if(!P2_2)
		{
			movCharacter(acima,boneco);//Mueve arriba
		}
		if(!P2_3)
		{
			movCharacter(abaixo,boneco);//Mueve abajo
		}
	}
	num=numAleatorio(2);//Genero num para la proxima posición de Bomba
	resetPosition();
	clearLCD();
	lcd_command(0x80);
	writePharse("Espera um pouco");
}
void selectLevel(void)
{
	int i=0;
	select=0;
	resetPosition();
	onButtonSelectLevel();
	clearLCD();
	lcd_command(0x80);
	writePharse("1 vez: Basico");
	lcd_command(0xC0);
	writePharse("2 vezes: Avancado");
	while(i<20)
	{
		i++;
		if(TL0==2)
		{
			break;
		}
	}
	if(TL0==2)
	{
		offButtonSelectLevel();
		runLevel(1);
	}
	else if(TL0==1)
	{
		offButtonSelectLevel();
		runLevel(0);
	}
	else
	{
		selectLevel();
	}
}
void writePharse(unsigned char* pharse1)
{
	while( *pharse1 ) lcd_data(*(pharse1++));
}
void clearLCD(void)
{
	lcd_command(0x01);//Comando que limpia LCD
	lcd_command(0x06);//0b0000110 Entrada de datos

}
void resetDisplay(void)
{
	posTab1=0;
	posTab2=0;
	posTab3=0;
	posTab4=0;
}
bit dead (void)
{
	if(posicionActualFlecha==posicionActualBoneco)
		return 1;
	else
		return 0;
}
bit victory(void)
{
	if(posTab3 >= 2)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
void positionBomba(void)
{
	if(num==0)
		posicionActualBomba=0x8F;
	if(num==1)
		posicionActualBomba=0xCF;
}
void main(void)
{
	num = numAleatorio(2);
	//__data unsigned char posicionActualBoneco=0x80;
	lcd_ini(); //Inicializa LCD
	//config();//Configuracion general
	character(); //Función para crear los caracteres especiales

	clearLCD();
	lcd_command(0x80);
	writePharse("By Ant. Marchena");
	lcd_command(0xC0);
	writePharse("Pressione start:");

	configInicial();

	while(1)
	{
		if(select==1)
		{
			select=0;
			selectLevel();
		}
	}
}