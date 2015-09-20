/*
   Send Data with IR.c
   This is the 'with structures' version of 02 Receive and Store IR Strings.side.
*/

#include "simpletools.h"                     // Include simpletools library
#include "badgetools.h"                      // Include badgetools library

#define ST_ZOMBIE 0x0000
#define ST_INFECTED 0x0001
#define ST_HUMAN 0x0002
#define MAX_SCAN_TIME 10
#define FLICKER_TIME 100
#define INFECTION_TIME 300
#define NUM_LEDS 6

int scanTime, color, buttonState;
char recvBuf[8];

typedef struct player
{
    char state; // zombie/human/infected
    int infectionTimer;
} player_t;

player_t you;

void flicker() {
    rgbs(OFF, OFF);
    pause(FLICKER_TIME);
    rgbs(color, color);
    pause(FLICKER_TIME);
    rgbs(OFF, OFF);
    pause(FLICKER_TIME);
    rgbs(color, color);
}

void init()
{
    printf("Badge setup...\n");
    badge_setup();
    printf("Badge setup\n");
    you.state = ST_HUMAN; 

    buttonState = 0;
    clear();
    text_size(SMALL);
    oledprint("  CHOOSE A SIDE\n");
    text_size(LARGE);    
    oledprint("<S    Z>\n");    
    while(1) {
        buttonState = buttons();
        if(buttonState == 32) {
            you.state = ST_HUMAN;
            break;
        } else if(buttonState == 1) {
            you.state = ST_ZOMBIE;
            break;        
        } else if(buttonState) {
            you.state = ST_HUMAN | ST_INFECTED;
            you.infectionTimer = INFECTION_TIME;
            break;
        }
    }
    
    printf("SIDE: %d\n", you.state);
}

void showPlayer() {
    clear();
    text_size(LARGE);
    
    if(you.state & ST_INFECTED) {
        color = YELLOW;
        oledprint("INFECTED\n");
    } else if(you.state & ST_HUMAN) {
        color = GREEN;        
        oledprint("SURVIVOR\n");
    } else {
        color = RED;
        oledprint(" Z E D\n");
    }
}

void runZombie() {
    while(1) {
        if(buttons() == 64) {
            printf("STATE: %d\n", you.state);
            irprint("%s\n", you.state);
            flicker();
        }
    }
}

void runHuman() {
    while(you.state & ST_HUMAN) {
        memset(recvBuf, 0, 8);
        irscan("%s", recvBuf);
        if(strlen(recvBuf)) {
            if(recvBuf[0] & ST_INFECTED) {
                // noop
                continue;
            } else if(recvBuf[0] & ST_HUMAN && you.state & ST_INFECTED) {
                leds(0b000000);
                you.state = ST_HUMAN;
                break;
            } else if(recvBuf[0] & ST_ZOMBIE && !(you.state & ST_INFECTED)) {
                you.state &= ST_INFECTED;
                you.infectionTimer = INFECTION_TIME;
                break;
            }
        } else if(you.state & ST_INFECTED) {
            if(! --you.infectionTimer) {
                leds(0b000000);
                you.state = ST_ZOMBIE;          
                break;
            } else {
                int ledCount = (int)(((INFECTION_TIME - you.infectionTimer) / (1.0d * INFECTION_TIME)) * 6) + 1;
                for(int i = 0; i < ledCount; i++) {
                    led(i, ON);
                }
            }
        } else if(buttons() == 64) {
            printf("STATE: %s\n", you.state);
            irprint("%s\n", you.state);
            flicker();
        }
    }
}

void main()                                  // Main function
{
    init();

    while(1) {
        showPlayer();
        
        irclear();
        rgbs(color, color);
        if(you.state & ST_HUMAN) {
            runHuman();
        } else {
            runZombie();
        }
        rgbs(OFF, OFF);
    }
}  


