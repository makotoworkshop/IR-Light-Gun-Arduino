#include <Wire.h>
  
#define SENSOR_ADDRESS  0x58

// Global
byte data_buf[16];
int X1, Y1 , s1 ;
int X2, Y2 , s2 ;

// Camera Settings
byte setting1[9] = {0x00,0x00,0x00,0x00,0x00,0x00,0x90,0x00,0x41};
byte setting2[2] = {0x40,0x00};
  
void Write_2bytes(byte d1, byte d2)
{
    Wire.beginTransmission(SENSOR_ADDRESS);
    Wire.write(d1); Wire.write(d2);
    Wire.endTransmission();
}
 
void Write_Nbytes(byte d1, byte * ds, byte n) {
    Wire.beginTransmission(SENSOR_ADDRESS);
    Wire.write(d1);
    for ( byte i = 0; i <  n; i++ ){
      Wire.write(ds[i]);
    }
    Wire.endTransmission();
}

void ReadCamera()
{
   Wire.beginTransmission(SENSOR_ADDRESS);
    Wire.write(0x36);
    Wire.endTransmission();
    Wire.requestFrom(SENSOR_ADDRESS, 16); 
     
    for (byte i = 0 ; i < 16 ; i++)
    {
      data_buf[i] = 0 ;
      data_buf[i] = Wire.read();
      //Serial.print(data_buf[i]);
      //Serial.print(" ");
    }
    //Serial.println();
 
    X1 = data_buf[1];
    Y1 = data_buf[2];
    s1   = data_buf[3];
    X1 += (s1 & 0x30) <<4;
    Y1 += (s1 & 0xC0) <<2;
    s1 &= 0x0F ;

    if(data_buf[6] == 255)
    {
      data_buf[4] = data_buf[7];
      data_buf[5] = data_buf[8];
      data_buf[6] = data_buf[9];
      //Serial.println("3 in 2");      
    }

    if(data_buf[6] == 255)
    {
      data_buf[4] = data_buf[10];
      data_buf[5] = data_buf[11];
      data_buf[6] = data_buf[12];      
      //Serial.println("4 in 2");
    }

    X2 = data_buf[4];
    Y2 = data_buf[5];
    s2 = data_buf[6];
    X2 += (s2 & 0x30) <<4;
    Y2 += (s2 & 0xC0) <<2;
    s2 &= 0x0F ;
}

/* Pythagore */
float pythagore(int a, int b)
{
  float c;
  a *= a; b *= b;
  c = a+b;
  return(c);
}
