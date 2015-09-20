/*
   Send Data with IR.c
   This is the 'with structures' version of 02 Receive and Store IR Strings.side.
*/

#include "simpletools.h"                     // Include simpletools library
#include "badgetools.h"                      // Include badgetools library

#define ST_ZOMBIE 0x0001
#define ST_INFECTED 0x0002
#define ST_HUMAN 0x0004
#define ST_DEAD 0x0008
#define MAX_SCAN_TIME 10
#define MAX_HEALTH_Z 10
#define MAX_HEALTH_H 3
#define FLICKER_TIME 100
#define INFECTION_TIME 300
#define NUM_LEDS 6

int splash[16];
int color, buttonState, attackerState;
char recvBuf[8];

typedef struct player
{
    int state; // zombie/human/infected
    int infectionTimer;
    int health;
} player_t;
player_t you;

void player_set_human() {
    you.state = ST_HUMAN;
    you.health = MAX_HEALTH_H;
}
void player_set_zombie() {
    you.state = ST_ZOMBIE;
    you.health = MAX_HEALTH_Z;
}
void player_set_infected() {
    you.state = ST_INFECTED;
    you.infectionTimer = INFECTION_TIME;
}
void player_set_dead() {
    you.state = ST_DEAD;
    you.health = 0;
}
void player_show() {
    clear();
    text_size(LARGE);
    
    switch(you.state) {
        case ST_INFECTED:
            color = YELLOW;
            oledprint("INFECTED");
            break;
        case ST_HUMAN:
            color = GREEN;        
            oledprint("SURVIVOR");  
            break;
        case ST_DEAD:
            color = WHITE;
            oledprint("D E A D");
            break;
        case ST_ZOMBIE:
        default:
            color = RED;
            cursor(2, 0);            
            oledprint("Z E D");
            text_size(SMALL);
            cursor(0, 7);
            oledprint("<3: ");
            for(int i = 0; i < you.health; i++) {
                oledprint("|");
            }
            break;
    }
}



void util_flicker() {
    rgbs(OFF, OFF);
    pause(FLICKER_TIME);
    rgbs(color, color);
    pause(FLICKER_TIME);
    rgbs(OFF, OFF);
    pause(FLICKER_TIME);
    rgbs(color, color);
}

void board_init() {
    printf("Badge setup...\n");
    pause(1000);
    badge_setup();
    printf("Badge setup\n");
}

/**
 * Init game state
 */
void game_init()
{
    you.state = ST_HUMAN;

    buttonState = 0;
    clear();
    text_size(SMALL);
    oledprint("  CHOOSE A SIDE");
    text_size(LARGE);
    cursor(0, 1);    
    oledprint("<S    Z>");
    while(1) {
        buttonState = buttons();
        if(buttonState == 32) {
            player_set_human();
            break;
        } else if(buttonState == 1) {
            player_set_zombie();
            break;        
        } else if(buttonState) {
            player_set_infected(); // just for testing
            break;
        }
    }
    
    printf("SIDE: %d\n", you.state);
}


void game_draw_splash() {
    clear();
    int i = 0;
    splash[i++] =  0b00000000000000011111111111111000;
    splash[i++] =  0b00011111111111111111111110000100;
    splash[i++] =  0b00100000000000000000000000000010;
    splash[i++] =  0b01000000000000000000000111000010;    
    splash[i++] =  0b01001000000000000000011000000010;
    splash[i++] =  0b01000011100000000001100000000010;    
    splash[i++] =  0b01000000010000010000011111100010;
    splash[i++] =  0b01000111110000010000010000100010;
    splash[i++] =  0b10000011100000010000001111000001;
    splash[i++] =  0b10000000000011111000000000000001;
    splash[i++] =  0b10000000000000000000000000000001;
    splash[i++] =  0b11000001111111111111111100000011;
    splash[i++] =  0b00100110110000000110001011000100;
    splash[i++] =  0b00100100010000000010000001000100;
    splash[i++] =  0b00100100000000100000000001000100;    
    splash[i++] =  0b00100100110000110000111001000100;
    splash[i++] =  0b00100111111111111111111111000100;
    splash[i++] =  0b00010000000000000000000000001000;
    splash[i++] =  0b00001111111111111111111111110000;    

    text_size(SMALL);    
    cursor(0, 0);
    oledprint("    IRZOMBIES");

    printf("Draw splash\n");
    for(int y = 0; y < i; y++) {
        for(int ys = 0; ys < 2; ys++) {
            for(int x = 0; x < sizeof(int) * 8; x++) {
                if((1 << (31 - x)) & splash[y]) {
                    for(int xs = 0; xs < 2; xs++) {
                        point(x*2+xs+33, y*2+ys+14, 1);
                    }
                }
            }
        }
    }

    printf("Wait\n");
    text_size(SMALL);    
    cursor(0, 7);
    oledprint("eucleo.com");
    pause(5000);
}

/**
 * Attack!
 */
void game_attack() {
    printf("STATE: %d\n", you.state);
    irprint("%d\n", you.state);
    util_flicker();
}

/**
 * Main game loop
 */
void game_run() {
    while(1) {

        // dead zombies can't do anything
        if(you.state == ST_DEAD) {
            pause(500);
            continue;
        }

        // run the infection countdown
        if(you.state == ST_INFECTED) {
            if(! --you.infectionTimer) {
                leds(0b000000);
                player_set_zombie();       
                break;
            } else {
                int ledCount = (int)(((INFECTION_TIME - you.infectionTimer) / (1.0d * INFECTION_TIME)) * 6) + 1;
                for(int i = 0; i < ledCount; i++) {
                    led(i, ON);
                }
            }
        }

        irclear();

        // check to see if we need to send an attack
        if(buttons() == 64) {
            game_attack();
        }

        // check to see if we need to receive an attack
        memset(recvBuf, 0, 8);
        irscan("%s", recvBuf);
        if(strlen(recvBuf)) {
            attackerState = atoi(recvBuf);
            switch(you.state) {
                case ST_ZOMBIE:
                    if(attackerState != ST_ZOMBIE) {
                        printf("HIT!!!\n");
                        if(--you.health <= 0) {
                            player_set_dead();
                        }
                        return;
                    }
                    break;
                case ST_INFECTED:
                    if(attackerState == ST_HUMAN) {
                        printf("HEALED!!!\n");                
                        leds(0b000000);
                        player_set_human();
                        return;                 
                    }
                    break;
                case ST_HUMAN:
                    if(attackerState == ST_ZOMBIE) {
                        printf("INFECTED!!!\n");
                        leds(0b000000);
                        player_set_infected();
                        return;
                    }
                    break;
            }
        }
    }
}

/**
 * Main loop
 */
void main()                               
{
    board_init();
    
    game_draw_splash();

    game_init();
    
    while(1) {
        player_show();
                
        rgbs(color, color);
        game_run();
        rgbs(OFF, OFF);
    }
}  


