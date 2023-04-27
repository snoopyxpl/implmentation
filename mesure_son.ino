//-----------------------------------------------------------------------------
//
// Mesure du niveau sonore (Capteur microphone electret).
//
//  Lire les commentaires pour comprendre le programme. Remplacer les "????" par
//  une valeur ou un code adéquat avant d'executer le programme.
//  Ne pas modifier les lignes qui ne comportent pas de "????"
//-----------------------------------------------------------------------------

// Definition des fonctions de la librairie
//------------------------------------------
void  Init_CaptSon(void);
float Mesure_PuissanceSon(void);


//  LED tricolore de la carte AFFICHEUR
//------------------------------------------
#define   LED_ROUGE   PF_1
#define   LED_VERTE   PF_2
#define   LED_BLEUE   PF_3


void setup()
//-----------------------------------------------------
// Fonction d'initialisation du programme Energia
//-----------------------------------------------------
{
  // Initialisation de la vitesse du port serie de la console à 9600 bauds
  Serial.begin(9600);
  // Appel du la fonction librairie pour initialiser le capteur
  Init_CaptSon();

  pinMode(LED_ROUGE, OUTPUT);
  pinMode(LED_VERTE, OUTPUT);
  pinMode(LED_BLEUE, OUTPUT);
}

//-------------------------------------------------------------
// Définir le seuil de puissance à x% de la puissance
// maximale en fonction des caractéristiques du convertisseur
// A/D de la carte TIVA
//-------------------------------------------------------------
#define   POWER_MAXIMUM   (20e3)
#define   POWER_SEUIL_FORT  (80e3)
#define   POWER_SEUIL_RIEN  (40e3)
void loop()
//-----------------------------------------------------
// Fonction principale du programme Energia
//-----------------------------------------------------
{
  // Déclaration de variables
  float powerSon;   // puissance du Son
  int   etat_led_rouge, etat_led_vert, etat_led_bleu;

  Serial.println(" ");
  Serial.println("--- Mesure du niveau sonore ---");

  // Appeler la fonction Mesure_PuissanceSon() 
  powerSon = Mesure_PuissanceSon();

  Serial.print("      Puissance mesurée = ");
  Serial.print(powerSon);   Serial.print("  /   ");

  // Tester la valeur retournée 
  if (powerSon < POWER_SEUIL_RIEN) {
    // puissance trop faible = pas de signal
    // allumer la LED en Blanc, 
    etat_led_rouge = HIGH;
    etat_led_vert = HIGH;
    etat_led_bleu = HIGH;
    Serial.println(" +++Pas de signal reçu");
  }
  else if (powerSon >= POWER_SEUIL_RIEN) {
    // Il y a un signal de puissance acceptable
    // allumer la LED en Jaune, 
    etat_led_rouge =HIGH;
    etat_led_vert = HIGH;
    etat_led_bleu = LOW;
    Serial.println("   Signal de puissance raisonnable");
  }
  else {  
    // Il y a un signal de puissance trop important
    // allumer la LED en Rouge, 
    Serial.println("   Signal de puissance trop importante");
    etat_led_rouge=HIGH;
    etat_led_bleu = LOW;
    etat_led_vert = LOW;
  }
  Serial.println("  ");

  digitalWrite(LED_ROUGE, etat_led_rouge);
  digitalWrite(LED_VERTE, etat_led_vert);
  digitalWrite(LED_BLEUE, etat_led_bleu);

  // Il faut laisser 2 seconde entre deux lectures du capteur
  delay(2000);
}


//--------------------------------------------------------------------
// Définir ici sur quel port du µC vous avez connecté le capteur son
//--------------------------------------------------------------------
// Remplacer dans la ligne ci-dessous ???? par le numéro du Port. Par Ex PC_7  
#define CAPTEUR_SON   PC_7


//---------------------------------------------------------------
// Définir ici les parametres d'echantillonnage
// Les indications de limites sont dictées par les paramètres de
// la carte TIVA utilisée (memoire = 32 KBytes, Frequence max de
// la fonction 'analogRead' = 10 KHz)
//---------------------------------------------------------------
//  Choisir la frequence d'echantillonnage en KHz (entre  1 et 8)
#define   FREQ_ECH  1

// Definir la duree du signal en secondes (1 à 4) et 
// calculer la taille des echantillons en fonction de la fréquence
#define   DUREE_SIGNAL  1
#define   SIZE_BUFF (DUREE_SIGNAL * FREQ_ECH*1000) /* fonction de DUREE_SIGNAL et FREQ_ECH */

// Definir la fenetre pour le calcul de la puissance (duree en ms < 20)
#define   DUREE_DEMI_FEN  10
// Calculer le nombre d'echantillons correspondant K
#define   LEN_K (DUREE_DEMI_FEN * FREQ_ECH)   /* fonction de DUREE_DEMI_FEN et FREQ_ECH */


//  Calcul automatique de la periode d'echantillonnage en micro-secondes
//  Voir la fonction micros()
#define   TIME_ECH  (1000/FREQ_ECH)
//  Buffer des échantillons (nombres entiers 16 bits)
short Buffer_Ech[SIZE_BUFF];
//  Buffer de la Puissance instantannée du signal
float Pwr_Inst[SIZE_BUFF];
// Buffer de depassement du seuil de la Puissance instantannée
// (Enlever le commentaire pour faire la partie optionnelle)
//char  PwrDepass[SIZE_BUFF];
// Definition de la duree minimale d'un son penible (Pinst > Seuil)
#define   DUREE_PENIBLE   500   /* en ms. 500 = 0.5 sec */
// Nombre d'echantillons correspondant à cette duree
#define   LEN_SON_PENIBLE   (DUREE_PENIBLE*FREQ_ECH)
#define   LEN_SON_FAIBLE    (LEN_SON_PENIBLE/2)


void  Lecture_BufferSon(void);


float Mesure_PuissanceSon(void)
//------------------------------------------------------------------------------
// Parametre en entree  : rien    
// Parametre en sortie  : la puissance moyenne calculée
//------------------------------------------------------------------------------
{ // il faut utiliser les variables définies
      float puiss_moy, puiss_sum, sumfen, pval;
      int   val_moy, n, k;
      // ajouter des variables pour votre algo
      int pCount;
      int puiss_inst;
      puiss_inst =0;
      int K = LEN_K;
      pCount = 0;
      puiss_sum =0;
      // lire un buffer du signal sonore en appelant la fonction Lecture_BufferSon
      Lecture_BufferSon();
      // Faire ici votre algorithme de Matlab
      // Calcul de la puissance instantanee puis la puissance moyenne
      // Remarque: le convertisseur A/D du µC donne un signal
      // entre 0v et Vdd. Donc, il faut recentrer le signal en soustrayant la moyenne a la valeur du buffer
      for(int i=0; i<=SIZE_BUFF-1;i++){
        val_moy += Buffer_Ech[i];
      }
      val_moy = val_moy/SIZE_BUFF;
      //On recentre le signal
      for(int i=0;i<=SIZE_BUFF -1;i++){
        Buffer_Ech[i] = Buffer_Ech[i]-val_moy;
      }
      puiss_sum = 0;
      for(n= K; n<= SIZE_BUFF - K-1;n++){
         
          sumfen =0;
          for(k=n-LEN_K; k<=n+LEN_K; k++) {
              pval = Buffer_Ech[k];
              sumfen += pval * pval;
          }
          puiss_inst= sumfen / (2*LEN_K+1);
          puiss_sum += puiss_inst;
          pCount+=1;
          Pwr_Inst[n] = puiss_inst;
      }
      // Partie optionnelle
      //--------------------
      // Calculer le vecteur de dépassement de la puissance instantanee
      // par rapport au seuil (POWER_SEUIL_FORT) et indiquer par un
      // message si le depassement dure plus de DUREE_PENIBLE
      puiss_moy = puiss_sum/(pCount);
      // Retourner la puissance moyenne
      return puiss_moy;
}

void  Lecture_BufferSon(void)
//------------------------------------------------------------------------------
// Parametre en entree  : rien    // Parametre en sortie  : rien
// Les informations sont dans des variables globales
//------------------------------------------------------------------------------
{ // vous n'avez pas besoin d'ajouter de variables dans cette fonction

  unsigned long curtime, nextime;   short nb_ech, val_ech;
  nb_ech =0;
  curtime = micros();
  nextime = TIME_ECH;
  // Voir l'aide en ligne de la fonction 'micros()' de Energia
  // initialiser le prochain temps d'echantillonnage (nextime)
  // faire une boucle qui attend la lecture des 'SIZE_BUFF' echantillons
  while (nb_ech <= SIZE_BUFF){
    curtime = micros();
    // attendre le prochain temps d'echantillonnage en testant si
    if(curtime >= nextime){
       // le temps actuel (curtime) a atteind ce prochain temps (nextime)
       // lire un echantillon et le mettre dans le buffer Buffer_Ech[]
       val_ech = analogRead(CAPTEUR_SON);
      Buffer_Ech[nb_ech] = val_ech;
      // actualiser le nombre d'échantillons lus (nb_ech)
      nb_ech +=1;
      // calculer le prochain temps d'echantillonnage (nextime)
      nextime += TIME_ECH;
    }
    
  }
  return;
}

void  Init_CaptSon(void)
//------------------------------------------------------------------------------
// Parametre en entree : rien   // Parametre en sortie : rien 
//------------------------------------------------------------------------------
{ // configure le port en entrée
  pinMode(CAPTEUR_SON, INPUT);
}
