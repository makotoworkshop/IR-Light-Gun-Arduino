/* Programme "Wii Camera to Pijama" v1.0
 * Par Aganyte, April 2020
 * Modifs Makoto Novembre 2022 :
 *  ok - Ajout du HID Joystick simultané (Stick XY et boutons)
 *  ok - Mode Souris : Reload activé si la souris sort de l'écran.
 *  ok - Switch de sélection pour désactiver le mode Souris.
 *  ok - Switch de sélection pour désactiver le mode Reload.
 */

#include <math.h>
#include "WiiLib.h"
#include "MouseAbs.h"
#include <Joystick.h> // Librairie Joystick by Matthew Heironimus V2.0.7

// Définition du Joystick
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,JOYSTICK_TYPE_JOYSTICK,
  3, 0,                           // Button Count, Hat Switch Count
  true, true, false,              // X and Y, but no Z Axis
  false, false, false,            // No Rx, Ry, or Rz
  false, false,                   // No rudder or throttle
  false, false, false);           // No accelerator, brake, or steering
const int pinToButtonMap = 4;     // Constant that maps the physical pin to the first joystick button.
int lastButtonState[3] = {0,0,0}; // Last state of the 3 buttons
int lastReloadState = 0;          // Spécial pour Reload qui n'est pas câblé

// Définition et réglages Camera IR
#define	Ratio	 			        1 			      // Ratio de l'ecran 1 = 4/3 et 0 = 16/9
#define Xcenter				      fullscale/2		// Position du centre de la camera sur X
#define Ycenter				      fullscale/2		// Position du centre de la camera sur Y
#define fullscale	  		    255				    // Valeur Max (Resolution de la camera moins 1)
#define Resolution_Pn  	    255				    // Valeur Max (Resolution des potentiometres numerique moins 1)
unsigned int Screen_wide = 55;            // largeur de l'écran en centimetres
signed int Y_Correction =  26;            // Décallage en Hauteur par rapport à la barre IR
signed int X_Correction = 0;              // Décallage en Largeur (ou déplacer un tipeu la barre ;))
unsigned int Wiibar_size = 20;            // Distance entre les 2 emetteurs Infra-Rouges

unsigned int selecteur = 0;
unsigned int ModeReload = 0;

void setup()
{
//  Serial.begin(115200); // Debug Mode (9600 for processing)
  Wire.begin();

  // Initialisation Camera IR (Décommenter TWBR si la cam ne fonctionne pas)
  // TWBR = 1 ; // max bit rate possible 800khz // IR_cam support the Fast Plus Mode I2C  
  // To change the bit rate use TWBR = ((F_CPU / F_I2C) - 16) / 2
  // For example 400khz (Fast Mode) with 16Mhz oscillator => TWBR = ((16000000L / 400000L) - 16) / 2
  Write_2bytes(0x30,0x08); delay(10);  // begin config
  Write_Nbytes(0x00,setting1,9); delay(10); // sensiblity 1
  Write_Nbytes(0x1A,setting2,2); delay(10); // sensiblity 2
  Write_2bytes(0x33,0x03); delay(10); // data mode extended

  // Bouton de Souris
  pinMode(4,INPUT_PULLUP);  // Click Droit (Reload) non cablé, actif par logiciel Souris ou Joystick
  pinMode(5,INPUT_PULLUP);  // Click Gauche (Tir)
  Mouse.init(); // Activer le mode Souris

  // Boutons de Joystick
  pinMode(6, INPUT_PULLUP);
  Joystick.setXAxisRange(0, 255);
  Joystick.setYAxisRange(0, 255);
  Joystick.begin();

  // Switch selecteurs
  pinMode(8, INPUT_PULLUP); // Mode reload
  pinMode(9, INPUT_PULLUP); // (Actif par défaut si non câblé) Désactivation Mode Souris (le HID est présent, mais les data ne sont plus envoyées à la souris
}


void loop()
{
  /* Variables */
  unsigned char Counter = 0;
  unsigned int Ymoy;
  signed int Xecart,Yecart,Xecart2,Yecart2;
  float Ecart, Ecart2, Ecart3, Ecart4, Ecart_2, Ecart2_2, Ecart3_2, Xc, Yc, Yc_2, Highsize, Widesize, Produit_Vectoriel; 

  Mouse.Actual_X = 32767/2;
  Mouse.Actual_Y = 32767/2;
  
	/* Boucle infinie du mode run */
	for(;;)
	{
    unsigned long RefreshTime = millis();
		
		/* Capturer 2 blobs */
		ReadCamera();

		/* Convertion 8 bits sur X et Y */
		X1 >>= 2;
		Y1 >>= 2;
		X2 >>= 2;
		Y2 >>= 2;

		/* Inverser les blobs 1 et 2 si le blob 1 est a droite */
		if(X1>X2)
		{
			unsigned char Data1 = Y1; Y1 = Y2; Y2 = Data1; // inversion sur Y
			Data1 = X1; X1 = X2; X2 = Data1; // inversion sur X
		}

		// Calcul de Yc et Xc
		Ymoy = (Y1 + Y2) / 2;
		Xecart = X2-X1; 
		Yecart = Y2-Y1;
		Xecart2 = Xcenter-X1; 
		Yecart2 = Ycenter-Y1;
		Ecart_2 = pythagore(Xecart,Yecart);
		Ecart = sqrt(Ecart_2);
		Ecart2_2 = pythagore(Xecart2,Yecart2);
		Ecart2 = sqrt(Ecart2_2);
		Widesize = (Ecart * Screen_wide) / Wiibar_size;
		if(Ratio == 1) 
		  Highsize = (Widesize * 3) / 4; 
		else if (Ratio == 0) 
		  Highsize = (Widesize * 9) / 16;
		Produit_Vectoriel = (Xecart*Yecart2)-(Xecart2*Yecart); 
		Produit_Vectoriel = abs(Produit_Vectoriel);
		Yc = (Produit_Vectoriel) / Ecart;
		Yc_2 = Yc * Yc;
		Xc = sqrt(Ecart2_2 - Yc_2);
		Xecart = (Xcenter-X2); 
		Yecart = (Ycenter-Y2);
		Ecart3_2 = pythagore(Xecart,Yecart);
		Ecart4 = (Widesize - Ecart) / 2;
		if ( Ecart3_2 >= (Ecart_2 + Ecart2_2)) 
		  Xc = (Ecart4 - Xc); 
		else 
		  Xc = (Xc + Ecart4);

		/* Correction des axes X et Y */
		Yc -= ( Y_Correction * Widesize ) / Screen_wide; 
		Xc -= ( X_Correction * Widesize ) / Screen_wide;

//    Serial.print("Bouton Clic Droit : ");
//    Serial.println(digitalRead(4));
//    Serial.print("Bouton Clic Gauche : ");  
//    Serial.println(digitalRead(5));

    /* RAFRAICHIR LES BOUTONS */
    // Gestion du clic gauche -> Tir
    if(digitalRead(5)==LOW)
      Mouse.press(MOUSE_LEFT);
    else
      Mouse.release(MOUSE_LEFT);
    
		/* Calcul de XC et YC et conversion 8 bits */
		Yc = (Yc * Resolution_Pn ) / Highsize;
		Xc = (Xc * Resolution_Pn ) / Widesize;

		if(Yc<0)
		  Yc = 0;
		if(Yc>255)
		  Yc = 255;
		if(Xc<0)
		  Xc = 0;
		if(Xc>255)
		  Xc=255;

//    Serial.print("Xc : ");  
//		Serial.print((int)Xc);
//		Serial.print(",  Yc : ");
//		Serial.println((int)Yc);

		if ( ((int)Xc == 0) & ((int)Yc == 0) )
		{
			Mouse.Actual_X = 1;
			Mouse.Actual_Y = 1; 
		}
		else
		{
			Mouse.Actual_X = map((int)Xc, 0, 255, 32767, 0);
			Mouse.Actual_Y = map((int)Yc, 0, 255, 0, 32767);
		} 
//    Serial.print("Souris Actual_X : "); 
//		Serial.print((int)Mouse.Actual_X);
//		Serial.print(",  Actual_Y : ");
//		Serial.println((int)Mouse.Actual_Y);

    /* Mode RELOAD automatique si le curseur sort de l'écran, géré par logiciel */
    ModeReload = digitalRead(8);
    if (ModeReload == 0)  // Si le mode Reload est activé avec le switch Reload
    {
      if ( (Mouse.Actual_X == 1) & (Mouse.Actual_Y == 1) )  // si on sort de l'écran
      {
//        Serial.println("Reload !!! Clic Droit");   
        Mouse.press(MOUSE_RIGHT);
        Joystick.setButton(0, HIGH);  // Le reload est sur l'entrée 4, première décrite, donc « button 0 »
        lastReloadState = HIGH;
      }
      else
      {
        Mouse.release(MOUSE_RIGHT);
        Joystick.setButton(0, LOW);
        lastReloadState = LOW; 
      }
    }
    else
    {
//      Serial.println("Mode Reload désactivé");
      Mouse.release(MOUSE_RIGHT);
      Joystick.setButton(0, LOW);
      lastReloadState = LOW; 
    }

//    Serial.println(""); // Paragraphe

// Bloc JOYSTICK à situer impérativement ici
  // Valeurs du Stick envoyées au HID Device
    int xAxis = map(Xc, 0, 255, 255, 0);  // pour inverser l'axe X
    int yAxis = Yc;

//    Serial.print("Joystick xAxis = ");
//    Serial.print(xAxis);
//    Serial.print(",  yAxis = ");
//    Serial.println(yAxis);
  
    Joystick.setXAxis(xAxis);
    Joystick.setYAxis(yAxis);

  // Valeurs des boutons envoyées au HID Device
    for (int index = 0; index < 3; index++)
    {
    int currentButtonState = !digitalRead(index + pinToButtonMap);
    if (currentButtonState != lastButtonState[index])
      {
        Joystick.setButton(index, currentButtonState);
        lastButtonState[index] = currentButtonState;
      }
    }
//    Serial.print("Bouton Joystick : ");
//    Serial.println(digitalRead(6));
// Bloc JOYSTICK Fin
    
		while( (millis() - RefreshTime) < 20); // 20ms => 50 Hertz

    selecteur = digitalRead(9);
    if (selecteur == 1)
  		Mouse.report(); // mettre à jour le curseur et les boutons
    else
      Serial.println("Mode Souris désactivé");
	}
}
