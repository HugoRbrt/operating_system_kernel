#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cpu.h>
#include <inttypes.h>
#include <stdbool.h>
#include <segment.h>

//GESTION ECRAN
//renvoie un pointeur sur la case mémoire correspondant aux coordonnées fournies
uint16_t *ptr_mem(uint32_t lig, uint32_t col);
//écrit le caractère c aux coordonnées spécifiées (ecriture blanc sur fond noir)
void ecrit_car(uint32_t lig, uint32_t col, char c);
//parcourir les lignes et les colonnes de l'écran pour écrire dans chaque case un espace en blanc sur fond noir
void efface_ecran(void);
//place le curseur à la position donnée
void place_curseur(uint32_t lig, uint32_t col);
// traite un caractère donné
void traite_car(char c);
// fait remonter d'une ligne l'affichage à l'écran
void defilement(void);
// affiche une chaine de caractères à la position courante du curseur
void console_putbytes(const char *s, int len);


//GESTION TEMPS
//affiche s dans le coin haut droite
void affiche_HAUTdroite(const char *s);
//écrire le traitant de l'interruption 32 qui affiche à l'écran le temps écoulé depuis le démarrage du système
void traitant_IT_32(void);
//renvoie dans motheur la date actuelle en hh::mm::ss
void calculdate(char* motheur);
//acquittement de l'interruption et la partie gérant l'affichage
void tic_PIT(void);
//initialiser l'entrée 32 dans la table des vecteurs d'interruptions
void init_traitant_IT(int32_t num_IT, void (*traitant)(void));
//régler la fréquence de l'horloge programmable
void regler_freq(void);
//démasquer l'IRQ0 pour autoriser les signaux en provenance de l'horloge
void masque_IRQ(uint32_t num_IRQ, bool masque);

//GESTION PROCESSUS
typedef struct{
  int pid;//pid du processus
  char name[20];//nom du processus (ne peut pas excéder 20 caractères)
  unsigned int state; //état du processus : 3 si mort, 2 si endormi, 1 si élu, 0 si activable
  int sauvegarde[5];//zone de sauvegarde des registres du processeur[%ebx, %esp, %ebp, %esi, %edi]
  int pile[512];//pile d'exécution du processus
  long int heure_reveil;
} processus;
//définition d'une valeur statique globale du nb de processus géré
#define NB_PROC 4
//définition variable globale
  //table des processus :
processus TABLE[NB_PROC];

//retourne nouveau processus
int32_t cree_processus(void (*code)(void), char *nom);
//créer un nouveau processus est le retourne
void InitProc(void);
//renvoie le nom du processus en cours d'exécution
char *mon_nom(void);
//ordonnanceur de processus
void ordonnance(void);
//processus idle
void idle(void);
//processus proc1
void proc1(void);
//processus proc2
void proc2(void);
//processus proc3
void proc3(void);
//endors le processus pendant nbr_seconds
void dors(uint32_t nbr_secs);
//le processus en cours devient mort
void fin_processus(void);
