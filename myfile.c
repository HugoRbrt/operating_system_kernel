#include "myfile.h"

//définition de variable globale
long int NB_TIC=4319600;
long int QUARTZ = 0x1234DD;
int CLOCKFREQ = 50;
int ACTUAL_PID =0;
//def des fonctions défini dans les fichier .s
extern void traitant_IT_32(void);
extern void ctx_sw(int new_ctx,int old_ctx);//paramètre : adresse de nouveau et de l'ancien contexte
//GESTION ECRAN
uint16_t *ptr_mem(uint32_t lig, uint32_t col){
  uint16_t * pointeur=(uint16_t* )(0xB8000+2*(lig*80+col));
  return pointeur;
}
void ecrit_car(uint32_t lig, uint32_t col, char c){
  uint16_t valeur;
  //ecriture du code ascii du caractere
  valeur = c ;
  //fond noir :
  valeur &= 0x0FFF;
  //texte en blanc :
  valeur |= 0x0F00;
  //forcer bit 7 a 0 :
  valeur &= 0x7FFF;
  //ecriture de la valeur à la bonne adresse :
  uint16_t * pointeur = ptr_mem(lig,col);
  *pointeur = valeur;
}
void efface_ecran(void){
  char caract = 0x20; //correspond a espace en ASCII
  for(int l=0;l<25;l++){
    for(int col=0;col<80;col++){
      ecrit_car( l,  col,  caract);
    }
  }
}
void place_curseur(uint32_t lig, uint32_t col){
  uint16_t pos = (uint16_t)col + 80*(uint16_t)lig;
  //envoyer la commande 0x0F sur le port de commande
  outb(0x0F, 0x3D4);
  //envoyer cette partie basse sur le port de données
  outb((uint8_t)pos,0x3D5);
  //envoyer la commande 0x0E sur le port de commande
  outb(0x0E, 0x3D4);
  // envoyer la partie haute de la position sur le port de données
  pos = pos>>8;
  outb((uint8_t)pos,0x3D5);
}
void traite_car(char c){

  uint16_t pos = 0x0000;
  //lecture partie haute
  outb(0x0E, 0x3D4);
  pos = (uint16_t)inb(0x3D5);
  pos = pos<<8;
  //lecture partie basse
  outb(0x0F, 0x3D4);
  pos += (uint16_t)inb(0x3D5);
  uint32_t lig=pos/80;
  uint32_t col=pos%80;


  if((int)c>31 && (int)c<127){  //cas caractere "normal"
    ecrit_car(lig, col, c);
    if(col == 79){col=0;lig+=1;}//cas fin de ligne
    else{col+=1;}
    place_curseur(lig,col);
  }
  if(8==(int)c){
    if(col!=0){col-=1;}
    place_curseur(lig,col);
  }
  if(9==(int)c){
    if(col>=72 && col !=79){col = 79;}
    else if(col ==79){col = 0;if(lig<24){lig+=1;}}
    else {col = 8*(1+(col/8));}
    place_curseur(lig,col);
  }
  if(10==(int)c){
    if(lig<24){lig+=1;col = 0;}
    place_curseur(lig,col);
  }
  if(12==(int)c){
    efface_ecran();
    lig=0;col=0;
    place_curseur(lig,col);
  }
  if(13==(int)c){
    col=0;
    place_curseur(lig,col);
  }
}
void defilement(void){
  memmove((void*)0xB8000,(void *)0xB80A0,2*(80*24));
//efface la derniere ligne
  for(int i=0;i<80;i++){
    ecrit_car( 24,  i, 0x20);
  }
}
void console_putbytes(const char *s, int len){
  for(int i=0;i<len;i+=1){
    traite_car(s[i]);
  }
}
//GESTION TEMPS
void affiche_HAUTdroite(const char *s){
  int l = strlen(s);
  int ligne = 0;
  int col = 80-l;
  place_curseur(ligne,80-strlen("        "));
  console_putbytes("        ",strlen("        "));
  place_curseur(ligne,col);
  console_putbytes(s,l);
}
void calculdate(char* motheur){
  //calcul heure
int heur = NB_TIC/(3600*CLOCKFREQ);
int val = NB_TIC - (3600*CLOCKFREQ*heur);
int min = val/(60*CLOCKFREQ);
val = val-(60*CLOCKFREQ*min);
int sec = val/CLOCKFREQ;
  //ecriture en mode HH:MM:SS
char motsec[10] = "";
char motmin[10] = "";
  //conversion min en char
if(sec<10){strcat(motsec,"0");char motsec2[10] = "";sprintf(motsec2,"%d",sec);strcat(motsec,motsec2);}
else{sprintf(motsec,"%d",sec);}

  //conversion min en char en affichant au format 2 chiffre tjr (si min=7 on affiche 07)
if(min<10){strcat(motmin,"0");char motmin2[10] = "";sprintf(motmin2,"%d",min);strcat(motmin,motmin2);}
else{sprintf(motmin,"%d",min);}
  //conversion heur en char
if(heur<10){strcat(motheur,"0");char motheur2[10] = "";sprintf(motheur2,"%d",heur);strcat(motheur,motheur2);}
else{sprintf(motheur,"%d",heur);}
  //concatenation au format HH:MM:SS
strcat(motheur,":");
strcat(motheur,motmin);
strcat(motheur,":");
strcat(motheur,motsec);
}
void tic_PIT(void){
  outb(0x20,0x20);
  NB_TIC=NB_TIC+1;
    //si jour suivant on efface le debordement a droite
  if(NB_TIC==86400*CLOCKFREQ){NB_TIC=0;place_curseur(0,70);traite_car(0x20);traite_car(0x20);traite_car(0x20);}
  //affichage heure
  char motheur[10] = "";
  calculdate(motheur);
  affiche_HAUTdroite(motheur);
  ordonnance();
}
void init_traitant_IT(int32_t num_IT, void (*traitant)(void)){
  uint32_t * address = (uint32_t *)0x1000;//on se place au debut de la table d'IDT
  address+= (num_IT*2);//on se place au 1er mot de la case num_IT de la table (car on avance de 32bit = 4 octets en 4 octets)
    //ecriture du 1er mot de la case num_IT
  uint32_t mot1 = (uint32_t)traitant;
  mot1 = mot1 & 0x0000ffff;//on garde les 16 bits de poids faible
  uint32_t inter1 = KERNEL_CS << 16;
  mot1 = mot1 | inter1;
  *address = mot1; //on a aisnsi ecrit le mot 16 bits de poids fort, 0x8E00, KERNEL_CS, 16 bits de poids faible
    //on avance au 2eme mot de la case num_IT
  address+=1;
    //ecriture du 2eme mot de la case num_IT
  uint32_t mot2 = (uint32_t)traitant;
  mot2 = mot2 & 0xffff0000;//on garde les 16 bits de poids fort
  uint32_t inter2 = (uint32_t)0x8E00 ;
  mot2 = mot2 | inter2;
  *address = mot2;
}
void regler_freq(){
  outb(0x34,0x43);
  outb((QUARTZ / CLOCKFREQ) % 256, 0x40);
  outb((QUARTZ / CLOCKFREQ)>>8, 0x40);//par defaut renvoie en priorite les bits de poids forts
}
void masque_IRQ(uint32_t num_IRQ, bool masque){//num_IRQ : de 0 a 7 ; masque : 1->masquer IRQ, 0->demasquer
  uint8_t masque_actuel =inb(0x21);
  if(masque){//cas ou il faut mettre 1 bit a 1
    uint8_t modif = 1<<num_IRQ;
    masque_actuel = masque_actuel | modif;
  }
  else{//cas ou il faut mettre un bit a 0
    uint8_t modif2 = ~(1<<num_IRQ);
    masque_actuel = masque_actuel & modif2;
  }
  outb(masque_actuel,0x21);
}
//GESTION PROCESSUS
//nombre processus existant (initialisé à 1 car idle est crée)
int PROC_EXISTANT = 1;
int32_t cree_processus(void (*code)(void), char *nom){
  if(PROC_EXISTANT==NB_PROC){return -1;}//si on essaie de créer plus de processus que le nombre maximal
  TABLE[PROC_EXISTANT].pid = PROC_EXISTANT;
  TABLE[PROC_EXISTANT].state = 0;
  for(int k=0;k<20;k++){
    TABLE[PROC_EXISTANT].name[k]=nom[k];
  }
  int* ptr_sommet_pile = &TABLE[PROC_EXISTANT].pile[sizeof(TABLE[PROC_EXISTANT].pile)/sizeof(TABLE[PROC_EXISTANT].pile[0])-1];
  TABLE[PROC_EXISTANT].sauvegarde[1] = (int)ptr_sommet_pile;//%esp pointe vers le sommet de la pile
  *ptr_sommet_pile = (int)(*code);
  PROC_EXISTANT+=1;
  return TABLE[PROC_EXISTANT-1].pid ;
}
void InitProc(){
  //init idle
  TABLE[0].pid = 0;
  TABLE[0].state = 1;
  char nom[20] = "idle";
  for(int k=0;k<20;k++){
    TABLE[0].name[k]=nom[k];
  }
  //init proc1, proc2, proc3
  int c1 =  cree_processus(&proc1,"proc1");
  if(c1!=1){printf("ERROR%d",c1);}
  int c2=  cree_processus(&proc2,"proc2");
  if(c2!=2){printf("ERROR%d",c2);}
  int c3 = cree_processus(&proc3,"proc3");
  if(c3!=3){printf("ERROR%d",c3);}
}


char *mon_nom(){
  return TABLE[ACTUAL_PID].name;
}

void ordonnance(){
  int Temps_max_dors  =3600;
  for(int k=0;k<NB_PROC;k++){
    if(TABLE[k].state==2 && TABLE[k].heure_reveil<NB_TIC && (NB_TIC/CLOCKFREQ<72000-Temps_max_dors || TABLE[k].heure_reveil/CLOCKFREQ>Temps_max_dors )){
      TABLE[k].heure_reveil=0;
      TABLE[k].state=0;
    }
  }
//on appel ctx_sw
  int old_ctx =(int)TABLE[ACTUAL_PID].sauvegarde;
  //on récupère le processus actif
  //état processus actif mis à ACTIVABLE (0) si il n'est ni endormi ni mort
  if(TABLE[ACTUAL_PID].state!=2 && TABLE[ACTUAL_PID].state!=3){
    TABLE[ACTUAL_PID].state  = 0;
  }
   //chgmt du processus actif
   do{
     ACTUAL_PID++;
     ACTUAL_PID%=NB_PROC;
   }while(TABLE[ACTUAL_PID].state!=0);

  //on récupère le processus suivant
  //état processus suivant mis à ELU (1)
    TABLE[ACTUAL_PID].state=1;
  int new_ctx =(int)TABLE[ACTUAL_PID].sauvegarde;
  ctx_sw(old_ctx,new_ctx);

}

void idle(void){
  /*
    for (int i=0;i<3;i++) {
        printf("[%s] pid = %i\n", mon_nom(), ACTUAL_PID);
        ordonnance();
    }
  */
  while(1){
      place_curseur(0,0);
      char motheur[10] = "";
      calculdate(motheur);
        printf("temps : %s[%s ] pid = %i\n",motheur, mon_nom(), ACTUAL_PID);
        sti();
        hlt();
        cli();
  }
}
void proc1(void) {
  /*
    for (int i=0;i<3;i++) {
        printf("[%s] pid = %i\n", mon_nom(), ACTUAL_PID);
        ordonnance();
    }
  */
  /*
  for (int i=0;i<3;i++) {
        printf("[%s] pid = %i\n", mon_nom(), ACTUAL_PID);
        sti();
        hlt();
        cli();
  }
  */
  char motheur[10] = "";
  for (int i=0;i<2;i++) {
    place_curseur(1,0);
    calculdate(motheur);
      printf("temps : %s[%s] pid = %i\n",motheur, mon_nom(), ACTUAL_PID);
        dors(3);
   }
   char motheur2[10] = "";
   calculdate(motheur2);
   place_curseur(1,0);
   printf("processus mort a %s[%s] pid = %i\n",motheur2, mon_nom(), ACTUAL_PID);
   fin_processus();
}
void proc2(void) {
  /*
    for (int i=0;i<3;i++) {
        printf("[%s] pid = %i\n", mon_nom(), ACTUAL_PID);
        ordonnance();
    }
    */
  /*
    for (int i=0;i<3;i++) {
          printf("[%s] pid = %i\n", mon_nom(), ACTUAL_PID);
          sti();
          hlt();
          cli();
    }*/
    char motheur[10] = "";
    for (int i=0;i<2;i++) {
      place_curseur(2,0);
      calculdate(motheur);
        printf("temps : %s[%s] pid = %i\n",motheur, mon_nom(), ACTUAL_PID);
          dors(5);
    }
    char motheur2[10] = "";
    place_curseur(2,0);
    calculdate(motheur2);
    printf("processus mort a %s[%s] pid = %i\n",motheur2, mon_nom(), ACTUAL_PID);
    fin_processus();
}
void proc3(void) {
  /*
    for (int i=0;i<3;i++) {
        printf("[%s] pid = %i\n", mon_nom(), ACTUAL_PID);
        ordonnance();
    }
    */
    /*
    for (int i=0;i<3;i++) {
          printf("[%s] pid = %i\n", mon_nom(), ACTUAL_PID);
          sti();
          hlt();
          cli();
    }
    */
    char motheur[10] = "";
    for (int i=0;i<2;i++) {
      place_curseur(3,0);
      calculdate(motheur);
        printf("temps : %s[%s] pid = %i\n",motheur, mon_nom(), ACTUAL_PID);
          dors(7);
    }
    char motheur2[10] = "";
    place_curseur(3,0);
    calculdate(motheur2);
    printf("processus mort a %s[%s] pid = %i\n",motheur2, mon_nom(), ACTUAL_PID);
    fin_processus();
}


void dors(uint32_t nbr_secs){
  TABLE[ACTUAL_PID].state=2;//on met le processus à l'état endormi
  TABLE[ACTUAL_PID].heure_reveil=(NB_TIC+nbr_secs*CLOCKFREQ)%(86400*CLOCKFREQ);//cela impose que l'attente soit inferieur a 1jour
  ordonnance();
}
void fin_processus(void){
  TABLE[ACTUAL_PID].state=3;//on met le processus à l'état endormi
  ordonnance();
}
