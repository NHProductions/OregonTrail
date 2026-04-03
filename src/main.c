#include <debug.h>
#include <graphx.h>
#include <keypadc.h>
#include "prompt.h"
#include "Tundora.h"
#include "gfx/gfx.h"
#include <ti/getcsc.h>
#include <fileioc.h>
#include <sys/timers.h>
#include <compression.h>
#include <stdlib.h>
#include <time.h>

#define VIEW_DISTANCE 1000
void renderMattsShop();
void sizeUp();
void monthLearnInfoPrompt();
void monthChoiceMade();
void tick();
char* healthToString();
char* weatherToString(int);
int randomInt(int, int);
void shopPrompt(bool isMatts);
typedef enum {
    NPROMPT_START,
    NPROMPT_JOBS,
    NPROMPT_MONTH,
    NPROMPT_MATTSHOP,
    NPROMPT_SIZEUP,
    NPROMPT_PACE,
    NPROMPT_RATIONS,
    NPROMPT_FERRYRIVER,
    NPROMPT_NORMALRIVER,
    NPROMPT_FORTSIZEUP,
    NPROMPT_BUY
} numberPromptIDs;
numberPromptParams npp[] = {
    (numberPromptParams){
        .p = (prompt){"You May: ", {NULL,0}},
        .titleStart = (vector2){37, 78},
        .optionsStart = (vector2){53, 99},
        .choiceDest = (vector2){36, 169},
        .choiceText = "What is your choice?"
    },
    (numberPromptParams){
        .p = (prompt){"You may: ", {NULL,0}},
        .titleStart = (vector2){28,82},
        .optionsStart = (vector2){45, 104},
        .choiceDest = (vector2){27,172},
        .choiceText = "What is your choice?"
    },
    (numberPromptParams){
        .p = (prompt){"", {NULL,0}},
        .titleStart = (vector2){0,0}, // No title
        .optionsStart = (vector2){54,107},
        .choiceDest = (vector2){28,193},
        .choiceText = "What is your choice?"
    },
    (numberPromptParams){
        .p = (prompt){"", {NULL,0} },
        .titleStart = (vector2){0,0},
        .optionsStart = (vector2){81,63},
        .choiceDest = (vector2){87,192},
        .choiceText = "like to buy?"
    },
    (numberPromptParams){
        .p = (prompt){"You may:", {NULL,0}},
        .titleStart = (vector2){5,95},
        .optionsStart = (vector2){29,119},
        .choiceDest = (vector2){5,220},
        .choiceText = "What is your choice?"
    },
    (numberPromptParams){
        .p = (prompt){"Change pace", {NULL,0}},
        .titleStart = (vector2){111,37},
        .optionsStart = (vector2){44,105},
        .choiceDest = (vector2){27,172},
        .choiceText = "What is your choice?"
    },
    (numberPromptParams){
        .p = (prompt){"Change rations", {NULL,0}},
        .titleStart = (vector2){111,37},
        .optionsStart = (vector2){44,105},
        .choiceDest = (vector2){27,172},
        .choiceText = "What is your choice?"
    },
    (numberPromptParams){
        .p = (prompt){"You may:", {NULL,0}},
        .titleStart = (vector2){3, 94+8},
        .optionsStart = (vector2){3, 94+16},
        .choiceDest = (vector2){1,181},
        .choiceText = "What is your choice?"
    },
    (numberPromptParams){
        .p = (prompt){"You may:", {NULL,0}},
        .titleStart = (vector2){3, 94+8},
        .optionsStart = (vector2){3, 94+16},
        .choiceDest = (vector2){1,173},
        .choiceText = "What is your choice?"
    },
    (numberPromptParams){
        .p = (prompt){"You may:", {NULL,0}},
        .titleStart = (vector2){5,95},
        .optionsStart = (vector2){29,119},
        .choiceDest = (vector2){5,220},
        .choiceText = "What is your choice?"
    },
    (numberPromptParams){
        .p = (prompt){"You may buy:", {NULL,0} },
        .titleStart = (vector2){5,95},
        .optionsStart = (vector2){29,119},
        .choiceDest = (vector2){5,228},
        .choiceText = "Which number? "
    },
};
typedef enum {
    TPROMPT_LEARN,
    TPROMPT_JOBINFO,
    TPROMPT_MONTHINFO,
    TPROMPT_SHOPSTART,
} textPromptIDs;
textPromptParams tpp[] = {
    (textPromptParams){
        .txtPrompts = NULL, 
        .startPos = (vector2){34,82}, 
        .charsPerLine = 30, 
        .spaceToContinueDisclaimer = true 
    },
    (textPromptParams){
        .txtPrompts = NULL,
        .startPos = (vector2){27,48},
        .charsPerLine = 30,
        .spaceToContinueDisclaimer = true
    },
    (textPromptParams){
        .txtPrompts = NULL,
        .startPos = (vector2){28, 42},
        .charsPerLine = 32,
        .spaceToContinueDisclaimer = false
    },
    (textPromptParams){
        .txtPrompts = NULL,
        .startPos = (vector2){28,42},
        .charsPerLine = 32,
        .spaceToContinueDisclaimer = false
    }
};
typedef enum {
    JOB_BANKER,
    JOB_CARPENTER,
    JOB_FARMER
} jobs;
typedef enum {
    NONE,
    CHOLERA,
    DYSENTERY,
    MEASLES,
    FEVER,
    EXHAUSTION,
    TYPHOID,
    BROKENLIMB,
    PLAGUE,
    INSANITY,
    HYSTERIA
} diseases;
typedef struct {
    char* name;
    int health; // if diseased, pick a random disease name.
    int severity;
    bool announcedDead;
} partyMember;
typedef enum {
    COLD,
    TEMPERATE,
    HOT,
    RAINY
} weather;
char* weatherToString(int w) {
    switch (w) {
        case RAINY: return "Rainy";
        case HOT: return "Hot";
        case COLD: return "Cold";
        case TEMPERATE: return "Temperate";
    }
    return "N/A";
}
typedef enum {
    STEADY=10,
    STRENUOUS=13,
    GRUELING=20
} pace;
typedef enum {
    FILLING,
    MEAGER,
    BAREBONES
} rations;
typedef struct {
    int job;
    double money;
    int wheels;
    int axles;
    int tongues;
    int ammunitions;
    int clothing;
    int food;
    int oxen;
    partyMember members[5];
    int day;
    int year;
    int startingMonth;
    rations currentRations;
    pace currentPace;
    weather currentWeather;
    int progress;
    /*
    January - Day 0 (0) / 31
    February - Day 31 / 29
    March - Day 60 (0) / 31
    April - Day 91 (1) / 30
    May - Day 121 (2) / 31
    June - Day 152 (3) / 30
    July - Day 182 (4) / 31
    August - Day 213 / 31
    September - Day 244 / 30
    October - Day 274 / 31
    November - Day 305 / 30
    December - Day 334 / 31
    // 365 total
    */
} gameState;
gameState globalGameState = (gameState){
    .job = 0,
    .money = 0,
    .wheels = 0,
    .axles = 0,
    .tongues = 0,
    .ammunitions = 0,
    .clothing = 0,
    .food = 0,
    .oxen = 0,
    .day = 0,
    .currentRations = MEAGER,
    .currentPace = STEADY,
    .currentWeather = COLD,
    .progress = 0
};
typedef enum  {
    WAGON1,
    WAGON2,
    INDEPENDENCEMISSOURI1,
    INDEPENDENCEMISSOURI2,
    KANSAS1,
    KANSAS2,
    CHIMNEY1,
    CHIMNEY2,
    KEARNEY1,
    KEARNEY2,
    FLAIR1,
    FLAIR2,
    TITLE1,
    TITLE2,
    MATT,
    BOISE1,
    BOISE2,
    BRIDGER1,
    BRIDGER2,
    GREENRIVER1,
    GREENRIVER2,
    HALL1,
    HALL2,
    INDEPROCK1,
    INDEPROCK2,
    LARAMIE1,
    LARAMIE2,
    OREGON1,
    OREGON2,
    SNAKE1,
    SNAKE2,
    SODA1,
    SODA2,
    SPASS1,
    SPASS2,
    WALLA1,
    WALLA2,
    BLUE1,
    BLUE2,
    GRAVE
} fullScreenSprites;
void fsSprite(fullScreenSprites fs, vector2 pos) {
    gfx_SetPalette(global_palette, sizeof_global_palette, 0);
    char* toOpen = "";
    uint8_t spriteBuffer[48000];
    switch (fs) {
        case WAGON1: {toOpen = "WAGON1";break;}
        case WAGON2: {toOpen = "WAGON2"; break;}
        case INDEPENDENCEMISSOURI1: {toOpen = "STOWN1"; break;};
        case INDEPENDENCEMISSOURI2: {toOpen = "STOWN2";  break;}
        case KANSAS1: {toOpen = "KANSAS1"; break;}
        case KANSAS2: {toOpen = "KANSAS2";  break;}
        case KEARNEY2: {toOpen = "KEARNEY2";break;}
        case KEARNEY1: {toOpen = "KEARNEY1";break;}
        case CHIMNEY1: {toOpen = "CHIMNEY1";break;}
        case CHIMNEY2: {toOpen = "CHIMNEY2"; break;}
        case MATT: {toOpen = "MATT";break;}
        case FLAIR1: {toOpen="FLAIR1"; break;}
        case FLAIR2: {toOpen="FLAIR2";break;}
        case TITLE1: {toOpen="TITLE1"; break;}
        case TITLE2: {toOpen="TITLE2"; break;}
        case BOISE1: {toOpen="BOISE1"; break;}
        case BOISE2: {toOpen="BOISE2"; break;}
        case BRIDGER1: {toOpen="BRIDGER1"; break;}
        case BRIDGER2: {toOpen="BRIDGER2"; break;}
        case GREENRIVER1: {toOpen="GRIV1"; break;}
        case GREENRIVER2: {toOpen="GRIV2"; break;}
        case HALL1: {toOpen="HALL1"; break;}
        case HALL2: {toOpen="HALL2"; break;}
        case INDEPROCK1: {toOpen="INDEROK1"; break;}
        case INDEPROCK2: {toOpen="INDEROK2"; break;}
        case LARAMIE1: {toOpen="LARAMIE1"; break;}
        case LARAMIE2: {toOpen="LARAMIE2"; break;}
        case OREGON1: {toOpen="OREGON1"; break;}
        case OREGON2: {toOpen="OREGON2"; break;}
        case SNAKE1: {toOpen="SNAKE1"; break;}
        case SNAKE2: {toOpen="SNAKE2"; break;}
        case SPASS1: {toOpen="SPASS1"; break;}
        case SPASS2: {toOpen="SPASS2"; break;}
        case SODA1: {toOpen="SODA1"; break;}
        case SODA2: {toOpen="SODA2"; break;}
        case WALLA1: {toOpen="WALLA1"; break;}
        case WALLA2: {toOpen="WALLA2"; break;}
        case BLUE1: {toOpen="BLUE1"; break;}
        case BLUE2: {toOpen="BLUE2"; break;}
        case GRAVE: {toOpen="GRAVE"; break;}
    }
    dbg_printf("\nAttempting to open: %s\n", toOpen);
    dbg_printf("Malloc'd!");
    ti_var_t slot = ti_Open(toOpen, "r");
    if (slot) {
        dbg_printf("Opened!\n");
        void *compressedSpritePtr = (uint8_t*)ti_GetDataPtr(slot);
        dbg_printf("Sprite pointer found!\n");
        zx0_Decompress(spriteBuffer, compressedSpritePtr);
        dbg_printf("Decompressed!\n");
        gfx_RLETSprite((gfx_rletsprite_t*)spriteBuffer, pos.x, pos.y);
        dbg_printf("Sprited!\n");
    }
    dbg_printf("Freed!\n");
    ti_Close(slot);
    dbg_printf("Closed!");
}
char* formatDate() {
    char* month = 0;
    int day = globalGameState.day;
    if (day < 31) {
        month = strdup("January %d, %d");
    }
    else if (day < 60) {
        month = strdup("February %d, %d");
        day -= 31;
    }
    else if (day < 91) {
        month = strdup("March %d, %d");
        day -= 60;
    }
    else if (day < 121) {
        month = strdup("April %d, %d");
        day -= 91;
    }
    else if (day < 152) {
        month = strdup("May %d, %d");
        day -= 121;
    }
    else if (day < 182) {
        month = strdup("June %d, %d");
        day -= 152;
    }
    else if (day < 213) {
        month = strdup("July %d, %d");
        day -= 182;
    }
    else if (day < 244) {
        month = strdup("August %d, %d");
        day -= 213;
    }
    else if (day < 274) {
        month = strdup("September %d, %d");
        day -= 244;
    }
    else if (day < 305) {
        month = strdup("October %d, %d");
        day -= 274;
    }
    else if (day < 344) {
        month = strdup("November %d, %d");
        day -= 305;
    }
    else {
        month = strdup("December, %d, %d");
        day -= 344;
    }
    char* toReturn = malloc(50);
    sprintf(toReturn, month, day+1, globalGameState.year);
    free(month);
    return toReturn;
}
int getWeatherForDay() {
    int roll = randomInt(1,100);
    if (globalGameState.day < 50 || globalGameState.day > 300) {
        if (roll < 10) return HOT;
        if (roll < 20) return TEMPERATE;
        if (roll < 40) return RAINY;
        return COLD;
    }
    if (globalGameState.day < 90) {
        if (roll < 50) return RAINY;
        if (roll < 60) return COLD;
        if (roll < 90) return TEMPERATE;
        return HOT;
    }
    if (globalGameState.day < 260) {
        if (roll < 50) return HOT;
        if (roll < 80) return TEMPERATE;
        return RAINY;
    }
    if (globalGameState.day > 260) {
        if (roll < 50) return TEMPERATE;
        if (roll < 80) return RAINY;
        return COLD;
    }
    dbg_printf("Invalid day %d", globalGameState.day);
    return HOT;
}
int travelModifier = 1;
int travelModifierLength = 1000;
char* doEvent() {
    int roll = randomInt(0,100);
    dbg_printf("\nEvent roll: %d", roll);
    if (roll < 6) {
        int brokenPart = randomInt(0,2);
        if (brokenPart == 0 && (randomInt(0,4) == 0 || (globalGameState.job == JOB_CARPENTER && randomInt(0,1) == 0) ) ) {
            return strdup("One of your wheels broke, but you were able to repair it.");
        }
        else if (brokenPart == 0 && globalGameState.wheels > 0) {
            globalGameState.wheels--;
            return strdup("One of your wheels broke, but you had a replacement.");
        }
        else if (brokenPart == 0) {
            return strdup("One of your wheels broke. You didn't have a replacement. Lose 10 days.");
            travelModifier = 0;
            travelModifierLength = 10;
        }
        else if (brokenPart == 1 && (randomInt(0,4) == 0 || (globalGameState.job == JOB_CARPENTER && randomInt(0,1) == 0) ) ) {
            return strdup("One of your tongues broke, but you were able to repair it.");
        }
        else if (brokenPart == 1 && globalGameState.tongues > 0) {
            globalGameState.tongues--;
            return strdup("One of your tongues broke, but you had a replacement.");
        }
        else if (brokenPart == 1) {
            travelModifier = 0;
            travelModifierLength = 10;
            return strdup("One of your tongues broke. You didn't have a replacement. Lose 10 days.");
            
        }
        else if (brokenPart == 2 && (randomInt(0,4) == 0 || (globalGameState.job == JOB_CARPENTER && randomInt(0,1) == 0) )) {
            return strdup("One of your axles broke, but you were able to repair it.");
        }
        else if (brokenPart == 2 && globalGameState.axles > 0) {
            globalGameState.axles--;
            return strdup("One of your axles broke, but you had a replacement.");
        }
        else if (brokenPart == 2) {
            return strdup("One of your axles broke. You didn't have a replacement. Lose 10 days.");
            travelModifier = 0;
            travelModifierLength = 10;
        }
    }
    else if (roll < 11) {globalGameState.oxen -= 1; travelModifier=0; travelModifierLength=3; return strdup("One of your oxen were injured. Lose 3 days");}
    else if (roll < 14) {globalGameState.oxen -= 1; travelModifier=0; travelModifierLength=3;  return strdup("One of your oxen got lost. Lose 3 days.");}
    else if (roll < 18) {travelModifier=0;travelModifierLength=3; return strdup("Bad weather. Lose 3 days.");}
    else if (roll < 22) {travelModifier=0;travelModifierLength=1; return strdup("Heavy rain. Lose 1 day");}
    else if (roll < 26) {
        int stolenThing = randomInt(0,100);
        if (stolenThing < 5) {
            int randomMember = randomInt(0,4);
            while (globalGameState.members[randomMember].health == 0) {
                randomMember = randomInt(0,4);
            }
            if (globalGameState.money > 50) {
                char* toReturn = malloc(100);
                sprintf(toReturn, "Bandits kidnapped %s. You paid $50 in ransom.", globalGameState.members[randomMember].name);
                globalGameState.money -= 50;
                return toReturn;
            }
            else {
                globalGameState.members[randomMember].health = 0;
                char* toReturn = malloc(100);
                sprintf(toReturn, "Bandits kidnapped %s. You couldn't pay the ransom.", globalGameState.members[randomMember].name);
                return toReturn;
            }
        }
        if (globalGameState.axles > 0) {globalGameState.axles--; return strdup("Bandits steal an axle.");};
        if (globalGameState.ammunitions > 50) {globalGameState.ammunitions -= 50; return strdup("Bandits steal a box of ammo.");};
        if (globalGameState.clothing > 0) {globalGameState.clothing--; return strdup("Bandits steal some clothes");};
        if (globalGameState.food > 200) {globalGameState.food -= 200; return strdup("Bandits steal 200lb of food.");};
        if (globalGameState.oxen > 0) {globalGameState.oxen--; return strdup("Bandits steal an ox.");};
        if (globalGameState.tongues > 0) {globalGameState.tongues--; return strdup("Bandits steal a tongue.");};
        if (globalGameState.wheels > 0) {globalGameState.wheels--; return strdup("Bandits steal a wheel.");};
    }
    else if (roll < 30) {
        int burnedThing = randomInt(0,100);
        if (burnedThing < 5) {
            int randomMember = randomInt(0,4);
            while (globalGameState.members[randomMember].health == 0) {
                randomMember = randomInt(0,4);
            }
            globalGameState.members[randomMember].health = 0;
            char* toReturn = malloc(75);
            sprintf(toReturn, "A fire occurs. %s died.", globalGameState.members[randomMember].name);
            return toReturn;
        };
        if (globalGameState.clothing > 0) {globalGameState.clothing--; return strdup("A fire occurs. You lost a some clothes.");};
        if (globalGameState.food > 0) {globalGameState.food = 0; return strdup("A fire occurs. All of your food is burned.");};
        return strdup("A fire occurs. Nothing is lost.");
    }
    else if (roll < 34) {
        if (globalGameState.food > 0) {
            globalGameState.food = globalGameState.food/2;
        }
        return strdup("Half of your food spoils.");
    }
    else if (roll < 38) {travelModifier=0;travelModifierLength=3; return strdup("You get lost on the trail. Lose 3 days.");}
    else if (roll < 42) {globalGameState.food += 50; return strdup("You find 50lb of food in a wagon.");}
    else if (roll < 46) {
        char* toReturn = malloc(75);
        int victim = randomInt(0,4);
        while (globalGameState.members[victim].health == 0) {victim = randomInt(0,4);}
        globalGameState.members[victim].health -= 10;
        if (globalGameState.members[victim].health < 0) {
            globalGameState.members[victim].health = 0;
            sprintf(toReturn, "%s was bitten by a snake. They died.", globalGameState.members[victim].name);
        }
        else {
            sprintf(toReturn, "%s was bitten by a snake. They died.", globalGameState.members[victim].name);
        }
        return toReturn;
    }
    else if (roll < 50) {travelModifier=0; travelModifierLength=10; return strdup("Your wagon is damaged. Lose 10 days.");}
    else if (roll < 60) {
        int victim = randomInt(0,4);
        while (globalGameState.members[victim].health == 0) {
            victim = randomInt(0,4);
        }
        int illness = randomInt(1,5);
        char* toReturn = malloc(75);
        switch (illness) {
            case FEVER: {
                sprintf(toReturn, "%s caught a fever.", globalGameState.members[victim].name);
                break;
            }
            case MEASLES: {
                sprintf(toReturn, "%s caught measles", globalGameState.members[victim].name);
                break;
            }
            case DYSENTERY: {
                sprintf(toReturn, "%s caught dysentery.", globalGameState.members[victim].name);
                break;
            }
            case CHOLERA: {
                sprintf(toReturn, "%s caught cholera.", globalGameState.members[victim].name);
            }
        }
        globalGameState.members[victim].severity = randomInt(1,10);
        return toReturn;
    }
    else if (roll < 70) {return strdup("^^^^");}
    else if (roll < 80) {
        int res = randomInt(0,10);
        if (res <= 8) {
            return strdup("Indians pass by. Nothing happens.");
        }
        if (res == 9) {
            globalGameState.food += 10;
            return strdup("Indians give you some food (10lb)");
        }
        if (res == 10) {
            if (globalGameState.food > 10) globalGameState.food -= 10;
            if (globalGameState.ammunitions > 10) globalGameState.ammunitions -= 10;
            return strdup("Indians steal some of your stuff.");
        }
    }
    else if (roll < 90) {travelModifier=0; travelModifierLength=3; return strdup("Blizzard; lose 5 days.");}
    else {
        int victim = randomInt(0,4);
        while (globalGameState.members[victim].health == 0) {
            victim = randomInt(0,4);
        }
        int illness = randomInt(EXHAUSTION,HYSTERIA);
        char* toReturn = malloc(75);
        switch (illness) {
            case EXHAUSTION: {
                sprintf(toReturn, "%s is exhausted.", globalGameState.members[victim].name);
                break;
            }
            case INSANITY: {
                sprintf(toReturn, "%s goes insane.", globalGameState.members[victim].name);
                break;
            }
            case TYPHOID: {
                sprintf(toReturn, "%s caught typhoid.", globalGameState.members[victim].name);
                break;
            }
            case BROKENLIMB: {
                sprintf(toReturn, "%s broke their leg.", globalGameState.members[victim].name);
                break;
            }
            case PLAGUE: {
                sprintf(toReturn, "%s caught the plague?", globalGameState.members[victim].name);
                break;
            }
            case HYSTERIA: {
                sprintf(toReturn, "%s caught hysteria.", globalGameState.members[victim].name);
                break;
            }

        }
        globalGameState.members[victim].severity = randomInt(5,12);
        return toReturn;
    }
    return strdup("^^^^");
}
typedef enum {
    KANSASRIVER=75,
    BLUERIVER=125,
    KEARNEY=200,
    CHIMNEY=250,
    LARAMIE=500,
    INDEPENDENCEROCK=750,
    SOUTHPASS=1000,
    BRIDGER=1250,
    SODA=1500,
    HALL=1750,
    SNAKERIVER=2000,
    BOISE=2250,
    BLUEMOUNTAINS=2500,
    WALLAWALLA=2750,
    WILLAMETTE=3000
} landmarks;
typedef enum {
    RIVER,
    FORT,
    ROCK
} landmarkTypes;
typedef struct {
    int mileNum;
    int index;
    landmarkTypes type;
    fullScreenSprites img1;
    fullScreenSprites img2;
} landmarkTuple;
char* landmarkNameFromIndex(int i) {
    switch (i) {
        case 0: {return "Kansas River";};
        case 1: {return "Blue River";};
        case 2: {return "Fort Kearney";};
        case 3: {return "Chimney Rock";};
        case 4: {return "Fort Laramie";};
        case 5: {return "Independence Rock";};
        case 6: {return "South Pass";};
        case 7: {return "Fort Bridger";};
        case 8: {return "Soda River";};
        case 9: {return "Fort Hall";};
        case 10: {return "Snake River";};
        case 11: {return "Fort Boise";};
        case 12: {return "Blue Mountains";};
        case 13: {return "Fort Walla Walla";};
        case 14: {return "Fort Willamette";};
        case 15: {return "Oregon City";};
        default: {return "";}
    }
}
landmarkTuple getNextLandmark() {
    int progress = globalGameState.progress;
    if (progress < KANSASRIVER) return (landmarkTuple){KANSASRIVER,0,RIVER, KANSAS1, KANSAS2};
    else if (progress < BLUERIVER) return (landmarkTuple){BLUERIVER,1,RIVER, BLUE1, BLUE2};
    else if (progress < KEARNEY) return (landmarkTuple){KEARNEY,2,FORT,KEARNEY1,KEARNEY2};
    else if (progress < CHIMNEY) return (landmarkTuple){CHIMNEY,3,ROCK,CHIMNEY1,CHIMNEY2};
    else if (progress < LARAMIE) return (landmarkTuple){LARAMIE,4,FORT,LARAMIE1,LARAMIE2};
    else if (progress < INDEPENDENCEROCK) return (landmarkTuple){INDEPENDENCEROCK,5,ROCK,INDEPROCK1,INDEPROCK2};
    else if (progress < SOUTHPASS) return (landmarkTuple){SOUTHPASS,6,FORT,SPASS1,SPASS2};
    else if (progress < BRIDGER) return (landmarkTuple){BRIDGER,7,FORT,BRIDGER1,BRIDGER2};
    else if (progress < SODA) return (landmarkTuple){SODA,8,RIVER,SODA1,SODA2};
    else if (progress < HALL) return (landmarkTuple){HALL,9,FORT,HALL1,HALL2};
    else if (progress < SNAKERIVER) return (landmarkTuple){SNAKERIVER,10,RIVER,SNAKE1,SNAKE2};
    else if (progress < BOISE) return (landmarkTuple){BOISE,11,FORT,BOISE1,BOISE2};
    else if (progress < WALLAWALLA) return (landmarkTuple){WALLAWALLA,13,FORT,WALLA1,WALLA2};
    else if (progress < WILLAMETTE) return (landmarkTuple){WILLAMETTE,14,FORT,OREGON1,OREGON2};
    return (landmarkTuple){KANSASRIVER, 0, RIVER, KANSAS1, KANSAS2};
}
landmarkTuple nextLandmark = (landmarkTuple){KANSASRIVER, 0, RIVER, KANSAS1, KANSAS2};
typedef struct {
    int width;
    double depth;
    double ferryCost;
    bool isFerryable;
} riverStats;
double randomDouble(double min, double max) {
    double scale = (double)rand() / (RAND_MAX + 1.0);
    return min + scale * (max - min);
}
riverStats getRandomRiverStats() {
    bool isFerryable = nextLandmark.index == 0 || nextLandmark.index == 1 || nextLandmark.index == 8 || nextLandmark.index == 10;
    return (riverStats){randomInt(0,250*2), randomDouble(1, 5), randomInt(5, 100), isFerryable};
}
void rollRandomLoss(vector2 line) {
    int chosenItem = randomInt(0,8);
    int* address = NULL;
    char* name="";
    char* unit="";
    switch (chosenItem) {
        case 0: {address = &(globalGameState.ammunitions); name="bullets";; break;}
        case 1: {address = &(globalGameState.axles); name="axles"; unit=""; break;}
        case 2: {address = &(globalGameState.clothing); name="pairs of clothing";; break;}
        case 3: {address = &(globalGameState.food); name="lb of food";; break;}
        case 4: {address = &(globalGameState.oxen); name="oxen"; break;}
        case 5: {address = &(globalGameState.tongues); name="tongues"; break;}
        case 6: {address = &(globalGameState.wheels); name="wheels"; break;}
        default: {break;}
    }
    if (address) {
        char toPrint[100];
        int amtLost = (double)(*address)*(double)(randomInt(0,50));
        if (amtLost > *address) {
            amtLost = *address;
        }
        *address = *address - amtLost;
        sprintf(toPrint, "You lost %d %s%s.", amtLost, unit, name);
        gfx_PrintStringXY(toPrint, line.x, line.y);
        return;
    }
    if (chosenItem == 7 || chosenItem == 8) {
        int preference = randomInt(0,5);
        partyMember victim;
        victim = (partyMember){
            .announcedDead = false,
            .health = 100,
            .name = "Error",
            .severity = 0
        };
        if (globalGameState.members[preference].health >= 0) {victim = globalGameState.members[preference];}
        else {
            for (int i = 0; i < 5; i++) {
                if (globalGameState.members[i].health >= 0) {
                    victim = globalGameState.members[i];
                    break;
                }
            }
            return;
        }
        victim.health -= 15;
        dbg_printf("%s was injured and/or drowned.", victim.name);
        if (victim.health < 0) {
            char buffer[50];
            sprintf(buffer, "%s drowned.", victim.name);
            victim.announcedDead = true;
            gfx_PrintStringXY(buffer, line.x, line.y);
        }
        else {
            char buffer[50];
            sprintf(buffer, "%s was injured.", victim.name);
            victim.severity += 3;
            gfx_PrintStringXY(buffer, line.x, line.y);
        }
    }

}
void deathScreen() {
    gfx_SetColor(5);
    gfx_FillScreen(2);
    gfx_FillRectangle(0,117,320,98);
    fsSprite(GRAVE,(vector2){94,29});
    gfx_SetTextFGColor(2);
    gfx_PrintStringXY("Here lies", 152, 84);
    char* leaderName = globalGameState.members[0].name;
    gfx_PrintStringXY(leaderName, 184-(4*strlen(leaderName)), 99);
    while (os_GetCSC() != sk_Enter) {}
    gfx_SetTextFGColor(3);
    bool epitaph = newBoolPrompt("Would you like to write an epitaph?", (vector2){1,219});
    gfx_SetColor(2);
    gfx_FillRectangle(1,219,320,13);
    if (epitaph) {
        gfx_PrintStringXY("What would you like on the tombstone?", 1,219);
        char* epi = newUserTextPrompt("",(vector2){1,235},32,NULL, true);
        gfx_SetTextFGColor(2);
        List* epiLines = splitStrLines(epi,16);
        for (int i = 0; i < epiLines->length; i++) {
            char* str = (char*)List_GetElement(epiLines,i)->data;
            gfx_PrintStringXY(str, 184-(4*strlen(str)), 108+(i*8));
        }
        free(epi);
    }
    gfx_SetTextFGColor(3);
    gfx_SetColor(2);
    gfx_FillRectangle(1,227,320,13);
    gfx_PrintStringXY("All of the people in your party have died.", 1, 219);
    while (os_GetCSC() != sk_Enter) {}
    exit(0);
};
void winScreen() {
    gfx_SetColor(2);
    gfx_FillRectangle(26,86,268,55);
    gfx_SetColor(3);
    gfx_Rectangle(26,86,268,55);
    gfx_Rectangle(27,87,267,54);
    gfx_Rectangle(28,88,266,53);
    gfx_PrintStringXY("Congratulations! You have", 29, 89);
    gfx_PrintStringXY("made it to Oregon! Let's see how many", 29, 97);
    gfx_PrintStringXY("points you have received!", 29, 105);
    while (os_GetCSC() != sk_Enter) {};
    gfx_FillScreen(2);
    gfx_PrintStringXY("However, I'm too lazy to make a scoring system. So, you get 15,000 score anyways", 0, 0);
    while (os_GetCSC() != sk_Enter) {};
    exit(0);
}
bool isFortScreen = false;
void tick() {
    gfx_FillScreen(2);
    char* health = healthToString();
    int chance = 2;
    if (globalGameState.food < 20) chance += 5;
    if (strcmp(health,"Struggling") == 0 ) chance += 10;
    if (strcmp(health,"Poor") == 0) chance += 15;
    if (strcmp(health, "Absolutely Cooked") == 0) chance += 25;
    if (globalGameState.currentPace == GRUELING) chance += 10;
    char* event = "";
    if (randomInt(0, 100) < chance && (nextLandmark.mileNum - globalGameState.progress > 20) ) {
        dbg_printf("Event happened w/ %d percent chance.", chance);
        event = doEvent();
    }
    globalGameState.day++;
    if (globalGameState.day > 365) {
        globalGameState.day = 0;
        globalGameState.year++;
    }
    globalGameState.progress += globalGameState.currentPace * travelModifier;
    if (travelModifierLength >= 0) travelModifierLength--;
    if (travelModifierLength == 0) {
        travelModifier = 1;
        travelModifierLength = 1000;
    }
    if (strcmp(globalGameState.members[0].name,"SUPERMAN") == 0) {
        globalGameState.members[0].severity = 0;
        globalGameState.members[0].health = 1000;
    }
    int aliveAmt = 0;
    bool toAnnounceDead[5] = {false, false, false, false, false};
    bool someoneDied = false;
    for (int i = 0; i < 5; i++) {
        if (globalGameState.members[i].health > 0 || !globalGameState.members[i].announcedDead) {
            if (globalGameState.members[i].severity > 0) {
                globalGameState.members[i].health -= globalGameState.members[i].severity;
                globalGameState.members[i].severity--;
            }
            if (globalGameState.food <= 0) {
                globalGameState.members[i].health -= randomInt(0,3);
            }
            if (globalGameState.clothing <= 0) {
                globalGameState.members[i].health -= randomInt(0,5);
            }
            if (globalGameState.day > 250 && randomInt(0,2) == 0) {
                globalGameState.members[i].health -= randomInt(0,5);
            }
            if (globalGameState.members[i].health <= 0 && !globalGameState.members[i].announcedDead) {
                someoneDied = true;
                toAnnounceDead[i] = true;
                globalGameState.members[i].announcedDead = true;
            }
            aliveAmt++;
        }
    }
    if (globalGameState.food > 0) {
        int consumption = globalGameState.currentRations*aliveAmt;
        globalGameState.food -= consumption;
        if (globalGameState.food < 0) globalGameState.food = 0;
    }
    globalGameState.currentWeather = getWeatherForDay();
    if (globalGameState.currentWeather == COLD) {
        gfx_SetColor(3);
        gfx_FillRectangle(0,86,320,52);
    }
    else {
        gfx_SetColor(5);
        gfx_FillRectangle(0,86,320,52);
    }
    gfx_SetColor(3);
    gfx_FillRectangle(0,138,320,86);
    gfx_PrintStringXY("Date:", 120, 158);
    gfx_PrintStringXY("Weather:", 96, 168);
    gfx_PrintStringXY("Health:", 107, 178);
    gfx_PrintStringXY("Food:", 119, 188);
    gfx_PrintStringXY("Next landmark:", 56, 198);
    gfx_PrintStringXY("Miles traveled:", 54, 208);
    char* dateStr = formatDate();
    gfx_PrintStringXY(dateStr, 160, 158);
    free(dateStr);
    char foodStr[15];
    sprintf(foodStr, "%dlb", globalGameState.food);
    gfx_PrintStringXY(foodStr, 160, 188);
    char milesStr[20];
    sprintf(milesStr, "%d miles", globalGameState.progress);
    gfx_PrintStringXY(milesStr, 160, 208);
    char leftStr[20];
    sprintf(leftStr, "%d miles", nextLandmark.mileNum-globalGameState.progress);
    gfx_PrintStringXY(leftStr, 160, 198);
    gfx_PrintStringXY(weatherToString(globalGameState.currentWeather), 160, 168);
    gfx_PrintStringXY(health, 160, 178);
    gfx_Sprite(travelGrass1, 0, 16);
    gfx_Sprite(travelGrass2, 157, 10);
    int distance = nextLandmark.mileNum - globalGameState.progress;
    gfx_SetColor(2);
    gfx_Sprite(ox1, 212, 50);
    msleep(300);
    gfx_FillRectangle(212,50,78,36);
    gfx_Sprite(ox2, 212, 50);
    msleep(300);
    if (distance < 188 && distance > 10) {
        int spriteX = abs(nextLandmark.mileNum-globalGameState.progress-188);
        switch (nextLandmark.type) {
            case ROCK:
                gfx_Sprite(rock, spriteX, 50);
                break;
            case FORT:
                gfx_Sprite(fort, spriteX, 50);
                break;
            case RIVER:
                gfx_Sprite(river, spriteX, 50);
                break;
        }
    }
    if (nextLandmark.mileNum - globalGameState.progress <= 0) {

        gfx_SetTextFGColor(3);
        char* arrive = landmarkNameFromIndex(nextLandmark.index);
        gfx_SetColor(2);
        gfx_FillRectangle(26,86,268,55);
        gfx_SetColor(3);
        gfx_Rectangle(26,86,268,55);
        gfx_Rectangle(27,87,267,54);
        gfx_Rectangle(28,88,266,53);
        char buffer[300];
        buffer[0] = '\0';
        sprintf(buffer, "You have arrived at %s. Would you like to look around?", arrive);
        List* splitLines = splitStrLines(buffer, 18);
        bool result = false;
        for (int i = 0; i < splitLines->length; i++) {
            char* res = (char*)List_GetElement(splitLines,i)->data;
            dbg_printf("\n%s\n", res);
            gfx_PrintStringXY(res, 36,95+(i*8));
        }
        
        dbg_printf("Starting bool prompt!\n");
        result = newBoolPrompt("", (vector2){277,120});
        List_Free_List(splitLines);
        bool didLookAround = result;
        if (result) {
            gfx_FillScreen(2);
            gfx_SetColor(3);
            gfx_SetTextFGColor(2);
            gfx_FillRectangle(61,197,198,26);
            fsSprite(nextLandmark.img1, (vector2){0, 0});
            fsSprite(nextLandmark.img2, (vector2){160, 0} );
            char* formatted = formatDate();
            gfx_PrintStringXY(formatted,69,208);
            free(formatted);
            gfx_PrintStringXY(arrive, 69, 200);
            msleep(500);
            while (os_GetCSC() != sk_Enter) {}
            gfx_SetColor(2);
            gfx_SetTextFGColor(3);
        }
        if (nextLandmark.type == RIVER) {
            gfx_FillScreen(2);
            fsSprite(FLAIR1, (vector2){12, 3});
            fsSprite(FLAIR2, (vector2){162, 3});
            fsSprite(FLAIR1, (vector2){12, 215});
            fsSprite(FLAIR2, (vector2){162, 215});
            char* formatted = formatDate();
            gfx_PrintStringXY(formatted, 8, 20);
            gfx_PrintStringXY(landmarkNameFromIndex(nextLandmark.index), 8, 28);
            free(formatted);
            gfx_PrintStringXY("You must cross the river in order", 23, 98);
            gfx_PrintStringXY("to continue. The river at this point is", 23, 106);
            char rstats[300];
            riverStats stats = getRandomRiverStats();
            sprintf(rstats, "currently %dft across, & %.1fft deep.", stats.width, stats.depth);
            gfx_PrintStringXY(rstats, 16, 114);
            while (os_GetCSC() != sk_Enter) {}
            gfx_SetColor(2);
            gfx_FillRectangle(0,54,320,149);
            gfx_PrintStringXY("Weather:",1,54);
            gfx_PrintStringXY("River width:", 1, 54+8);
            gfx_PrintStringXY("River depth:", 1, 54+16 );
            gfx_PrintStringXY(weatherToString(globalGameState.currentWeather), 33, 54);
            char widthStr[30];
            char depthStr[30];
            sprintf(widthStr, "%dft", stats.width);
            sprintf(depthStr, "%.1fft", stats.depth);
            gfx_PrintStringXY(widthStr,104,54+8);
            gfx_PrintStringXY(depthStr,104,54+16);
            if (stats.isFerryable) {
                gfx_PrintStringXY("Ferry cost:", 1, 54+24);
            }
            int choice = -1;
            bool actualFail = false;
            while (1==1) {
                choice = newNumberedPromptStruct(stats.isFerryable ? npp[NPROMPT_FERRYRIVER] : npp[NPROMPT_NORMALRIVER]);
                bool fail = false;
                if ( (choice == 1 && stats.depth <= 2.5) || randomInt(0,10) < 3 ) fail = true;
                if ( fail == false && ((choice == 2 && stats.depth >= 2.5) || randomInt(0,10) < 3) ) fail = true;
                if (choice == 3) fail = false;
                bool doBreak = false;
                switch (choice) {
                    case 1: {
                        doBreak = true;
                        actualFail = fail;
                        break;
                    }
                    case 2: {
                        doBreak = true;
                        actualFail = fail;
                        break;
                    }
                    case 3: {
                        break;
                    }
                    case 4: {
                        break;
                    }
                    case 5: {
                        doBreak = true;
                        actualFail = false;
                        travelModifier = 0;
                        travelModifierLength = 3;
                        break;
                    }
                }
                if (doBreak) break;
            }
            dbg_printf("\nChoice: %d\n", choice);
            int progress = 0; // 0-50;
            int progfactor = 5;
            gfx_SetDrawBuffer();
            while (progress <= 50) {
                gfx_SetColor(2);
                gfx_FillScreen(2);
                gfx_FillRectangle(80,60,160,120);
                gfx_SetColor(3);
                gfx_Rectangle(81,61,160,120);
                gfx_Rectangle(82,62,160,120);
                gfx_Rectangle(80,60,160,120);
                gfx_SetColor(6);
                gfx_FillRectangle(83,63,157,117);
                if (progress <= 10 || progress >= 40) {
                    int triX = progfactor*(progress <= 10 ? (10-progress) : (progress-39));
                    gfx_SetColor(7);
                    if (progress <= 10) gfx_FillTriangle(240-(triX),180,240,180,240,180-(triX));
                    if (progress >= 40) gfx_FillTriangle(80+(triX),60,80,60,80,60+(triX));
                }
                int height = 0;
                int width = 0;
                switch (choice) {
                    case 1: {
                        gfx_Sprite(ford,160,120);
                        height = ford_height;
                        width = ford_width;
                        break;
                    }
                    case 2: {
                        gfx_Sprite(caulk,160,120);
                        height = caulk_height;
                        width = caulk_width;
                        break;
                    }
                    case 5: {
                        gfx_Sprite(ferry,160,120);
                        height = ferry_height;
                        width = ferry_width;
                        break;
                    }
                    default: {break;}
                }
                if (progress > 30 && actualFail) {
                    gfx_SetColor(6);
                    vector2 pos1 = (vector2){160,120+height/2};
                    int heightdiff = (progress-29);
                    gfx_SetColor(6);
                    gfx_FillRectangle(pos1.x, pos1.y-heightdiff, width, height);
                    if (progress > 35) break;
                }
                gfx_SwapDraw();
                msleep(100);
                progress++;
            }
            gfx_SetDrawScreen();
            if (actualFail) {
                gfx_SetColor(2);
                gfx_FillRectangle(26,86,268,55);
                gfx_SetColor(3);
                gfx_Rectangle(26,86,268,55);
                gfx_Rectangle(27,87,267,54);
                gfx_Rectangle(28,88,266,53);
                int failSeverity = randomInt(0,40)+randomInt(0,stats.width/16)+randomInt(0,(int)(stats.depth)*2); // [0-40]+[0-31]+[0-20] = [0-91]
                gfx_PrintStringXY("You lost:", 30, 94);
                if (failSeverity < 10) {
                    gfx_PrintStringXY("Nothing", 30, 102);    
                }
                else if (failSeverity < 30) {
                    rollRandomLoss((vector2){30,102});
                }
                else if (failSeverity < 50) {
                    rollRandomLoss((vector2){30,102});
                    rollRandomLoss((vector2){30,110});
                }
                else if (failSeverity < 70) {
                    rollRandomLoss((vector2){30,102});
                    rollRandomLoss((vector2){30,110});
                    rollRandomLoss((vector2){30,118});
                }
                else {
                    rollRandomLoss((vector2){30,102});
                    rollRandomLoss((vector2){30,110});
                    rollRandomLoss((vector2){30,118});
                    rollRandomLoss((vector2){30,126});
                }
                
            }
            else {
                gfx_SetColor(2);
                gfx_FillRectangle(26,86,268,55);
                gfx_SetColor(3);
                gfx_Rectangle(26,86,268,55);
                gfx_Rectangle(27,87,267,54);
                gfx_Rectangle(28,88,266,53);
                gfx_PrintStringXY("You got across w/o any problems.", 30, 94);
                while (os_GetCSC() != sk_Enter);
            }
            while (os_GetCSC() != sk_Enter) {}
        }
        else if (nextLandmark.type == FORT) {
            if (didLookAround) {
                isFortScreen = true;
            }
        }
        if (nextLandmark.mileNum == WILLAMETTE) {
            winScreen();
        }
        nextLandmark = getNextLandmark();
        gfx_SetColor(2);
        gfx_FillRectangle(26,86,268,55);
        gfx_SetColor(3);
        gfx_Rectangle(26,86,268,55);
        gfx_Rectangle(27,87,267,54);
        gfx_Rectangle(28,88,266,53);
        char buffer2[150];
        sprintf(buffer2, "From %s it is %d miles to the %s", landmarkNameFromIndex(nextLandmark.index-1), nextLandmark.mileNum-globalGameState.progress, landmarkNameFromIndex(nextLandmark.index));
        List* s = splitStrLines(buffer2, 33);
        for (int i = 0; i < s->length; i++) {
            gfx_PrintStringXY((char*)List_GetElement(s, i)->data, 36, 95+(i*8));
        }
        List_Free_List(s);
        while (os_GetCSC() != sk_Enter) {}
        dbg_printf("\nNext landmark %d, %s\n", nextLandmark.mileNum, landmarkNameFromIndex(nextLandmark.index));
        return;
    }
    gfx_SetTextFGColor(3);
    if (strlen(event) != 0 || someoneDied) {
        if (strcmp(event, "^^^^") == 0) {
            free(event);
        }
        else {
            gfx_SetColor(2);
            gfx_FillRectangle(26,86,268,55);
            gfx_SetColor(3);
            gfx_Rectangle(26,86,268,55);
            gfx_Rectangle(27,87,267,54);
            gfx_Rectangle(28,88,266,53);
            if (strlen(event) != 0) {
                List* splitLines = splitStrLines(event, 29);
                for (int i = 0; i < splitLines->length; i++) {
                    char* res = (char*)List_GetElement(splitLines, i)->data;
                    gfx_PrintStringXY(res, 36,95+(i*8));
                }
                List_Free_List(splitLines);
                while (os_GetCSC() != sk_Enter) {}
                free(event);
            }
            else {
                for (int i = 0; i < 5; i++) {
                    if (toAnnounceDead[i]) {
                        gfx_SetColor(2);
                        gfx_FillRectangle(26,86,268,55);
                        gfx_SetColor(3);
                        gfx_Rectangle(26,86,268,55);
                        gfx_Rectangle(27,87,267,54);
                        gfx_Rectangle(28,88,266,53);
                        char announcementMsg[50];
                        sprintf(announcementMsg, "%s has died.", globalGameState.members[i].name);
                        gfx_PrintStringXY(announcementMsg, 36, 95);
                        while (os_GetCSC() != sk_Enter) {}
                    }
                }
            }
        }

    }
    bool someoneAlive = false;
    for (int i = 0; i < 5; i++) {
        if (globalGameState.members[i].health > 0) someoneAlive = true;
    }
    if (!someoneAlive) {
        deathScreen();
    }
    gfx_SetTextFGColor(2);
    gfx_FillRectangle(212,50,78,36);
    gfx_Sprite(ox3, 212, 50);
    msleep(300);
}
void learnClrTxtZone() {
    gfx_SetColor(2);
    gfx_FillRectangle(0,62, 320, 135);
    gfx_SetColor(3);
}
void jobClrTxtZone() {
    gfx_SetColor(2);
    gfx_FillRectangle(0,25, 320, 190);
    gfx_SetColor(3);
}
void learnPrompt() {
    learnClrTxtZone();
    newTextPromptStruct(tpp[TPROMPT_LEARN], learnClrTxtZone);
    learnClrTxtZone();
    newNumberedPromptStruct(npp[NPROMPT_START]);
}
void jobLearnPrompt() {
    jobClrTxtZone();
    newTextPromptStruct(tpp[TPROMPT_JOBINFO], jobClrTxtZone);
    jobClrTxtZone();
}

void monthLearnGUI() {
    while (1==1) {
        gfx_FillScreen(2);
        fsSprite(FLAIR1, (vector2){12, 3});
        fsSprite(FLAIR2, (vector2){162, 3});
        fsSprite(FLAIR1, (vector2){12, 215});
        fsSprite(FLAIR2, (vector2){162, 215});
        gfx_SetColor(3);
        gfx_PrintStringXY("It is 1848. Your jumping off place ", 28, 42);
        gfx_PrintStringXY("for Oregon is Independence, Missouri.", 28, 50);
        gfx_PrintStringXY("You must decide which month to leave", 28, 58);
        gfx_PrintStringXY("Independence, Missouri", 28, 66);
        bool doBreak = false;
        switch (newNumberedPromptStruct(npp[NPROMPT_MONTH])) {
            case 1: {globalGameState.day = 60;doBreak = true; break;}
            case 2: {globalGameState.day = 91;doBreak = true; break;}
            case 3: {globalGameState.day = 121;doBreak = true; break;}
            case 4: {globalGameState.day = 152;doBreak = true; break;}
            case 5: {globalGameState.day = 182;doBreak = true; break;}
            case 6: {monthLearnInfoPrompt(); break;}
        }
        if (doBreak) break;
    }
    monthChoiceMade();
}
void monthLearnInfoPrompt() {
    gfx_FillScreen(2);
    fsSprite(FLAIR1, (vector2){12, 3});
    fsSprite(FLAIR2, (vector2){162, 3});
    fsSprite(FLAIR1, (vector2){12, 215});
    fsSprite(FLAIR2, (vector2){162, 215});
    gfx_SetColor(3);
    newTextPromptStruct(tpp[TPROMPT_MONTHINFO], jobClrTxtZone);
}
void partyNameGUI() {
    gfx_FillScreen(2);
    fsSprite(WAGON1, (vector2){0,0});
    fsSprite(WAGON2, (vector2){150,0});
    bool isNameCorrect = false;
    while (!isNameCorrect) {
        gfx_SetColor(2);
        gfx_FillRectangle(30,153,310,87);
        gfx_PrintStringXY("What is the first name of the", 30, 153); // TODO: finish this
        gfx_PrintStringXY("wagon leader?", 30, 161);
        globalGameState.members[0].name = newUserTextPrompt("", (vector2){128, 161}, 8, NULL, false);
        globalGameState.members[0].health = 100;
        gfx_SetColor(2);
        gfx_FillRectangle(30,153,310,87); // Fill screen w/ black
        gfx_PrintStringXY("What are the first names of the", 30, 153);
        gfx_PrintStringXY("four other members in your party?", 30, 161);
        gfx_PrintStringXY("1. ", 30, 176);
        gfx_PrintStringXY(globalGameState.members[0].name, 56, 176);
        gfx_PrintStringXY("Tip: Press ALPHA to toggle lowercase", 0, 224); // center is (160, 240); so 160-48
        globalGameState.members[1].name = newUserTextPrompt("2. ", (vector2){30, 184}, 8, NULL, false);
        globalGameState.members[1].health = 100;
        globalGameState.members[2].name = newUserTextPrompt("3. ", (vector2){30, 192}, 8, NULL, false);
        globalGameState.members[2].health = 100;
        globalGameState.members[3].name = newUserTextPrompt("4. ", (vector2){30, 200}, 8, NULL, false);
        globalGameState.members[3].health = 100;
        globalGameState.members[4].name = newUserTextPrompt("5. ", (vector2){30, 208}, 8, NULL, false);
        globalGameState.members[4].health = 100;
        gfx_SetColor(2);
        gfx_FillRectangle(0,224,320,16);
        isNameCorrect = newBoolPrompt("Are these names correct?", (vector2){0,224} );
        gfx_SetColor(2);
        gfx_FillRectangle(0,224,320,16);
    }
    monthLearnGUI();
}

void travelTrailPrompt() {
    while (1==1) {
        gfx_FillScreen(2);
        fsSprite(FLAIR1, (vector2){12, 3});
        fsSprite(FLAIR2, (vector2){162, 3});
        fsSprite(FLAIR1, (vector2){12, 215});
        fsSprite(FLAIR2, (vector2){162, 215});
        gfx_PrintStringXY("Many kinds of people made the", 27, 48);
        gfx_PrintStringXY("trip to Oregon.", 27, 56);
        gfx_PrintStringXY("between these options", 69, 136);
        bool doBreak = false;
        switch (newNumberedPromptStruct(npp[NPROMPT_JOBS])) {
            case 1: {globalGameState.job = JOB_BANKER; globalGameState.money = 1600;doBreak = true; break;}
            case 2: {globalGameState.job = JOB_CARPENTER; globalGameState.money = 800;doBreak = true; break;}
            case 3: {globalGameState.job = JOB_FARMER; globalGameState.money = 400;doBreak = true; break;}
            case 4: {jobLearnPrompt(); break;}
        };
        if (doBreak) break;
    }
    partyNameGUI();
}
struct bill {
    double Oxen;
    double Food;
    double Clothing;
    double Ammunition;
    double wheels;
    double axles;
    double tongues;
};
/*
Matt's prices:
Oxen - $40/2
Food - $0.2/per
Clothing - $10/per
Ammunition - $20/2
Parts - $10/per
*/
struct bill currentBill = (struct bill){
    .Ammunition = 0,
    .axles = 0,
    .Clothing = 0,
    .Food = 0,
    .Oxen = 0,
    .tongues = 0,
    .wheels = 0
};
void printMoneyAmount(double money, vector2 pos, int color) {
    gfx_SetTextFGColor(color);
    char toPrint[10];
    sprintf(toPrint, "$%.2f", money);
    gfx_PrintStringXY(toPrint, pos.x, pos.y);
    gfx_SetTextFGColor(3);
}
void clearShopGUIForPurchase() {
    gfx_SetColor(2);
    gfx_FillRectangle(71,0,249,239);
    gfx_SetColor(4);
    gfx_FillRectangle(71,3,224,3);
    gfx_FillRectangle(71,27,224,3);
    gfx_SetColor(3);
    gfx_PrintStringXY("Matt's General Store", 93, 10);
    gfx_PrintStringXY("Independence, Missouri", 93, 18);
}
void purchaseMattsOxen() {
    clearShopGUIForPurchase();
    gfx_SetColor(3);
    gfx_PrintStringXY("There are 2 oxen in a yoke;", 79, 74);
    gfx_PrintStringXY("I recommend at least 3 yoke.", 79, 82);
    gfx_PrintStringXY("I charge $40 a yoke.", 79, 90);
    gfx_PrintStringXY("How many yoke do you", 79, 106);
    currentBill.Oxen = 2*newUserNumberPrompt("", (vector2){78, 128}, "want?", (vector2){78, 128}, 1, 2, NULL);
}
void purchaseMattsFood() {
    clearShopGUIForPurchase();
    gfx_SetColor(3);
    gfx_PrintStringXY("I recommend you take at", 79, 74);
    gfx_PrintStringXY("least 200 pounds of food", 79, 82);
    gfx_PrintStringXY("for each person in your family", 79, 90);
    gfx_PrintStringXY("I see that you have 5 people in all", 79, 98);
    gfx_PrintStringXY("You'll need flour, sugar, bacon,", 79, 106);
    gfx_PrintStringXY("and coffe. My price is $0.2/lb", 79, 114);
    gfx_PrintStringXY("How many pounds of food do", 79, 130);
    currentBill.Food = newUserNumberPrompt("", (vector2){79, 138}, "you want?", (vector2){79, 138}, 1, 3, NULL);
}
void purchaseMattsClothes() {
    clearShopGUIForPurchase();
    gfx_SetColor(3);
    gfx_PrintStringXY("You'll need warm clothing in", 79, 74);
    gfx_PrintStringXY("the mountains. I recommend", 79, 82);
    gfx_PrintStringXY("taking at least 2 sets of clothes", 79, 90);
    gfx_PrintStringXY("per person. Each set is $10.", 79, 98);
    gfx_PrintStringXY("How many sets of clothes do", 79, 114);
    currentBill.Clothing = newUserNumberPrompt("", (vector2){79, 122}, "you want?", (vector2){79, 122}, 1, 2, NULL);
}
void purchaseMattAmmo() {
    clearShopGUIForPurchase();
    gfx_SetColor(3);
    gfx_PrintStringXY("If you run low on food, or ", 79, 74);
    gfx_PrintStringXY("Indians attack you, you'll need ammunition", 79, 82);
    gfx_PrintStringXY("I sell it in boxes of 20 bullets. Each box", 79, 90);
    gfx_PrintStringXY("costs $2.", 79, 98);
    gfx_PrintStringXY("How many boxes of ammo do", 79, 114);
    currentBill.Ammunition = newUserNumberPrompt("", (vector2){79, 122}, "you want?", (vector2){79, 122}, 1, 3, NULL);
}
void purchaseMattsParts() {
    clearShopGUIForPurchase();
    gfx_SetColor(3);
    gfx_PrintStringXY("It's a good idea to have a", 79, 74);
    gfx_PrintStringXY("few spare parts for your", 79, 82);
    gfx_PrintStringXY("wagon. Here are the prices:", 79, 90);
    gfx_PrintStringXY("wagon wheel - $10 each", 87, 106);
    gfx_PrintStringXY("wagon axle - $10 each", 87, 114);
    gfx_PrintStringXY("wagon tongue - $10 each", 87, 122);
    currentBill.wheels = newUserNumberPrompt("", (vector2){79, 138}, "How many wagon wheels?", (vector2){79, 138}, 1, 2, NULL);
    gfx_SetColor(2);
    gfx_FillRectangle(79,138,200,200);
    gfx_SetColor(3);
    currentBill.axles = newUserNumberPrompt("", (vector2){79, 138}, "How many wagon axles?", (vector2){79, 138}, 1, 2, NULL);
    gfx_SetColor(2);
    gfx_FillRectangle(79,138,200,200);
    gfx_SetColor(3);
    currentBill.tongues = newUserNumberPrompt("", (vector2){79, 138}, "How many wagon axles?", (vector2){79, 138}, 1, 2, NULL);
}
char* healthToString() {
    double sum = 0;
    double alive = 0;
    for (int i = 0; i < 5; i++) {
        if (globalGameState.members[i].health != 0) {
            sum += globalGameState.members[i].health;
            alive++;
        }
    }
    double avg = sum/alive;
    if (avg > 90) return "Excellent";
    if (avg > 75) return "Good";
    if (avg == 69) return "Nice";
    if (avg > 40) return "Mid";
    if (avg > 25) return "Struggling";
    if (avg > 10) return "Poor";
    return "Absolutely Cooked";
}
char* paceToString() {
    switch (globalGameState.currentPace) {
        case STEADY: return "Steady";
        case GRUELING: return "Grueling";
        case STRENUOUS: return "Strenuous";
    }
}
char* rationsToString() {
    switch (globalGameState.currentRations) {
        case FILLING: return "Filling";
        case MEAGER: return "Meager";
        case BAREBONES: return "Bare Bones";
    }
}
void changePaceF() {
    gfx_FillScreen(2);
    fsSprite(FLAIR1, (vector2){12, 3});
    fsSprite(FLAIR2, (vector2){162, 3});
    fsSprite(FLAIR1, (vector2){12, 215});
    fsSprite(FLAIR2, (vector2){162, 215});
    // 111, 37+8
    char* currentPaceStr = paceToString();
    char* currentlyTxt = malloc(strlen(currentPaceStr)+20);
    currentlyTxt[0] = '\0';
    sprintf(currentlyTxt, "(currently \"%s\")", currentPaceStr);
    gfx_PrintStringXY(currentlyTxt, 111, 45);
    gfx_PrintStringXY("The pace at which you travel", 27, 71);
    gfx_PrintStringXY("can change. Your choices are:", 27, 79);
    int choice = newNumberedPromptStruct(npp[NPROMPT_PACE]);
    switch (choice) {
        case 1: {globalGameState.currentPace = STEADY; break; }
        case 2: {globalGameState.currentPace = STRENUOUS; break;}
        case 3: {globalGameState.currentPace = GRUELING; break;}
    }
}
void changeRationsF() {
    gfx_FillScreen(2);
    fsSprite(FLAIR1, (vector2){2, 2});
    fsSprite(FLAIR2, (vector2){159, 2});
    fsSprite(FLAIR1, (vector2){2, 220});
    fsSprite(FLAIR2, (vector2){159, 220});
    // 111, 37+8
    char* currentRationsStr = rationsToString();
    char* currentlyTxt = malloc(sizeof(currentRationsStr)+20);
    currentlyTxt[0] = '\0';
    sprintf(currentlyTxt, "(currently \"%s\")", currentRationsStr);
    gfx_PrintStringXY(currentlyTxt, 111, 45);
    gfx_PrintStringXY("The amount of food the people in", 27, 71);
    gfx_PrintStringXY("your party eat each day can", 27, 79);
    gfx_PrintStringXY("change. These amounts are:", 27, 87);
    int choice = newNumberedPromptStruct(npp[NPROMPT_RATIONS]);
    switch (choice) {
        case 1: {globalGameState.currentRations = FILLING; break; }
        case 2: {globalGameState.currentRations = MEAGER; break;}
        case 3: {globalGameState.currentRations = BAREBONES; break;}
    }
}
void checkSuppliesF() {
    gfx_FillScreen(2);
    gfx_PrintStringXY("Your Supplies", 110, 15);
    gfx_PrintStringXY("Oxen", 91, 41);
    gfx_PrintStringXY("Sets of Clothing", 91, 49);
    gfx_PrintStringXY("Bullets", 91, 57);
    gfx_PrintStringXY("Wagon Wheels", 91, 65);
    gfx_PrintStringXY("Wagon Axles", 91, 73);
    gfx_PrintStringXY("Wagon Tongues", 91, 81);
    gfx_PrintStringXY("Pounds of food", 91, 89);
    gfx_PrintStringXY("Money Left", 91, 97);
    double maxValue = 0;
    if (globalGameState.ammunitions > maxValue) maxValue = globalGameState.ammunitions;
    if (globalGameState.oxen > maxValue) maxValue = globalGameState.oxen;
    if (globalGameState.axles > maxValue) maxValue = globalGameState.axles;
    if (globalGameState.clothing > maxValue) maxValue = globalGameState.clothing;
    if (globalGameState.food > maxValue) maxValue = globalGameState.food;
    if (globalGameState.tongues > maxValue) maxValue = globalGameState.tongues;
    if (globalGameState.wheels > maxValue) maxValue = globalGameState.wheels;
    int len = floor(log10(maxValue));
    gfx_SetTextXY(246, 41);
    gfx_PrintInt(globalGameState.oxen, len);
    gfx_SetTextXY(246, 49);
    gfx_PrintInt(globalGameState.clothing, len);
    gfx_SetTextXY(246, 57);
    gfx_PrintInt(globalGameState.ammunitions, len);
    gfx_SetTextXY(246, 65);
    gfx_PrintInt(globalGameState.wheels, len);
    gfx_SetTextXY(246, 73);
    gfx_PrintInt(globalGameState.axles, len);
    gfx_SetTextXY(246, 81);
    gfx_PrintInt(globalGameState.tongues, len);
    gfx_SetTextXY(246, 89);
    gfx_PrintInt(globalGameState.food, len);
    gfx_SetTextXY(246, 97);
    gfx_PrintInt(globalGameState.money, len);
    while (os_GetCSC() != sk_Enter) {}
}
#define KEYLENGTH 18
#define KEYHEIGHT 16
#define KEYSIZE (vector2){KEYLENGTH, KEYHEIGHT}
void drawKey(char* key, vector2 pos, vector2 keySize) {
    gfx_SetTextFGColor(2);
    gfx_SetColor(3);
    gfx_FillRectangle(pos.x,pos.y,keySize.x, keySize.y);
    
    gfx_SetColor(2);
    gfx_SetPixel(pos.x,pos.y);
    gfx_SetPixel(pos.x+keySize.x-1,pos.y);
    gfx_SetPixel(pos.x,pos.y+keySize.y-1);
    gfx_SetPixel(pos.x+keySize.x-1,pos.y+keySize.y-1);
    vector2 center = (vector2){pos.x+(keySize.x/2), pos.y+(keySize.y/4)};
    int len = strlen(key);
    vector2 startPos = (vector2){center.x - (len*4), center.y};
    gfx_PrintStringXY(key,startPos.x, startPos.y);
    gfx_SetTextFGColor(3);
    gfx_SetColor(2);

}
int randomInt(int min, int max) {
    return (rand()%(max-min+1)) + min;
}
typedef enum {
    GRASS1,
    GRASS2,
    GRASS3,
    TREE1,
    RABBIT,
    DEADRABBIT,
    SQUIRREL,
    DEADSQUIRREL,
    BISON,
    DEADBISON,
    DEER,
    DEADDEER,
    BULLET
} obstacleTypes;
typedef struct {
    vector2 pos;
    vector2 size;
    obstacleTypes type;
} huntingObject;
void displayOBJs(huntingObject* arr, int len,int deadAnimalAllowance) {
    for (int i = 0; i < len+deadAnimalAllowance; i++) {
        huntingObject obj = arr[i];
        if (obj.pos.x == -1 && obj.pos.y == -1) continue;
        switch (obj.type) {
            case GRASS1: {gfx_Sprite(grass1, obj.pos.x, obj.pos.y); break;}
            case GRASS2: {gfx_Sprite(grass2, obj.pos.x, obj.pos.y); break;}
            case GRASS3: {gfx_Sprite(grass3, obj.pos.x, obj.pos.y); break;}
            case TREE1: {gfx_Sprite(tree1, obj.pos.x, obj.pos.y); break;}
            case DEADRABBIT: {gfx_Sprite(rbDead, obj.pos.x, obj.pos.y); break;}
            case DEADSQUIRREL: {gfx_Sprite(sqDead, obj.pos.x, obj.pos.y); break;}
            case DEADBISON: {gfx_Sprite(bnDead, obj.pos.x, obj.pos.y); break;}
            case DEADDEER: {gfx_Sprite(drDead, obj.pos.x, obj.pos.y); break;}
            default: {break;}
        }
    }
}
bool isCollision(vector2 a, vector2 aSize, vector2 b, vector2 bSize) {
    return (
        a.x < b.x+bSize.x && 
        a.x + aSize.x > b.x &&
        a.y < b.y + bSize.y &&
        a.y + aSize.y > b.y
    );
}

vector2 generateAnimalStartPos(int tolerance) {
    switch (randomInt(1,4)) {
        case 1: {return (vector2){randomInt(0,tolerance),randomInt(0,240)};}
        case 2: {return (vector2){randomInt(0,320), randomInt(0, tolerance)};}
        case 3: {return (vector2){randomInt(320-tolerance, 320), randomInt(0,240)};}
        case 4: {return (vector2){randomInt(0,320), randomInt(240-tolerance, 240)};}
        default: {dbg_printf("Invalid tolerance or randomInt;"); return (vector2){0,0};}
    }
}
void getRandomAnimal(huntingObject* obj) {
    obj->pos = generateAnimalStartPos(10);
    int typeNum = randomInt(1, 100);
    if (typeNum < 10) {
        obj->type = BISON;
        obj->size = (vector2){23,22};
    }
    else if (typeNum < 40) {
        obj->type = DEER;
        obj->size = (vector2){26,25};
    } 
    else if (typeNum < 80) {
        obj->type = RABBIT;
        obj->size = (vector2){12,11};
    }
    else {
        obj->type = SQUIRREL;
        obj->size = (vector2){23,9};
    }
}
typedef enum {
    NORTH,
    NORTHEAST,
    EAST,
    SOUTHEAST,
    SOUTH,
    SOUTHWEST,
    WEST,
    NORTHWEST
} directions;
typedef struct {
    huntingObject objData;
    vector2 velocity;
    int listNum;
} bullet;
vector2 velocityFromDirection(directions d) {
    switch (d) {
        case NORTH: return (vector2){0,-1};
        case SOUTH: return (vector2){0,1};
        case EAST: return (vector2){1,0};
        case WEST: return (vector2){-1,0};
        case NORTHEAST: return (vector2){1,-1};
        case NORTHWEST: return (vector2){-1,-1};
        case SOUTHEAST: return (vector2){1,1};
        case SOUTHWEST: return (vector2){-1,1};
    }
}
vector2 getbulletStartPos(huntingObject hunter, directions d) {
    vector2 toReturn = (vector2){hunter.pos.x+hunter.size.x/2, hunter.pos.y+hunter.size.y/2};
    switch (d) {
        case NORTH: {
            toReturn.y -= hunter.size.y;
            break;
        }
        case SOUTH: {
            toReturn.y += hunter.size.y;
            break;
        }
        case EAST: {
            toReturn.x -= hunter.size.x;
            break;
        }
        case WEST: {
            toReturn.x += hunter.size.x;
            break;
        }
        case NORTHEAST: {
            toReturn.y -= hunter.size.y;
            toReturn.x += hunter.size.x;
            break;
        }
        case NORTHWEST: {
            toReturn.y -= hunter.size.y;
            toReturn.x -= hunter.size.x;
            break;
        }
        case SOUTHEAST: {
            toReturn.y += hunter.size.y;
            toReturn.x -= hunter.size.x;
            break;
        }
        case SOUTHWEST: {
            toReturn.y -= hunter.size.y;
            toReturn.x -= hunter.size.x;
            break;
        }
    }
    return toReturn;
}
void startHunt() {
    if (globalGameState.ammunitions <= 0) {
        gfx_FillScreen(2);
        gfx_PrintStringXY("You don't have any bullets.", 95, 16);
        while (os_GetCSC() != sk_Enter) {}
        dbg_printf("Hunt ended; going into sizeup().");
        return;
    }
    gfx_FillScreen(2);
    
    gfx_PrintStringXY("Hunting Instructions", 61, 15);
    drawKey("stat", (vector2){3,43},(vector2){KEYLENGTH*2,KEYHEIGHT});
    drawKey(")", (vector2){3+(2*KEYLENGTH)+4,43},(vector2){KEYLENGTH, KEYHEIGHT});
    gfx_PrintStringXY("To start or stop walking", 101, 43+KEYHEIGHT/4);
    
    drawKey("del", (vector2){3, 55+KEYHEIGHT+4}, (vector2){ceil(KEYLENGTH*1.5), KEYHEIGHT});
    drawKey("(", (vector2){3+ceil(KEYLENGTH*1.5)+4, 55+KEYHEIGHT+4}, KEYSIZE);
    gfx_PrintStringXY("To fire the rifle", 101, 55+KEYHEIGHT+4+KEYHEIGHT/4);
    
    drawKey("<-", (vector2){3, 67+KEYHEIGHT+16}, (vector2){KEYLENGTH*2, KEYHEIGHT});
    drawKey("->", (vector2){3+(2*KEYLENGTH)+4, 67+KEYHEIGHT+16}, (vector2){KEYLENGTH*2, KEYHEIGHT});
    gfx_PrintStringXY("To point the rifle", 101, 67+KEYHEIGHT+16);
    gfx_PrintStringXY("(novice hunters)", 101, 75+KEYHEIGHT+16);
    
    int y = 75+KEYHEIGHT+32;
    drawKey("7", (vector2){5, y}, KEYSIZE);
    drawKey("8", (vector2){8+KEYLENGTH, y}, KEYSIZE);
    drawKey("9", (vector2){11+(2*KEYLENGTH), y}, KEYSIZE);

    drawKey("4", (vector2){5, y+KEYHEIGHT+4}, KEYSIZE);
    drawKey("5", (vector2){8+KEYLENGTH, y+KEYHEIGHT+4}, KEYSIZE);
    drawKey("6", (vector2){11+(2*KEYLENGTH), y+KEYHEIGHT+4}, KEYSIZE);

    drawKey("7", (vector2){5, y+(2*KEYHEIGHT)+8}, KEYSIZE);
    drawKey("8", (vector2){8+KEYLENGTH, y+(2*KEYHEIGHT)+8}, KEYSIZE);
    drawKey("9", (vector2){11+(2*KEYLENGTH), y+(2*KEYHEIGHT)+8}, KEYSIZE);

    int newy = y+(2*KEYHEIGHT)+32;
    
    vector2 advPointInstructions = (vector2){101, y+KEYHEIGHT+4};
    gfx_PrintStringXY("To point the rifle", advPointInstructions.x, advPointInstructions.y);
    gfx_PrintStringXY("(expert hunters)", advPointInstructions.x, advPointInstructions.y+8);
    
    drawKey("clear", (vector2){3, newy}, (vector2){ceil(KEYLENGTH*2.5), KEYHEIGHT});
    gfx_PrintStringXY("To stop hunting", 101, newy+4);
    
    
    msleep(60);
    while (os_GetCSC() != sk_Enter) {}

    gfx_SetDrawBuffer();
    gfx_FillScreen(2);
    int obstaclesAmt = randomInt(6,10); // Random # between 4 & 8 obstacles.
    int maxAnimals = 4;
    huntingObject* obstacles = malloc(sizeof(huntingObject)*(maxAnimals+obstaclesAmt));
    for (int i = 0; i < obstaclesAmt+4; i++) {
        obstacles[i] = (huntingObject){
            .pos = (vector2){-1, -1},
            .size = (vector2){0, 0},
            .type = -1
        };
    }
    for (int i = 0; i < obstaclesAmt; i++) {
        huntingObject* cOBJ = &obstacles[i];
        cOBJ->type = randomInt(0, TREE1);
        switch (cOBJ->type) {
            case GRASS1: {cOBJ->size = (vector2){39, 19}; break;};
            case GRASS2: {cOBJ->size = (vector2){38, 12}; break;};
            case GRASS3: {cOBJ->size = (vector2){22, 11}; break;};
            case TREE1: {cOBJ->size = (vector2){33, 49}; break;};
            default: {break;}
        }
        while (1==1) {
            cOBJ->pos = (vector2){randomInt(10, 310), randomInt(10, 210)};
            bool doBreak = true;
            for (int j = 0; j < i; j++) {
                huntingObject* jOBJ = &obstacles[j];
                if (isCollision(cOBJ->pos, cOBJ->size, jOBJ->pos, jOBJ->size)) doBreak = false;
            }
            if (doBreak) break;
        }
    }
    for (int i = obstaclesAmt; i < obstaclesAmt+4; i++) {
        huntingObject* cOBJ = &obstacles[i];
        cOBJ->pos = (vector2){-1,-1};
        cOBJ->type = -1;
        cOBJ->size = (vector2){0,0};
    }
    displayOBJs(obstacles, obstaclesAmt, maxAnimals);
    int animalsCounter = 0;
    huntingObject currentAnimal;
    getRandomAnimal(&currentAnimal);
    vector2 currentTarget = (vector2){320/2, 240/2};
    int walkCycleNum = 1;
    int chanceNum = 0;
    int animCount = 0;
    gfx_sprite_t* flipped = gfx_MallocSprite(32,32);


    huntingObject hunter;
    bool isWalking = false;
    int hunterAnim = 1;
    int bulletSpd = 8;
    directions hunterRotation = NORTH;
    vector2 walkVector = (vector2){0,0};
    
    int maxBullets = 10;
    bullet* bullets = malloc(sizeof(bullet)*(maxBullets+1));
    for (int i = 0; i < maxBullets; i++) {
        bullets[i].objData = (huntingObject){
            .pos = (vector2){-1,-1},
            .size = (vector2){3,3},
            .type = BULLET,
        };
        bullets[i].listNum = i;
        bullets[i].velocity = (vector2){0,0};
        dbg_printf("%d bullet made %d, %d\n", i, bullets[i].velocity.x, bullets[i].velocity.y);
    }
    dbg_printf("Bullets made");
    hunter.size = (vector2){19, 30};
    while (1==1) {
            hunter.pos = (vector2){randomInt(10, 310), randomInt(10, 210)};
            bool doBreak = true;
            for (int j = 0; j < obstaclesAmt; j++) {
                huntingObject* jOBJ = &obstacles[j];
                if (isCollision(hunter.pos, (vector2){hunter.size.x,hunter.size.y}, jOBJ->pos, jOBJ->size)) doBreak = false;
            }
            if (doBreak) break;
    }
    int totalFoodObtained = 0;
    while (animalsCounter < maxAnimals) {
        uint8_t keyPress = os_GetCSC();
        gfx_FillScreen(2);
        displayOBJs(obstacles, obstaclesAmt, maxAnimals);
        bool changeTgt = false;
        if (currentAnimal.pos.x > 320 || currentAnimal.pos.x < 0 || currentAnimal.pos.y > 240 || currentAnimal.pos.y < 0) {
            chanceNum++;
            getRandomAnimal(&currentAnimal);
            changeTgt = true;
        }
        for (int i = 0; i < maxBullets; i++) { // vel = (0,0) (1,0)
            if ((bullets[i].velocity.x != 0 || bullets[i].velocity.y != 0)) {
                bullets[i].objData.pos.x += bullets[i].velocity.x*bulletSpd;
                bullets[i].objData.pos.y += bullets[i].velocity.y*bulletSpd;
                if (isCollision(currentAnimal.pos, currentAnimal.size, bullets[i].objData.pos, bullets[i].objData.size)) {
                    switch (currentAnimal.type) {
                            case RABBIT: {
                                totalFoodObtained += 25;
                                break;
                            }
                            case SQUIRREL: {
                                totalFoodObtained += 20;
                                break;
                            }
                            case DEER: {
                                totalFoodObtained += 100;
                                break;
                            }
                            case BISON: {
                                totalFoodObtained += 500;
                                break;
                            }
                            default: {break;}
                    }
                    for (int k = 0; k < maxAnimals+obstaclesAmt; k++) {
                        if (obstacles[k].pos.x == -1) {
                            obstacles[k] = (huntingObject){
                                .pos = currentAnimal.pos,
                                .size = currentAnimal.size,
                                .type = currentAnimal.type + 1
                            };
                        }
                    }
                    chanceNum++;
                    getRandomAnimal(&currentAnimal);
                    changeTgt = true;
                }
            }
        }
        for (int i = 0; i < obstaclesAmt+4; i++) {
            if (isCollision(currentAnimal.pos, currentAnimal.size, obstacles[i].pos, obstacles[i].size)) {
                changeTgt = true;        
                break;
            }
        }
        if (isCollision(currentAnimal.pos, currentAnimal.size, currentTarget, (vector2){6,7})) changeTgt = true;
        if (changeTgt) {
            while (1==1) {
                currentTarget = (vector2){currentAnimal.pos.x+randomInt(-100, 100), currentAnimal.pos.y+randomInt(-100, 100)};
                bool breakW = true;
                for (int j = 0; j < obstaclesAmt; j++) {
                    if (isCollision(obstacles[j].pos, obstacles[j].size, currentTarget, (vector2){currentAnimal.size.x*2, currentAnimal.size.y*2})) breakW = false;
                }
                if (breakW) break;
            }
        }
        int spd = 1;
        switch (currentAnimal.type) {
            case RABBIT: {spd = 2; break;}
            case SQUIRREL: {spd = 3; break;}
            case BISON: {spd = 1; break;}
            case DEER: {spd = 2; break;}
            default: {break;}
        }
        int xChange = currentTarget.x > currentAnimal.pos.x ? spd : currentTarget.x == currentAnimal.pos.x ? 0 : -spd;
        if (randomInt(1,10) >= 3) currentAnimal.pos.x += currentTarget.x > currentAnimal.pos.x ? spd : currentTarget.x == currentAnimal.pos.x ? 0 : -spd;
        if (randomInt(1,10) >= 3) currentAnimal.pos.y += currentTarget.y > currentAnimal.pos.y ? spd : currentTarget.y == currentAnimal.pos.y ? 0 : -spd;
        gfx_sprite_t* sprite;
        switch (walkCycleNum) {
            case 1: {
                switch (currentAnimal.type) {
                    case BISON: {sprite = bn1; break;}
                    case SQUIRREL: {sprite = sq1; break;}
                    case DEER: {sprite = dr1; break;}
                    case RABBIT: {sprite = rb1;; break;}
                    default: {break;}
                };
                break;
            }
            case 2: {
                switch (currentAnimal.type) {
                    case BISON: {sprite = bn2; break;}
                    case SQUIRREL: {sprite = sq2; break;}
                    case DEER: {sprite = dr2; break;}
                    case RABBIT: {sprite = rb2; break;}
                    default: {break;}
                };
                break;
            }
            case 3: {
                switch (currentAnimal.type) {
                    case BISON: {sprite = bn3; break;}
                    case SQUIRREL: {sprite = sq3; break;}
                    case DEER: {sprite = dr3; break;}
                    case RABBIT: {sprite = rb3; break;}
                    default: {break;}
                };
                break;
            }
        }
        if (xChange < 0) {
            gfx_FlipSpriteY(sprite, flipped);
            gfx_Sprite(flipped, currentAnimal.pos.x, currentAnimal.pos.y);
        }
        else {
            gfx_Sprite(sprite, currentAnimal.pos.x, currentAnimal.pos.y);
        }
        if (chanceNum > 2) {
            getRandomAnimal(&currentAnimal);
            animCount = 0;
            walkCycleNum = 1;
            chanceNum = 0;
            animalsCounter++;
            currentTarget = (vector2){320/2, 240/2};
        }
        if (animCount < 2) {
            animCount++;
        }
        else {
            walkCycleNum = (walkCycleNum >= 3) ? 1 : walkCycleNum + 1;
            animCount = 0;
        }
        if (!isWalking) {
            switch (keyPress) {
                case sk_Up: {hunterRotation = NORTH; walkVector = (vector2){0,1}; break;}
                case sk_8: {hunterRotation = NORTH; walkVector = (vector2){0,1}; break;}
                case sk_Down: {hunterRotation = SOUTH; walkVector = (vector2){0,-1}; break;}
                case sk_2: {hunterRotation = SOUTH; walkVector = (vector2){0,-1}; break;}
                case sk_Right: {hunterRotation = EAST; walkVector = (vector2){1,0}; break;}
                case sk_6: {hunterRotation = EAST; walkVector = (vector2){1,0}; break;}
                case sk_Left: {hunterRotation = WEST; walkVector = (vector2){-1,0}; break;}
                case sk_4: {hunterRotation = WEST; walkVector = (vector2){-1,0}; break;}
                case sk_7: {hunterRotation = NORTHWEST; walkVector = (vector2){-1,1}; break;}
                case sk_9: {hunterRotation = NORTHEAST; walkVector = (vector2){1,1}; break;}
                case sk_1: {hunterRotation = SOUTHWEST; walkVector = (vector2){-1,-1}; break;}
                case sk_3: {hunterRotation = SOUTHEAST; walkVector = (vector2){1,-1}; break;}
                default: {break;}
            }
        }
        if (!isWalking) {
            switch (hunterRotation) {
                case NORTH: {gfx_Sprite(hunterN1, hunter.pos.x, hunter.pos.y); break;}
                case NORTHEAST: {gfx_Sprite(hunterNE1, hunter.pos.x, hunter.pos.y); break;}
                case NORTHWEST: {gfx_Sprite(hunterNW1, hunter.pos.x, hunter.pos.y); break;}
                case EAST: {gfx_Sprite(hunterE1, hunter.pos.x, hunter.pos.y); break;}
                case WEST: {gfx_Sprite(hunterW1, hunter.pos.x, hunter.pos.y); break;}
                case SOUTHWEST: {gfx_Sprite(hunterSW1, hunter.pos.x, hunter.pos.y); break;}
                case SOUTH: {gfx_Sprite(hunterS1, hunter.pos.x, hunter.pos.y); break;}
                case SOUTHEAST: {gfx_Sprite(hunterSE1, hunter.pos.x, hunter.pos.y); break;}
                default: {break;}
            }
        }
        else {
            walkCycleNum += 1;
            hunterAnim = (walkCycleNum > 2) ? hunterAnim+1 : hunterAnim;
            walkCycleNum = (walkCycleNum > 2) ? 0 : walkCycleNum;
            vector2 newPos = (vector2){hunter.pos.x + walkVector.x, hunter.pos.y - walkVector.y};
            bool newPosCollision = false;
            for (int j = 0; j < obstaclesAmt; j++) {
                if (isCollision(newPos, (vector2){hunter.size.x/2,hunter.size.y/2}, obstacles[j].pos, obstacles[j].size) || newPos.x > 320 || newPos.x < 0 || newPos.y > 240 || newPos.y < 0) {newPosCollision = true; break;}
            }
            if (!newPosCollision) {
                hunter.pos = newPos;
            }
            else {
                isWalking = false;
            }
            switch ( (hunterAnim % 3) + 1) {
                case 1: {
                    switch (hunterRotation) {
                        case NORTH: {gfx_Sprite(hunterN1, hunter.pos.x, hunter.pos.y); break;}
                        case NORTHEAST: {gfx_Sprite(hunterNE1, hunter.pos.x, hunter.pos.y); break;}
                        case NORTHWEST: {gfx_Sprite(hunterNW1, hunter.pos.x, hunter.pos.y); break;}
                        case EAST: {gfx_Sprite(hunterE1, hunter.pos.x, hunter.pos.y); break;}
                        case WEST: {gfx_Sprite(hunterW1, hunter.pos.x, hunter.pos.y); break;}
                        case SOUTHWEST: {gfx_Sprite(hunterSW1, hunter.pos.x, hunter.pos.y); break;}
                        case SOUTH: {gfx_Sprite(hunterS1, hunter.pos.x, hunter.pos.y); break;}
                        case SOUTHEAST: {gfx_Sprite(hunterSE1, hunter.pos.x, hunter.pos.y); break;}
                    }
                    break;
                }
                case 2: {
                    switch (hunterRotation) {
                        case NORTH: {gfx_Sprite(hunterN2, hunter.pos.x, hunter.pos.y); break;}
                        case NORTHEAST: {gfx_Sprite(hunterNE2, hunter.pos.x, hunter.pos.y); break;}
                        case NORTHWEST: {gfx_Sprite(hunterNW2, hunter.pos.x, hunter.pos.y); break;}
                        case EAST: {gfx_Sprite(hunterE2, hunter.pos.x, hunter.pos.y); break;}
                        case WEST: {gfx_Sprite(hunterW2, hunter.pos.x, hunter.pos.y); break;}
                        case SOUTHWEST: {gfx_Sprite(hunterSW2, hunter.pos.x, hunter.pos.y); break;}
                        case SOUTH: {gfx_Sprite(hunterS2, hunter.pos.x, hunter.pos.y); break;}
                        case SOUTHEAST: {gfx_Sprite(hunterSE2, hunter.pos.x, hunter.pos.y); break;}
                    }
                    break;
                }
                case 3: {
                    switch (hunterRotation) {
                        case NORTH: {gfx_Sprite(hunterN3, hunter.pos.x, hunter.pos.y); break;}
                        case NORTHEAST: {gfx_Sprite(hunterNE3, hunter.pos.x, hunter.pos.y); break;}
                        case NORTHWEST: {gfx_Sprite(hunterNW3, hunter.pos.x, hunter.pos.y); break;}
                        case EAST: {gfx_Sprite(hunterE3, hunter.pos.x, hunter.pos.y); break;}
                        case WEST: {gfx_Sprite(hunterW3, hunter.pos.x, hunter.pos.y); break;}
                        case SOUTHWEST: {gfx_Sprite(hunterSW3, hunter.pos.x, hunter.pos.y); break;}
                        case SOUTH: {gfx_Sprite(hunterS3, hunter.pos.x, hunter.pos.y); break;}
                        case SOUTHEAST: {gfx_Sprite(hunterSE3, hunter.pos.x, hunter.pos.y); break;}
                    }
                    break;
                }

            }
        }
        for (int k = 0; k < maxBullets; k++) {
            if (bullets[k].objData.pos.x < 0 || bullets[k].objData.pos.x > 320 || bullets[k].objData.pos.y < 0 || bullets[k].objData.pos.y > 240) {
                bullets[k].velocity = (vector2){0,0};
                bullets[k].objData.pos = (vector2){-1,-1};
            }
            else if (bullets[k].velocity.x != 0 || bullets[k].velocity.y != 0) {
                gfx_SetColor(3);
                gfx_FillCircle(bullets[k].objData.pos.x, bullets[k].objData.pos.y, bullets[k].objData.size.x);
                gfx_SetColor(2);
            }
        }
        if ((keyPress == sk_LParen || keyPress == sk_Del) && globalGameState.ammunitions > 0) {
            int usedBulletIndex = 0;
            for (int i = 0; i < maxBullets; i++) {
                if (bullets[i].velocity.x == 0 && bullets[i].velocity.y == 0) {
                    usedBulletIndex = i;
                    break;
                }
            }
            bullets[usedBulletIndex].objData.pos = getbulletStartPos(hunter, hunterRotation);
            bullets[usedBulletIndex].velocity = velocityFromDirection(hunterRotation);
            globalGameState.ammunitions--;
        }
        if (keyPress == sk_Clear) break;
        if (keyPress == sk_RParen || keyPress == sk_Stat) isWalking = !isWalking;
        msleep(10);
        gfx_SwapDraw();
    }
    gfx_SetDrawScreen();
    gfx_FillScreen(2);
    int maxFood = globalGameState.job == JOB_FARMER ? 175 : 100;
    if (totalFoodObtained > maxFood) {
        char obtainTxt[60];
        sprintf(obtainTxt, "You had obtained %d pounds of food.", totalFoodObtained);
        gfx_PrintStringXY(obtainTxt, 16, 16);
        char howeverTxt[60];
        sprintf(howeverTxt, "However, you only could carry back %d pounds.", maxFood);
        gfx_PrintStringXY(howeverTxt, 16, 25);
        globalGameState.food += maxFood;
    }
    else {
        char obtainTxt[40];
        sprintf(obtainTxt, "You have obtained %d pounds of food.", totalFoodObtained);
        gfx_PrintStringXY(obtainTxt, 16, 16);
        globalGameState.food += totalFoodObtained;
    }
    free(obstacles);
    free(bullets);
    free(flipped);
    while (os_GetCSC() != sk_Enter) {}

}
void sizeUp() {
    sizeUpStart:
        while (1==1) {
            dbg_printf("Starting sizeup \n");
            gfx_FillScreen(2);
            gfx_SetColor(3);
            gfx_FillRectangle(3, 35, 314, 53);
            gfx_SetTextFGColor(2);
            gfx_PrintStringXY("Weather: ", 14, 48);
            gfx_PrintStringXY("Health: ", 14, 56);
            gfx_PrintStringXY("Pace: ", 14, 64);
            gfx_PrintStringXY("Rations: ", 14, 72);
            dbg_printf("Getting player data.");
            gfx_PrintStringXY(weatherToString(globalGameState.currentWeather), 86, 48);
            gfx_PrintStringXY(healthToString(), 78, 56 );
            gfx_PrintStringXY(paceToString(), 62, 64);
            gfx_PrintStringXY(rationsToString(), 86, 72);
            dbg_printf("Getting date.");
            gfx_SetTextFGColor(3);
            char* formattedDate = formatDate();
            gfx_PrintStringXY(formattedDate, 16, 8);
            dbg_printf("%s, %d", formattedDate, globalGameState.day);
            free(formattedDate);
            dbg_printf("Putting up num prompt.");
            bool doBreak = false;
            int choice = !isFortScreen ? newNumberedPromptStruct(npp[NPROMPT_SIZEUP]) : newNumberedPromptStruct(npp[NPROMPT_FORTSIZEUP]);
            switch (choice) {
                    case 1: {doBreak = true; break;};
                    case 2: {checkSuppliesF(); break;};
                    case 3: {doBreak = true; break;}
                    case 4: {changePaceF(); break;}
                    case 5: {changeRationsF(); break;}
                    case 6: {doBreak = true; break;}
                    case 7: {doBreak = true; break;}
                    case 8: {
                        if (isFortScreen) shopPrompt(false);
                        if (!isFortScreen) startHunt();
                        break;
                    }
                    
            }        
            isFortScreen = false;
            if (doBreak) break;
        }
    while (1==1) {
        tick();
        if (isFortScreen) goto sizeUpStart;
        if (os_GetCSC() == sk_Enter) goto sizeUpStart;
    }
}
void renderMattsShop() {
    while (1==1) {
        gfx_FillScreen(2);
        gfx_SetColor(4);
        fsSprite(MATT, (vector2){0, 58});
        gfx_FillRectangle(71,3,224,3);
        gfx_FillRectangle(71,54,224,3);
        gfx_FillRectangle(71,130,224,3);
        gfx_SetColor(3);
        gfx_PrintStringXY("Matt's General Store", 93, 10);
        gfx_PrintStringXY("Independence, Missouri", 93, 18);
        char* formattedDate = formatDate();
        gfx_PrintStringXY(formattedDate, 162, 42);
        free(formattedDate);
        char moneyAmtString[30];
        sprintf(moneyAmtString, "Amount you have: $%.2f", globalGameState.money);
        gfx_PrintStringXY(moneyAmtString,80,159);
        printMoneyAmount(currentBill.Oxen*40,(vector2){230,63}, 3);
        printMoneyAmount(currentBill.Food*0.2,(vector2){230,71}, 3);
        printMoneyAmount(currentBill.Clothing*10,(vector2){230,79}, 3);
        printMoneyAmount(currentBill.Ammunition*10,(vector2){230,88}, 3);
        printMoneyAmount((currentBill.tongues+currentBill.wheels+currentBill.axles)*10,(vector2){230,96}, 3);
        gfx_PrintStringXY("Total Bill: ", 125, 140);
        double total = (currentBill.Oxen*40)+(currentBill.Food*0.2)+(currentBill.Clothing*10)+(currentBill.Ammunition*10)+(currentBill.tongues+currentBill.wheels+currentBill.axles)*10;
        int billColor = (total > globalGameState.money) ? 4 : 3;
        printMoneyAmount(total, (vector2){230,140}, billColor);
        gfx_PrintStringXY("Which item would you", 87, 181);
        bool doBreak = false;
        switch (newNumberedPromptStruct(npp[NPROMPT_MATTSHOP])) {
            case 1: {purchaseMattsOxen(); break;};
            case 2: {purchaseMattsFood(); break;}
            case 3: {purchaseMattsClothes(); break;}
            case 4: {purchaseMattAmmo(); break;}
            case 5: {purchaseMattsParts(); break;}
            case 6: {doBreak = true; break;}
        }
        if (doBreak) break;
    }
    globalGameState.ammunitions += currentBill.Ammunition;
    globalGameState.axles += currentBill.axles;
    globalGameState.food += currentBill.Food;
    globalGameState.oxen += currentBill.Oxen;
    globalGameState.wheels += currentBill.wheels;
    globalGameState.tongues += currentBill.tongues;
    globalGameState.clothing += currentBill.Clothing;
    globalGameState.money -= (currentBill.Oxen*40)+(currentBill.Food*0.2)+(currentBill.Clothing*10)+(currentBill.Ammunition*10)+(currentBill.tongues+currentBill.wheels+currentBill.axles)*10;
    gfx_FillScreen(2);
    fsSprite(MATT, (vector2){0,58});
    gfx_PrintStringXY("Well then, you're ready", 77, 78);
    gfx_PrintStringXY("to start. Good luck!", 77, 96);
    gfx_PrintStringXY("You have a long and ", 77, 104);
    gfx_PrintStringXY("difficult journey ahead", 77, 112);
    gfx_PrintStringXY("of you.", 77, 120);
    gfx_FillScreen(2);
    
    fsSprite(INDEPENDENCEMISSOURI1, (vector2){0,0});
    fsSprite(INDEPENDENCEMISSOURI2, (vector2){136,0});
    gfx_SetColor(3);
    gfx_FillRectangle(57, 198, 183, 25);
    gfx_SetTextFGColor(2);
    gfx_PrintStringXY("Independence", 104, 201);
    gfx_PrintStringXY(formatDate(), 101, 212);
    gfx_SetTextFGColor(2);
    gfx_SetColor(3);
    while (os_GetCSC() != sk_Enter) {}
    gfx_FillScreen(2);
    sizeUp();
    
}
void printPrice(char* name, double price, vector2 pos, int currentAmt) {
    char buffer[100];
    sprintf(buffer, "$%01.2f per %s (%d)", price, name, currentAmt);
    gfx_PrintStringXY(buffer, pos.x, pos.y);
}
double sumBill(struct bill b, struct bill prices) {
    return (b.Ammunition*prices.Ammunition)+(b.axles*prices.axles)+(b.Clothing*prices.Clothing)+(b.Food*prices.Food)+(b.Oxen*prices.Oxen)+(b.tongues*prices.tongues)+(b.wheels*prices.wheels);
}
void shopPrompt(bool isMatts) {
    if (isMatts) {
        renderMattsShop();
    }
    else {
        currentBill = (struct bill){
            .Ammunition = 0,
            .axles = 0,
            .Clothing = 0,
            .Food = 0,
            .Oxen = 0,
            .tongues = 0,
            .wheels = 0
        };
        double markup = 1;
        switch (nextLandmark.index) {
            case 2: {markup = 1.25; break;}
            case 4: {markup = 1.5; break;}
            case 7: {markup = 1.75; break;}
            case 9: {markup = 2; break;}
            case 11: {markup = 2.25; break;}
            case 13: {markup = 2.5; break;}
            case 14: {markup = 3; break;}
        }
        struct bill prices = (struct bill){
            .Ammunition = 2*markup,
            .axles = 10*markup,
            .Clothing = 10*markup,
            .Food = 0.2*markup,
            .Oxen = 10*markup,
            .wheels = 10*markup,
            .tongues = 10*markup,
        };
        while (1==1) {
            gfx_FillScreen(2);
            printPrice("ox", prices.Oxen, (vector2){180,119}, currentBill.Oxen);
            printPrice("set", prices.Clothing, (vector2){180,127}, currentBill.Clothing);
            printPrice("box", prices.Ammunition, (vector2){180,135}, currentBill.Ammunition);
            printPrice("wheel", prices.wheels, (vector2){180,143}, currentBill.wheels);
            printPrice("axle", prices.axles, (vector2){180,151}, currentBill.axles);
            printPrice("tongue", prices.tongues, (vector2){180, 159}, currentBill.tongues);
            printPrice("pound", prices.Food, (vector2){180, 167}, currentBill.Food);
            int choice = newNumberedPromptStruct(npp[NPROMPT_BUY]);
            gfx_SetColor(2);
            int amt = 0;
            if (choice != 8) {
                gfx_FillRectangle(npp[NPROMPT_BUY].choiceDest.x,npp[NPROMPT_BUY].choiceDest.y, 128, 8);
                amt = newUserNumberPrompt("",(vector2){0,0},"How many?",npp[NPROMPT_BUY].choiceDest,1,2,NULL);
            }

            struct bill simBill = (struct bill){
                .Ammunition = currentBill.Ammunition,
                .axles = currentBill.axles,
                .Clothing = currentBill.Clothing,
                .Food = currentBill.Food,
                .Oxen = currentBill.Oxen,
                .tongues = currentBill.tongues,
                .wheels = currentBill.wheels
            };
            bool doBreak = false;
            switch (choice) {
                case 1: {
                    simBill.Oxen = amt;
                    if (sumBill(simBill, prices) <= globalGameState.money) {
                        currentBill.Oxen = amt;
                    }
                    break;
                }
                case 2: {
                    simBill.Clothing = amt;
                    if (sumBill(simBill, prices) <= globalGameState.money) {
                        currentBill.Clothing = amt;
                    }
                    break;
                }
                case 3: {
                    simBill.Ammunition = amt;
                    if (sumBill(simBill, prices) <= globalGameState.money) {
                        currentBill.Ammunition = amt;
                    }
                    break;
                }
                case 4: {
                    simBill.wheels = amt;
                    if (sumBill(simBill, prices) <= globalGameState.money) {
                        currentBill.wheels = amt;
                    }
                    break;
                }
                case 5: {
                    simBill.axles = amt;
                    if (sumBill(simBill, prices) <= globalGameState.money) {
                        currentBill.axles = amt;
                    }
                    break;
                }
                case 6: {
                    simBill.tongues = amt;
                    if (sumBill(simBill, prices) <= globalGameState.money) {
                        currentBill.tongues = amt;
                    }
                    break;
                }
                case 7: {
                    simBill.Food = amt;
                    if (sumBill(simBill, prices) <= globalGameState.money) {
                        currentBill.Food = amt;
                    }
                    break;
                }
                case 8: {
                    doBreak = true;
                    break;
                }
            }
            if (doBreak) {
                globalGameState.money -= sumBill(currentBill, prices);
                globalGameState.ammunitions += currentBill.Ammunition;
                globalGameState.axles += currentBill.axles;
                globalGameState.clothing += currentBill.Clothing;
                globalGameState.food += currentBill.Food;
                globalGameState.tongues += currentBill.tongues;
                globalGameState.wheels += currentBill.wheels;
                globalGameState.oxen += currentBill.Oxen;
                currentBill = (struct bill){
                    .Ammunition = 0,
                    .axles = 0,
                    .Clothing = 0,
                    .Food = 0,
                    .Oxen = 0,
                    .tongues = 0,
                    .wheels = 0
                };
                break;
            }
        }
    }
}
void monthChoiceMade() {
    gfx_FillScreen(2);
    globalGameState.year = 1848;
    fsSprite(FLAIR1, (vector2){12, 3});
    fsSprite(FLAIR2, (vector2){162, 3});
    fsSprite(FLAIR1, (vector2){12, 215});
    fsSprite(FLAIR2, (vector2){162, 215});
    newTextPromptStruct(tpp[TPROMPT_SHOPSTART], jobClrTxtZone);
    fsSprite(MATT, (vector2){0, 58});
    gfx_PrintStringXY("Hello, I'm Matt. So you're going to Oregon!", 28, 42);
    gfx_PrintStringXY("I can fix you up with what you need:", 28, 50);
    gfx_PrintStringXY("- a team of oxen to pull", 84, 91);
    gfx_PrintStringXY("your wagon", 100, 99);
    gfx_PrintStringXY("- clothing for both", 84, 124);
    gfx_PrintStringXY("summer & winter", 100, 132);
    while (os_GetCSC() != sk_Enter) { }
    jobClrTxtZone();
    fsSprite(MATT, (vector2){0, 58});
    gfx_PrintStringXY("Hello, I'm Matt. So you're going to Oregon!", 28, 42);
    gfx_PrintStringXY("I can fix you up with what you need:", 28, 50);
    gfx_PrintStringXY("- plenty of food for the trip", 84, 91);
    gfx_PrintStringXY("- ammunition for your rifles", 84, 124);
    gfx_PrintStringXY("- spare parts for your wagon", 84, 157);
    while (os_GetCSC() != sk_Enter) { }
    gfx_FillScreen(2);
    shopPrompt(true);
}

void initializePrompts() {

    // ---------- START ----------
    static promptOption startOptions[] = {
        {"Travel the trail"},
        {"Learn about the trail"}
    };
    npp[NPROMPT_START].p.options = NewList();
    for (int i = 0; i < 2; i++)
        List_AppendElement(&npp[NPROMPT_START].p.options, &startOptions[i]);

    // ---------- LEARN ----------
    static char* learnTexts[] = {
        "Try taking a journey by covered wagon across 2000 miles of plains, rivers, and mountains! Tr! On the plains, will you slosh your oxen through mud and water filled ruts or will you plod through dust six inches deep?",
        "How will you cross the rivers? If you have money, you might take a ferry. Or, you can ford the river and hope you and your wagon aren't swallowed alive!",
        "What about supplies? Well, if you're low on food you can hunt. You might get a buffalo . . . you might. And there are bears in the mountains.",
        "At the Dalles, you can navigate through the dangerous Barlow Road.",
        "If for some reason you don't survive -- your wagon burns, or thieves steal your oxen, or you run out of provisions, or die die of cholera -- don't give up! Try again . . . and again . . .",
        "Press 2nd twice at any prompt to quit.",
        "Original Oregon Trail team: Gratz, Kapplinger, Paquette, Phenow, Redland.",
        "TI-84 CE port by Gabriel Hollis. (NHProductions on Github)"
    };
    static List learnList;
    learnList = NewList();
    tpp[TPROMPT_LEARN].txtPrompts = &learnList;
    for (int i = 0; i < 8; i++)
        List_AppendElement(&learnList, learnTexts[i]);

    // ---------- JOBS ----------
    static promptOption jobOptions[] = {
        {"Be a banker from Boston"},
        {"Be a carpenter from Ohio"},
        {"Be a farmer from Illinois"},
        {"Find out the differences between these"}
    };
    npp[NPROMPT_JOBS].p.options = NewList();
    for (int i = 0; i < 4; i++)
        List_AppendElement(&npp[NPROMPT_JOBS].p.options, &jobOptions[i]);

    // ---------- JOB INFO ----------
    static char* jobInfoTexts[] = {
        "Travelling to Oregon isn't easy! But if you're a banker, you'll have more money for supplies and services than a carpenter or a farmer. However, farmers have an easier time finding food, & carpenters can repair wagon parts easier.",
        "However, the harder you have to try, the more points you deserve! Therefore, the farmer earns the greatest number of points & the banker earns the least"
    };
    static List jobInfoList;
    jobInfoList = NewList();
    tpp[TPROMPT_JOBINFO].txtPrompts = &jobInfoList;
    List_AppendElement(&jobInfoList, jobInfoTexts[0]);
    List_AppendElement(&jobInfoList, jobInfoTexts[1]);

    // ---------- MONTH ----------
    static promptOption monthOptions[] = {
        {"March"},{"April"},{"May"},{"June"},{"July"},{"Ask for advice"}
    };
    npp[NPROMPT_MONTH].p.options = NewList();
    for (int i = 0; i < 6; i++)
        List_AppendElement(&npp[NPROMPT_MONTH].p.options, &monthOptions[i]);

    // ---------- MONTH INFO ----------
    static char* monthInfoTexts[] = {
        "You attend a public meeting held for \" folds w/ the California-Oregon fever. \" You're told: If you leave too early, there won't be any grass. If you leave too late, winter will come. If you leave at the right time, there'll be grass & good weather."
    };
    static List monthInfoList;
    monthInfoList = NewList();
    tpp[TPROMPT_MONTHINFO].txtPrompts = &monthInfoList;
    List_AppendElement(&monthInfoList, monthInfoTexts[0]);

    // ---------- SHOP START ----------
    static char* shopStartTexts[] = {
        "Before leaving Independence you should buy supplies.",
        "You can buy what you need at Matt's General Store."
    };
    static List shopStartList;
    shopStartList = NewList();
    tpp[TPROMPT_SHOPSTART].txtPrompts = &shopStartList;
    for (int i = 0; i < 2; i++)
        List_AppendElement(&shopStartList, shopStartTexts[i]);

    // ---------- SHOP ----------
    static promptOption shopOptions[] = {
        {"Oxen"},{"Food"},{"Clothing"},{"Ammunition"},
        {"Spare Parts"},{"Leave Store"}
    };
    npp[NPROMPT_MATTSHOP].p.options = NewList();
    for (int i = 0; i < 6; i++)
        List_AppendElement(&npp[NPROMPT_MATTSHOP].p.options, &shopOptions[i]);

    // ---------- BUY ----------
    static promptOption buyOptions[] = {
        {"Oxen"},{"Food"},{"Clothing"},{"Ammunition"},
        {"Wagon Wheels"},{"Wagon Tongues"},{"Wagon Axles"},{"Leave Store"}
    };
    npp[NPROMPT_BUY].p.options = NewList();
    for (int i = 0; i < 8; i++)
        List_AppendElement(&npp[NPROMPT_BUY].p.options, &buyOptions[i]);

    // ---------- SIZEUP ----------
    static promptOption sizeupOptions[] = {
        {"Continue on trail"},
        {"Check Supplies"},
        {"Look at map"},
        {"Change pace"},
        {"Change food rations"},
        {"Stop to rest"},
        {"Attempt to trade"},
        {"Hunt for food"},
        {"Buy supplies"}
    };

    npp[NPROMPT_SIZEUP].p.options = NewList();
    npp[NPROMPT_FORTSIZEUP].p.options = NewList();

    for (int i = 0; i < 8; i++) {
        if (i != 7) {
            List_AppendElement(&npp[NPROMPT_SIZEUP].p.options, &sizeupOptions[i]);
            List_AppendElement(&npp[NPROMPT_FORTSIZEUP].p.options, &sizeupOptions[i]);
        }
        else {
            List_AppendElement(&npp[NPROMPT_SIZEUP].p.options, &sizeupOptions[7]);
            List_AppendElement(&npp[NPROMPT_FORTSIZEUP].p.options, &sizeupOptions[8]);
        }
    }

    // ---------- PACE ----------
    static promptOption paceOptions[] = {
        {"Steady - 8hr/day"},
        {"Strenuous - 12hr/day"},
        {"Grueling - 16hr/day"}
    };
    npp[NPROMPT_PACE].p.options = NewList();
    for (int i = 0; i < 3; i++)
        List_AppendElement(&npp[NPROMPT_PACE].p.options, &paceOptions[i]);

    // ---------- RATIONS ----------
    static promptOption rationOptions[] = {
        {"Filling"},{"Meager"},{"Bare Bones"}
    };
    npp[NPROMPT_RATIONS].p.options = NewList();
    for (int i = 0; i < 3; i++)
        List_AppendElement(&npp[NPROMPT_RATIONS].p.options, &rationOptions[i]);

    // ---------- RIVER ----------
    static promptOption riverOptions[] = {
        {"Ford river"},
        {"Caulk wagon"},
        {"Wait"},
        {"More info"},
        {"Take ferry"}
    };

    npp[NPROMPT_NORMALRIVER].p.options = NewList();
    npp[NPROMPT_FERRYRIVER].p.options = NewList();

    for (int i = 0; i < 5; i++) {
        List_AppendElement(&npp[NPROMPT_NORMALRIVER].p.options, &riverOptions[i]);
        List_AppendElement(&npp[NPROMPT_FERRYRIVER].p.options, &riverOptions[i]);
    }
}
int main() {
    srand(time(NULL));
    initializePrompts();
    gfx_Begin();
    gfx_SetPalette(global_palette, sizeof_global_palette, 0);
    while (1==1) {
        gfx_FillScreen(2);
        fsSprite(TITLE1, (vector2){7, 5});
        fsSprite(TITLE2, (vector2){152, 5});
        fsSprite(FLAIR1, (vector2){10, 41});
        fsSprite(FLAIR2, (vector2){159, 41});
        fsSprite(FLAIR1, (vector2){10, 220});
        fsSprite(FLAIR2, (vector2){159, 220});
        switch (newNumberedPromptStruct(npp[NPROMPT_START])) {
            case 1: {travelTrailPrompt(); break;};
            case 2: {learnPrompt(); break;};
        };
    }
    gfx_End();
    return 0;
}