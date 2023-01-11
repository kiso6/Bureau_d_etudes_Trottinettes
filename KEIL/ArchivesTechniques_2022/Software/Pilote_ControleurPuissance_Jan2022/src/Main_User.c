
/*
	!!!! NB : ALIMENTER LA CARTE AVANT DE CONNECTER L'USB !!!

VERSION 16/12/2021 :
- ToolboxNRJ V4
- Driver version 2021b (synchronisation de la mise à jour Rcy -CCR- avec la rampe)
- Validé Décembre 2021

*/


/*
STRUCTURE DES FICHIERS

COUCHE APPLI = Main_User.c : 
programme principal à modifier. Par défaut hacheur sur entrée +/-10V, sortie 1 PWM
Attention, sur la trottinette réelle, l'entrée se fait sur 3V3.
Attention, l'entrée se fait avec la poignée d'accélération qui va de 0.6V à 2.7V !

COUCHE SERVICE = Toolbox_NRJ_V4.c
Middleware qui configure tous les périphériques nécessaires, avec API "friendly"

COUCHE DRIVER =
clock.c : contient la fonction Clock_Configure() qui prépare le STM32. Lancée automatiquement à l'init IO
lib : bibliothèque qui gère les périphériques du STM : Drivers_STM32F103_107_Jan_2015_b
*/



#include "ToolBox_NRJ_v4.h"




//=================================================================================================================
// 					USER DEFINE
//=================================================================================================================





// Choix de la fréquence PWM (en kHz)
#define FPWM_Khz 20.0
#define Fe 5000.0
#define Ft 400.0



//On define les valeurs de R et L pour régler finement le correcteur
#define R 1.0
#define L	0.002		
#define K1 1.0/R
#define K2 1.0+12.0/10.0
#define K3 10000.0/15100.0
#define Gcapt 0.104
#define Vbat 24.0

//==========END USER DEFINE========================================================================================

// ========= Variable globales indispensables et déclarations fct d'IT ============================================

void IT_Principale(void);
//=================================================================================================================


/*=================================================================================================================
 					FONCTION MAIN : 
					NB : On veillera à allumer les diodes au niveau des E/S utilisée par le progamme. 
					
					EXEMPLE: Ce progamme permet de générer une PWM (Voie 1) à 20kHz dont le rapport cyclique se règle
					par le potentiomètre de "l'entrée Analogique +/-10V"
					Placer le cavalier sur la position "Pot."
					La mise à jour du rapport cyclique se fait à la fréquence 1kHz.

//=================================================================================================================*/
float Ki,Kp,C1,C2;

float eps[2] = {0.0, 0.0};
float alpha[2] = {0.0, 0.0};

float Te,Te_us;
// ------------- Calculs des coef de filtres -------------------

	
int main (void)
{
// !OBLIGATOIRE! //	
Conf_Generale_IO_Carte();	
// ------------- Discret, choix de Te -------------------	
Te=	1/Fe; // en seconde
Te_us=Te*1000000.0; // conversion en µs pour utilisation dans la fonction d'init d'interruption

	
Ki = 2.0*3.1415*Ft*R/(2*Vbat*Gcapt*K2*K3);
Kp = Ki*L/R;
C1 = Kp+(Te*(Ki/2.0));
C2 = Kp-(Te*(Ki/2.0));



//______________ Ecrire ici toutes les CONFIGURATIONS des périphériques ________________________________	
// Paramétrage ADC pour entrée analogique
Conf_ADC();
// Configuration de la PWM avec une porteuse Triangle, voie 1 & 2 activée, inversion voie 2
Triangle (FPWM_Khz);
Active_Voie_PWM(1);	
Active_Voie_PWM(2);	
Inv_Voie(2);

Start_PWM;
R_Cyc_1(2048);  // positionnement à 50% par défaut de la PWM
R_Cyc_2(2048);

// Activation LED
LED_Courant_On;
LED_PWM_On;
LED_PWM_Aux_Off;
LED_Entree_10V_On;
LED_Entree_3V3_Off;
LED_Codeur_Off;

// Conf IT
Conf_IT_Principale_Systick(IT_Principale, Te_us);

	while(1)
	{}

}

//=================================================================================================================
// 					FONCTION D'INTERRUPTION PRINCIPALE SYSTICK
//=================================================================================================================
int Courant_1,Cons_In;


void IT_Principale(void)
{
 //Cons_In=Entree_10V();


	eps[0] = (Entree_3V3()-I1())*3.3/4095;
	alpha[0] = C1*eps[0]-C2*eps[1]+alpha[1];
	
	alpha[0]=(alpha[0] > 0.5)?0.5:(alpha[0]<-0.5)?-0.5:alpha[0]; //saturation
	
	Cons_In = (int)(alpha[0]*4095.0)+2048;
	R_Cyc_1(Cons_In);
  R_Cyc_2(Cons_In);
	eps[1]=eps[0];
	alpha[1]=alpha[0];
	
}

