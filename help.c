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
        return 1 ;
    }
}