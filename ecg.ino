#include <math.h>
#include <stdio.h>
#include <stdlib.h>

//-----------------------------------------------------------------------------
//
// Mesure du rythme cardiaque (Capteur frequence cardiaque).
//
//  Lire les commentaires pour comprendre le programme. Remplacer les "????" par
//  une valeur ou un code adéquat avant d'executer le programme.
//  Ne pas modifier les lignes qui ne comportent pas de "????"
//-----------------------------------------------------------------------------

// Definition des fonctions de la librairie
//------------------------------------------
void Init_CapteurEcg(void);
int Mesure_FrequenceEcg(void);

//  LED tricolore de la carte AFFICHEUR
//------------------------------------------
#define LED_ROUGE PF_1
#define LED_VERTE PF_2
#define LED_BLEUE PF_3

void setup()
//-----------------------------------------------------
// Fonction d'initialisation du programme Energia
//-----------------------------------------------------
{
  // Initialisation de la vitesse du port serie de la console à 9600 bauds
  Serial.begin(9600);
  // Appel du la fonction librairie pour initialiser le capteur
  Init_CapteurEcg();

  pinMode(LED_ROUGE, OUTPUT);
  pinMode(LED_VERTE, OUTPUT);
  pinMode(LED_BLEUE, OUTPUT);
}

//-------------------------------------------------------------
// Définir les valeurs minimale et maximale d'une
// fréquence cardiaque raisonnables
//-------------------------------------------------------------
#define VALEUR_ECG_MIN (30)
#define VALEUR_ECG_MAX (160)

void loop()
//-----------------------------------------------------
// Fonction principale du programme Energia
//-----------------------------------------------------
{
  // Déclaration de variables
  short FreqEcg_bpm; // frequence cardiaque en bpm
  short etat_led_rouge, etat_led_vert, etat_led_bleu;

  Serial.println(" ");
  Serial.println("--- Mesure du rythme cardiaque ---");

  // Appeler la fonction
  FreqEcg_bpm = Mesure_FrequenceEcg();

  // afficher la valeur mesurée
  Serial.print("      Fréquence mesurée = ");
  Serial.print(FreqEcg_bpm);

  // Tester la valeur retournée
  if (FreqEcg_bpm <= 0)
  { // puissance trop faible = pas de signal
    Serial.println("   +++ Pas de signal reçu.");
    // allumer la LED en Blanc
    etat_led_rouge = HIGH; // état HIGH ou LOW
    etat_led_vert = HIGH;
    etat_led_bleu = HIGH;
  }
  else if (FreqEcg_bpm >= VALEUR_ECG_MIN && FreqEcg_bpm <= VALEUR_ECG_MAX)
  { // Il y a une valeur acceptable,
    Serial.println("    Signal ECG probable.");
    // allumer la LED en verte
    etat_led_rouge = LOW; // état HIGH ou LOW
    etat_led_vert = HIGH;
    etat_led_bleu = LOW;
  }
  else
  { // Signal avec de la puissance mais pas acceptable
    Serial.println("    Signal pas acceptable.");
    // allumer la LED en Rouge,
    etat_led_rouge = HIGH; // état HIGH ou LOW
    etat_led_vert = LOW;
    etat_led_bleu = LOW;
  }
  Serial.println(" ");

  digitalWrite(LED_ROUGE, etat_led_rouge);
  digitalWrite(LED_VERTE, etat_led_vert);
  digitalWrite(LED_BLEUE, etat_led_bleu);

  // Il faut laisser 2 seconde entre deux lectures du capteur
  delay(2000);
}

//--------------------------------------------------------------------
// Définir ici sur quel port du µC vous avez connecté le capteur ECG
//--------------------------------------------------------------------
// Remplacer dans la ligne ci-dessous ?? par le numéro du Port.
// Cela doit etre PD_1 ou PD_0
#define CAPTEUR_ECG PD_1

//---------------------------------------------------------------
// Définir ici les parametres d'echantillonnage
// Les indications de limites sont dictées par les paramètres de
// la carte TIVA utilisée (memoire = 32 KBytes, Frequence max de
// la fonction 'analogRead' = 10 KHz)
//---------------------------------------------------------------
//  Choisir la frequence d'echantillonnage en Hz
// Valeur raisonnable = 10/20/40 fois (frequence Ecg max)
#define FREQ_ECH (20)

// Definir la duree du signal en secondes
// Valeur raisonnable = temps pour avoir 6/8/10 pics
#define DUREE_SIGNAL (10 * 1 / FREQ_ECH)

// calculer la taille des echantillons en fonction de
// la fréquence FREQ_ECH et de la duree DUREE_SIGNAL
#define SIZE_BUFF (FREQ_ECH * DUREE_SIGNAL)

// Definir la fenetre pour le calcul de la puissance (duree en ms)
// #define   DUREE_DEMI_FEN  ("???")
// Calculer le nombre d'echantillons correspondant K
// #define   LEN_K (DUREE_DEMI_FEN*FREQ_ECH/1000)
// Le choix est un peu "guidé" pour avoir une fenetre de 10 echantillons
#define LEN_K (10)

//  Calcul automatique de la periode d'echantillonnage en millisecondes
//  Voir la fonction millis()
#define TIME_ECH (1000 / FREQ_ECH) /* temps en millisecondes   */

//  Buffer des échantillons (nombres entiers 16 bits)
short Buffer_Ech[SIZE_BUFF];
short Buffer_ecg[SIZE_BUFF];
//  Buffer de la Puissance instantannée du signal
float Pwr_Inst[SIZE_BUFF];
//  Buffer de calcul de l'autocorrelation
float SignCorr[SIZE_BUFF];

//--------------------------------------------------------------------
// Définir expérimentalement la puissance seuil qui indique une absence
// de signal.
//--------------------------------------------------------------------
#define P_MOYEN_SEUIL (40e3)

void Lecture_BufferSignalEcg(void);

int Mesure_FrequenceEcg(void)
//------------------------------------------------------------------------------
// Parametre en entree  : rien
// Parametre en sortie  : la frequence cardiaque en bpm calculée
//      Retourner (-1) s'il n'y a pas de signal (puissance < Pseuil)
// Remarque: il ne sert à rien d'exprimer une frequence cardiaque
// à 1/10eme ou 1/100eme près. Une valeur entière est suffisante.
//------------------------------------------------------------------------------
{ // il faut utiliser les variables définies
  float puiss_moy, puiss_sum, sumfen, pval;
  int val_moy, n, k, FreqBpm;
  int K = LEN_K;
  puiss_sum = 0;

  // ajouter des variables pour votre algo
  int puiss_inst;
  float ac[SIZE_BUFF];
  int peaks[SIZE_BUFF];
  int num_peaks = 0;
  float peak_distance = 0;
  int i, j;
  float periodeBPM;
  int detection[SIZE_BUFF];
  int *idstart = (int *)calloc(SIZE_BUFF, sizeof(int));
  int *idend = (int *)calloc(SIZE_BUFF, sizeof(int));
  int nbbruit = 0;
  int nb = 0;
  int counter = 0;

  // lire un buffer du signal sonore en appelant la fonction ...
  Lecture_BufferSignalEcg();

  // Faire ici votre algorithme de Matlab
  // Calcul de la puissance instantanee puis la puissance moyenne
  for (n = K; n <= SIZE_BUFF - K - 1; n++)
  {
    sumfen = 0;
    for (k = n - LEN_K; k <= n + LEN_K; k++)
    {
      pval = Buffer_Ech[k];
      sumfen += pval * pval;
    }
    puiss_inst = sumfen / (2 * LEN_K + 1);
    puiss_sum += puiss_inst;
    Pwr_Inst[n] = puiss_inst;
  }
  puiss_moy = puiss_sum / (SIZE_BUFF - 2 * k - 1);
  for (int i = 0; i <= SIZE_BUFF - 1; i++)
  {
    Buffer_Ech[i] = Buffer_Ech[i] - val_moy;
  }
  for (n = k; n <= SIZE_BUFF - k - 1; n++)
  {
    if (Pwr_Inst[n] >= puiss_moy)
      ;
    {
      detection[n] = 1;
    }
  }
  for (n = k; n <= SIZE_BUFF - k - 1; n++)
  {
    if (detection[n] == 1)
    {
      nb++;
    }
  }
  int Buffer_ecg[nb];
  for (n = k; n <= SIZE_BUFF - k - 1; n++)
  {
    if (detection[n] == 1)
    {
      Buffer_ecg[counter] = Buffer_Ech[n];
      counter++;
    }
  }
  //--------------------
  // Faire votre algorithme MATLAB pour
  // trouver la frequence cardiaque
  //--------------------

  // Calculer l'autocorrélation
  for (i = 0; i < SIZE_BUFF; i++)
  {
    ac[i] = 0;
    for (j = 0; j < SIZE_BUFF - i; j++)
    {
      ac[i] += Buffer_ecg[j] * Buffer_ecg[j + i];
    }
    
    ac[i] /= SIZE_BUFF;
  }
  // Trouver les pics de l'autocorrélation
  for (i = 1; i < SIZE_BUFF - 1; i++)
  {
    if (ac[i] > ac[i - 1] && ac[i] > ac[i + 1])
    {
      peaks[num_peaks] = i;
      num_peaks++;
    }
  }

  periodeBPM = peaks[1] / FREQ_ECH;
  FreqBpm = 60 / periodeBPM;
  // Retourner la frequence cardiaque
  return FreqBpm;
}

void Lecture_BufferSignalEcg(void)
//------------------------------------------------------------------------------
// Parametre en entree  : rien    // Parametre en sortie  : rien
// Les informations sont dans des variables globales
//------------------------------------------------------------------------------
{ // vous n'avez pas besoin d'ajouter de variables dans cette fonction
  unsigned long curtime, nextime;
  short nb_ech, val_ech;
  nb_ech = 0;
  curtime = micros();
  nextime = TIME_ECH;
  // Voir l'aide en ligne de la fonction 'micros()' de Energia
  // initialiser le prochain temps d'echantillonnage (nextime)
  // faire une boucle qui attend la lecture des 'SIZE_BUFF' echantillons
  while (nb_ech <= SIZE_BUFF)
  {
    curtime = micros();
    // attendre le prochain temps d'echantillonnage en testant si
    if (curtime >= nextime)
    {
      // le temps actuel (curtime) a atteind ce prochain temps (nextime)
      // lire un echantillon et le mettre dans le buffer Buffer_Ech[]
      val_ech = analogRead(CAPTEUR_ECG);
      Buffer_ecg[nb_ech] = val_ech;
      // actualiser le nombre d'échantillons lus (nb_ech)
      nb_ech += 1;
      // calculer le prochain temps d'echantillonnage (nextime)
      nextime += TIME_ECH;
    }
  }
  return;
}

void Init_CapteurEcg(void)
//------------------------------------------------------------------------------
// Parametre en entree : rien   // Parametre en sortie : rien
//------------------------------------------------------------------------------
{ // configure le port en entrée
  pinMode(CAPTEUR_ECG, INPUT);
}
