#include "terminal.h"
#include "utilities.h"
#include <stdint.h>


int help_main(int argc, char** argv){

    if(argc == 0){
        return 1 ; /* Ne devrait pas arrievr !*/
    }
    else if (argc == 1){
        char* commands[7] = {"bonjour","clear","echo","exit","help","sleep", "time"};
        terminal_writestring("Le terminal a les commandes suivantes : \n");
        for(int i = 0; i<7; i++){
            terminal_writestring(commands[i]);
            terminal_writestring("    ");
        }
        terminal_writestring("\n");
        terminal_writestring("Tapez help command pour plus de precisions sur une commande\n");
        return 0;
    }
    else if (argc==2) {
        if(strcmp(argv[1], "bonjour")){
            terminal_writestring("Ecrit Hello World\n");
            return 0 ;
        }
        if(strcmp(argv[1], "clear")){
            terminal_writestring("Efface l'historique des commandes\n");
            return 0 ;
        }
        if(strcmp(argv[1], "echo")){
            terminal_writestring("Prend 1 argument , ecrit son argument\n");
            return 0 ;
        }
        if(strcmp(argv[1], "exit")){
            terminal_writestring("Tue le terminal\n");
            return 0 ;
        }
        if(strcmp(argv[1], "help")){
            terminal_writestring("Module d'aide \n");
            terminal_writestring("help liste les commandes existantes");
            terminal_writestring("Tapez help command pour plus de precisions sur une commande\n");
            return 0 ;
        }
        if(strcmp(argv[1], "sleep")){
            terminal_writestring("La commande sleep prend un argument entier et attend le nombre de ticks correspondant\n");
            terminal_writestring("Actuellement 18 ticks = 1 seconde\n");
            return 0 ;
            /* TODO : A preciser    */
        }
        if(strcmp(argv[1], "time")){
            terminal_writestring("La commande time renvoie le temps en ticks depuis le demarrage\n");
            terminal_writestring("Actuellement 18 ticks = 1 seconde\n");
            return 0 ;
            /* TODO : A preciser    */
        }


        terminal_writestring("Commande ou option inconnue ou non documentee\n");
        return 1 ;
    }
    else{
        terminal_writestring("too much arguments\n");
        return 1 ;
    }
}