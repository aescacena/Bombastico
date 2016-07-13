# Bombastico

![Bombastico](/Images/Bombastico.jpg)

O jogo termina se o boneco for atingido por uma bomba ou quando obtiver 200 pontos. A aplicação é suportada por um 
microcontrolador 8051, devendo ser desenvolvida e testada com o MCU8051IDE e respectivos periféricos. O código da aplicação deve 
ser feito em C e Assembly inline para blocos de código específicos.

Juego que consiste en esquivar bombas. Las bombas aparecen aleatoria mente por el límite derecho hasta el límite izquierdo del LCD. 
El muñeco puede moverse por todo el LCD, en cualquier dirección (controlado por el jugador), debiendo esquivar las bombas. El juego
termina cuando una bomba toca al muñeco o cuando obtiene 200 puntos. La aplicación esta desarrollada para un microcontrolador 
8051, un LCD y 4 botones.

1. Función del juego
    Este juego se compone de dos niveles, nivel BASICO y AVANZADO.
    
    + Básico:
      Este nivel consiste en esquivar las flechas que lanza la bomba hasta conseguir 200 puntos.
      El juego termina al conseguir 200 puntos o al ser alcanzado por una flecha.
      Cada flecha esquivada suma 5 puntos.
      
    + Avanzado:
    
      Este nivel tiene una diferencia respecto al anterior, y es que el Muñeco puede lanzar flechas.
      El juego termina al conseguir 200 puntos o al ser alcanzado por una flecha.
      Cada flecha esquivada suma 5 puntos.
      Cada bomba alcanzada por una flecha suma 10 puntos.
      
    Se puede parar el proceso del juego pulsando el botón abandonar, el cual vuelves al inicio del juego
    
2. Funciones básicas

  a. configInicial
    
    ```c
    void configInicial(void)
    {
      IE0=0; //LIMPIA EL FLAG EXTERNO 0
      IT0=1; //CONFIGURA A EXT0 PARA SER ACTIVADA POR TRANSICION 0->1
      EA=1; //Activa interrupciones
      EX0=1; //Activa interrupción externa de botón(P3_2)
    }
    ```
    
    Esta función activa las interrupciones de botón startGame (P3_2), la cual se utiliza para iniciar partida.
    
  b. Config
  
    ```c
    void config(void)
    {
      TMOD=0b000100010; //C/T=0, GATE=0, Modo=2
      TR0=1; //Permite contar
      TF0=0;
      // Temporizador de 100 microsegundos.
      TL0=256-92;
      TH0=256-92;
      IT1=1; //Configura interrupción para ser activada por transición 0->1
      EX1=1; //Activa interrupción externa botón (P3_3)
    }
    ```
    
    Esta configuración se usa para activar temporizador encargado de mostrar puntos por el display (4x7 segmentos) y 
    además activa la interrupción de P3_3 para abandonar juego (exitGame)
  
  c. Escreve4x7
  
  ```c
    void escreve4x7(void)
    {
      digito1=Tab[posTab1];
      digito2=Tab[posTab2];
      digito3=Tab[posTab3];
      digito4=Tab[posTab4];
      __asm
      MOV r1,0x30
      MOV R2, 0x36
      MOV r3,0x42
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
      __endasm;
    }
    ```
    
    Digito1, digito2, digito3 son variables guardadas en memoria interna respectivamente en las direcciones 0x30, 0x36, 0x42.
    Son variables donde vamos a guardar 1 número de la tabla Tab[] para luego mostrarlos por el display.
    Lo guardamos en memoria interna para que puedan comunicarse codigo C con ASM
  
  d. runLevel
  
  Este código no lo vamos a pegar aquí porque es un código muy lardo.
  Este es un metodo que recive una variable la cual indica si es nivel BASICO o nivel AVANZADO.
  Este metodo controla los movimientos del muñeco (arriba, abajo,derecha, izquierda)
  Mueve la flecha de Bomba y flecha de muñeco si está en nivel Avanzado.
  Suma puntos al contador para mostrarlos por el display.
  Y controla si la partida ha terminado por abandono, por muerte o victoria.
  Aquí abajo mostramos un diagrama de flujo.
  
  ![Bombastico](/Images/Flujo_runLevel.jpg)
  
  e. selectLevel
  
  ```c
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
    while(i<20){
      i++;
      if(TL0==2){
        break;
      }
    }
    if(TL0==2){
      offButtonSelectLevel();
      runLevel(1);
    }
    else if(TL0==1){
      offButtonSelectLevel();
      runLevel(0);
    }
    else{
      selectLevel();
    }
  }
  ```
  
  Este método imprime por LCD un mensaje mostrando las opciones de juegos de las que dispone.
  El cual después de este mensaje tienes un temporizador para seleccionar en que nivel deseas jugar.
  Si el temporizador llega alfinal vuelve a ejecutarse el programa.
