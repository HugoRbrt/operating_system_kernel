#include <cpu.h>
#include <string.h>
#include <inttypes.h>
#include "myfile.h"
// on peut s'entrainer a utiliser GDB avec ce code de base
// par exemple afficher les valeurs de x, n et res avec la commande display

// une fonction bien connue
uint32_t fact(uint32_t n){
    uint32_t res;
    if (n <= 1) {
        res = 1;
    } else {
        res = fact(n - 1) * n;
    }
    return res;
}

void kernel_start(void)
{
      //-1< ligne <25
      //-1< col <80
    // uint32_t ligne = 10;
    // uint32_t colonne = 10;
        //TEST ECRAN
    // efface_ecran();
    // place_curseur(ligne,colonne);
    // traite_car('b');
    // traite_car(9);
    // defilement();
    // char * mot = "hello world !";
    // console_putbytes(mot, strlen(mot));
        // quand on saura gerer l'ecran, on pourra afficher x
    // uint32_t x = fact(5);
    // char charx[10]="";
    // sprintf(charx,"%d",x);
        //TEST TEMPS
    regler_freq();
    init_traitant_IT(32, &traitant_IT_32);
    masque_IRQ(0,0);
    //sti();

      //TEST BASE PROCESSUS
    InitProc();
    efface_ecran();
    idle();
    // on ne doit jamais sortir de kernel_start
    while (1) {
        // cette fonction arrete le processeur
        hlt();
    }
}
