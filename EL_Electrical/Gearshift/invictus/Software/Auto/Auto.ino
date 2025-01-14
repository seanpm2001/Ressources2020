/**************************************************************************/
/*!
    @file     main_carte.cpp
    @author   Bruno Moreira Nabinger and Corentin Lepais (EPSA)
                                                  Ecurie Piston Sport Auto
    
    Main code for control the Motor with integrated Controller and CAN 
    interface BG 45 CI

 
    @section  HISTORY
    v0.7 - 17/04/2019 Bob Aubouin change a lot of things to associate the code with the real word
    v0.6 - 06/04/2019 Add the sending of the speed to the DTA
    v0.5 - 13/02/2019 Modifications linked to the class CAN
    v0.4 - 06/02/2019 Add fonction to can_interface
    v0.3 - 10/11/2018 Comments of code and add of the file 
           "projectconfig.h", with the definitions of the pins numbers.
    v0.2 - 17/10/2018 Management of pallets and beginning of creations of 
                      the associated functions
    v0.1 - 10/10/2018 First release (previously code of Pedro)
*/
/**************************************************************************/
#include "projectconfig.h"

//Ajout des fonctions
#include "fonct_palette_homing.h"
#include "fonct_mot.h"
#include "can_interface.h"

//initialisation Canbus
#include <SPI.h>

//Definition of key-word
#define ERREUR 1 //Use to transmit that the motor is in error

//Definition of used variables 
boolean statePaletteIncrease; 
boolean statePaletteIncreaseBefore;
boolean statePaletteDecrease;
boolean statePaletteDecreaseBefore;

int positionEngagee; // Contain what motor position is currently engaged
int wantedPosition;// Contain the motor position wanted so the speed rapport of the bike



boolean outMotor1; //Info return by the motor
boolean outMotor2;//Info return by the motor

boolean homingState; // Will contain the state of the homing button
boolean neutreState; // Will contain the state of the neutral button
boolean shiftcutState;
boolean gearpotState;
boolean launchcontrolState;
boolean wetdryState;     
   
boolean error;
const int neutrePosition = 2;
const int homingPosition=1;

boolean positionReached=true;

unsigned long T_Descente;
unsigned long T_Montee;

int valAnalog[9];


int canStable;

//Auto
bool Auto; // if true we are in auto mode
int RPM_shift[3] = {12500,12200,12000};
int push_time = 5000; //time in ms to activate the auto mode
unsigned long T_Auto;
unsigned long T_ShiftAuto;
signed RPM ;



//Table which will contain the combination of the motor input for each speed
boolean motorPosition[7][3];//We use only 4 input motor to command it.

//Initialization of CANBUS
can_interface CAN;

void setup() 
{ 
  
  Serial.begin(115200);
  //Initialization of the pins
  pinMode(motorState1, INPUT);
  pinMode(motorState2, INPUT);
  pinMode(motorInput1, OUTPUT);
  pinMode(motorInput2, OUTPUT);
  pinMode(motorInput3, OUTPUT);
  pinMode(motorInput4, OUTPUT);
  pinMode(shiftCut, OUTPUT); 
  pinMode(gearPot, OUTPUT);
  pinMode(pinLED, OUTPUT); 

  pinMode(paletteIncrease, INPUT);
  pinMode(paletteDecrease, INPUT);
  
  digitalWrite(motorInput1, LOW);
  digitalWrite(motorInput2, LOW);
  digitalWrite(motorInput3, LOW);
  digitalWrite(motorInput4, LOW);
  digitalWrite(shiftCut, HIGH);
  digitalWrite(pinLED,LOW);

  //Initialization of the variables
  statePaletteIncreaseBefore = LOW; //The pallets mode is INPUT but there is a PULL Down resistor on the shield so the inactive state is low
  statePaletteDecreaseBefore = LOW;
  
  positionEngagee = 0; // We don't know but this normaly not the homming position
  wantedPosition = 1; // First we want to do the homming of the motor
  error = false; // we suppose that there is no error 
  T_Descente = millis();
  T_Montee = millis();
  
    //mappage des valeurs de tensions envoyés au DTA en fonction de la vitesse (0->0.2V .... 6->5V)
  valAnalog[0] = 0;
  valAnalog[1] = 0;
  valAnalog[2] = 43;
  valAnalog[3] = 77;
  valAnalog[4] = 110;
  valAnalog[5] = 143;
  valAnalog[6] = 176;
  valAnalog[7] = 209;
  
    //Initialization of the table. We use only the position 1-6, clear error and start Homing
    
    //Clear error and Stop
    motorPosition[0][0] = 0;
    motorPosition[0][1] = 0;
    motorPosition[0][2] = 0; 

    //Start Homing
    motorPosition[1][0] = 1;
    motorPosition[1][1] = 0;
    motorPosition[1][2] = 0;


    //Position 1: Neutre
    motorPosition[2][0] = 0;
    motorPosition[2][1] = 1;
    motorPosition[2][2] = 0;

    //Position 2 : vitesse 1 
    motorPosition[3][0] = 1;
    motorPosition[3][1] = 1;
    motorPosition[3][2] = 0;


    //Position 3 : vitesse 2
    motorPosition[4][0] = 0;
    motorPosition[4][1] = 0;
    motorPosition[4][2] = 1;

    //Position 4 : vitesse 3
    motorPosition[5][0] = 1;
    motorPosition[5][1] = 0;
    motorPosition[5][2] = 1;

    //Position 5 : vitesse 4
    motorPosition[6][0] = 0;
    motorPosition[6][1] = 1;
    motorPosition[6][2] = 1;

    //Position 6 : vitesse 5
    motorPosition[7][0] = 1;
    motorPosition[7][1] = 1;
    motorPosition[7][2] = 1;

    

   

  //Auto
  Auto = false; //we start in normal mode
  T_Auto = millis();
  T_ShiftAuto = millis();

  //Homming du motored
   EngageVitesse(wantedPosition);
   outMotor1 = digitalRead(motorState1);
   outMotor2 = digitalRead(motorState2);
    while (!PositionReachedOrHomingDone(outMotor1,outMotor2))
    {
         outMotor1 = digitalRead(motorState1);
         outMotor2 = digitalRead(motorState2);
    }
    positionEngagee = wantedPosition; 
    //wantedPosition = 2; //on débute au neutre comme ça pas de surprise
   analogWrite(gearPot, 255);
  
  canStable = 0;
}

void loop() 
{ 
  //MAJ of attributs by receiving the last datas
  CAN.Receive();
  outMotor1 = digitalRead(motorState1);
  outMotor2 = digitalRead(motorState2);

  //Control of pallet+
  statePaletteIncrease = digitalRead(paletteIncrease);
  //If we only test if the state of the palette have changed or if we only test if the state is now 1, the test is going to be true 2 times:
  // when the pilot presses the palette and when it is released. We want to change the speed only  one time for each press

  //0 -> 1 = Detect the rising edge of signal statePaletteIncrease;
  if (statePaletteIncrease != statePaletteIncreaseBefore)// Check if state have changed
  {  
    if (statePaletteIncrease) // Check if state changed to 1, so we have the rising edge 0 -> 1 
    { 
      Auto = false;
      // Si demande de Homming
      if (NeedHoming(outMotor1,outMotor2))
      {
          if (statePaletteIncrease) // Check if state changed to 1, so we have the rising edge 0 -> 1
          {
            wantedPosition = 1; //On fait le homming
          }
      }
      else if(PassageVitesseIsPossible(positionEngagee+1)) 
      {
        if ((millis() - T_Montee) > 200)
        {
        digitalWrite(shiftCut, LOW);//Close the injection
        wantedPosition = positionEngagee+1;
        T_Montee = millis();
        }
       }
       else if (positionEngagee == 1)
       {  if ((millis() - T_Montee) > 200)
          {
          wantedPosition = positionEngagee+2;
          T_Montee = millis();
          }
       }
    }
    statePaletteIncreaseBefore = statePaletteIncrease;
  }
  //Control of pallet -
  statePaletteDecrease = digitalRead(paletteDecrease);
  //If we only test if the state of the palette have changed or if we only test if the state is now 1, the test is going to be true 2 times:
  // when the pilot presses the palette and when it is released. We want to change the speed only  one time for each press
  //0 -> 1 = Detect the rising edge of signal statePaletteDecrease;
  if (statePaletteDecrease != statePaletteDecreaseBefore)// Check if state have changed
  {
    if (statePaletteDecrease) // Check if state changed to 1, so we have the rising edge 0 -> 1
    {
      if(PassageVitesseIsPossible(positionEngagee-1)) //on vérifie que la vitesse à laquelle on veut passer est atteignable
      {
        if ((millis() - T_Descente) > 200)
        {
          Auto = false;
          wantedPosition = positionEngagee-1;
          T_Descente = millis();
          T_Auto = millis();
        }
      }
       else if (positionEngagee == 3 and wantedPosition != positionEngagee-1){
        if (Auto==0 and (millis() - T_Auto) >1000){
          Auto = true;
        }
        //else {
          //Auto = false;
       // }
      }  
    }
    statePaletteIncreaseBefore = statePaletteIncrease;
  }
  

  //NEUTRE
  neutreState=CAN.getNeutreState();
  canStable++;
  canStable = min(canStable,32500);
  if(neutreState and canStable > 32000) //if state is 1 
  {
    Auto = false;
    wantedPosition = neutrePosition;
  }

  
  //HOMING
  homingState=CAN.getHomingState(); //We have the state of the homing thank to the can attribut

  if(homingState and canStable > 32000) //if state is true 
  {
    Auto = false;
    wantedPosition=homingPosition;
  }

  //SHIFTCUT
  shiftcutState=CAN.getShiftcutState();
  if(shiftcutState and canStable > 32000) //if state is true 
  { digitalWrite(shiftCut,HIGH); // Pas fini __ARS

  }
  
  //GEARPOT
  gearpotState=CAN.getGearpotState();
  if(gearpotState and canStable > 32000) //if state is true 
  { digitalWrite(gearPot,HIGH); // Pas fini __ARS

  }
  //LAUNCH CONTROL
  launchcontroltState=CAN.getLaunchcontrolState();
  if(canStable > 32000) //if state is true 
  { digitalWrite(launchControl,launchcontroltState);
  }
  //WET DRY
  wetdryState=CAN.getWetdryState();
  if(canStable > 32000) 
  { digitalWrite(wetDry,wetdryState);
  }



//  //AUTO

//Le mode s'active lorsqu'on appuie sur doswn shift et que l'on est en première


if (Auto==true)
   {
    RPM=CAN.getRPM();
    if (millis()-T_ShiftAuto>500) {
      wantedPosition = CalculAuto(positionEngagee,RPM);
    }
    if (wantedPosition!=positionEngagee) {
      T_ShiftAuto = millis();
    }
   }

  // MOVE
  if (wantedPosition!=positionEngagee) //We try to change speed only if the pilot demands it
  {
    if (wantedPosition == positionEngagee-1) {
      delay(20);
      }
    else {
      delay(10);
    }
      
    EngageVitesse(wantedPosition);
    outMotor1 = digitalRead(motorState1);
    outMotor2 = digitalRead(motorState2);
    
    char flagQuitWhile = 0;
    
    while(MotorIsTurning(outMotor1,outMotor2)) //while the motor is turning
    { flagQuitWhile++;
      outMotor1 = digitalRead(motorState1);
      outMotor2 = digitalRead(motorState2);
      if (flagQuitWhile > 10)
      {  
        flagQuitWhile = 0;
        break;
      }
      
    }
    if (PositionReachedOrHomingDone(outMotor1,outMotor2)) //We change the current speed only if we have reached the position
    {
      positionEngagee=wantedPosition; //We save the engaged position
    }
    digitalWrite(shiftCut, HIGH);//Open the injection
    TransmetToDTATheGear(positionEngagee-2); // We send to the DTA the engaged gear
  }

  // Transmission des erreurs : le moteur est en erreur, le moteur veut encore tourner ou le moteur veut faire son homming
  if ((MotorIsLost(outMotor1,outMotor2)) or (MotorIsTurning(outMotor1,outMotor2)) or (NeedHoming(outMotor1,outMotor2)))
      {
        CAN.Transmit(positionEngagee-2, ERREUR,Auto);
      }
  else 
  {
    CAN.Transmit(positionEngagee-2, 0,Auto);
  }
}

void EngageVitesse(int wantedPosition) //Function which pass the speed
{
  digitalWrite(motorInput1, motorPosition[wantedPosition][0]);
  digitalWrite(motorInput2, motorPosition[wantedPosition][1]);
  digitalWrite(motorInput3, motorPosition[wantedPosition][2]);
}

void TransmetToDTATheGear(int rapportEngage)
{
  analogWrite(gearPot,valAnalog[rapportEngage]);
}

int CalculAuto(int pos,int RPM){
  if (pos<=2) {
    return(3);
  }
  else if (pos==3 && RPM>RPM_shift[0]){
    return(4);
  }
  else if (pos==4 && RPM>RPM_shift[1]){
    return(5);
  }
  else if (pos==5 && RPM>RPM_shift[2]){
    return(6);
    }
  else {
    return(pos);
    }
}
