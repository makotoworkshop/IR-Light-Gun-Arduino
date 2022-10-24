/* Programme "Wii Camera to Pijama" v1.0
 * Par Aganyte, April 2020
 */
 // Modifs Makoto en cours…

#include <math.h>
#include "WiiLib.h"
#include "MouseAbs.h"

/* definition des equivalences */
#define	Ratio	 			        1 			      // Ratio de l'ecran 1 = 4/3 et 0 = 16/9
#define Xcenter				      fullscale/2		// Position du centre de la camera sur X
#define Ycenter				      fullscale/2		// Position du centre de la camera sur Y
#define fullscale	  		    255				    // Valeur Max (Resolution de la camera moins 1)
#define Resolution_Pn  	    255				    // Valeur Max (Resolution des potentiometres numerique moins 1)

/* Réglages */
unsigned int Screen_wide = 55; // largeur de l'écran en centimetres
signed int Y_Correction =  28;
signed int X_Correction = 0;
unsigned int Wiibar_size = 20; // Taille de la Wii bar en centimetres (ou distance entre les 2 emetteurs I.R)

void setup()
{
//  Serial.begin(115200); // Debug Mode (9600 for processing)
  Wire.begin();

// Décommenter TWBR si la cam ne fonctionne pas
//  TWBR = 1 ; // max bit rate possible 800khz // IR_cam support the Fast Plus Mode I2C  
  // To change the bit rate use TWBR = ((F_CPU / F_I2C) - 16) / 2
  // For example 400khz (Fast Mode) with 16Mhz oscillator => TWBR = ((16000000L / 400000L) - 16) / 2

  // IR sensor initialize
  Write_2bytes(0x30,0x08); delay(10);  // begin config
  Write_Nbytes(0x00,setting1,9); delay(10); // sensiblity 1
  Write_Nbytes(0x1A,setting2,2); delay(10); // sensiblity 2
  Write_2bytes(0x33,0x03); delay(10); // data mode extended
 
  Mouse.init(); // Activer le mode souris

  /* Activation des entrées */
  pinMode(4,INPUT_PULLUP);  // Click Droit (Reload)
  pinMode(5,INPUT_PULLUP);  // Click Gauche (Tir)
  pinMode(6,INPUT_PULLUP);  // Click Milieu (Start)
}

/* Programme principal */
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


    Serial.print("Bouton Clic Droit : ");
    Serial.println(digitalRead(4));
    Serial.print("Bouton Clic Gauche : ");  
    Serial.println(digitalRead(5));
    Serial.print("Bouton Clic Milieu : ");  
    Serial.println(digitalRead(6));

    /* RAFRAICHIR LES BOUTONS */
    // Gestion du clic gauche -> Tir
    if(digitalRead(5)==LOW)
      Mouse.press(MOUSE_LEFT);
    else
      Mouse.release(MOUSE_LEFT);
    
    // Gestion du clic droit désactivé pour mode RELOAD automatique cf. en bas
//    if(digitalRead(4)==LOW) -> Reload
//      Mouse.press(MOUSE_RIGHT);
//    else
//      Mouse.release(MOUSE_RIGHT);
    
    // Gestion du clic central  -> Start
    if(digitalRead(6)==LOW)
      Mouse.press(MOUSE_MIDDLE);
    else
      Mouse.release(MOUSE_MIDDLE);



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

    Serial.print("Xc : ");  
		Serial.print((int)Xc);
		Serial.print(",  Yc : ");
		Serial.println((int)Yc);

		if ( ((int)Xc == 0) & ((int)Yc == 0) )
		{
			Mouse.Actual_X = 1;
			Mouse.Actual_Y = 1; 
		}
		else
		{
			Mouse.Actual_X = map((int)Xc,0,255,32767,0);
			Mouse.Actual_Y = map((int)Yc,0,255,0,32767);
		} 
    Serial.print("Actual_X : "); 
		Serial.print((int)Mouse.Actual_X);
		Serial.print(",  Actual_Y : ");
		Serial.println((int)Mouse.Actual_Y);


    /* Mode RELOAD automatique si le curseur sort de l'écran */
//    if ( (Mouse.Actual_X == 1) & (Mouse.Actual_Y == 1) )
//    {
//      Serial.println("Reload !!! Clic Droit");   
//      Mouse.press(MOUSE_RIGHT);
//    }
//    else
//    {
////      Serial.println("Load !!! ");   
//      Mouse.release(MOUSE_RIGHT);
//    }

    Serial.println(""); // Paragraphe
    
		while( (millis() - RefreshTime) < 20); // 20ms => 50 Hertz
		Mouse.report(); // mettre à jour le curseur et les boutons
	}
}
