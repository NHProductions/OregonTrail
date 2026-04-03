#include <graphx.h>
#include "prompt.h"
#include "Tundora.h"
#include "vector2.h"
#include <keypadc.h>
#include <sys/timers.h>
#include <debug.h>
#include <ti/getcsc.h>
#include <stdlib.h>
#define TYPEDELAY 30
char* TYPECHARS = "\0\0\0\0\0\0\0\0\0\0\0WRMH\0\0\0\0VQLG\0\0\0ZUPKFC\0\0YTOJEB\0\0XSNIDA\0\0\0\0\0\0\0";
#define BLACK gfx_RGBTo1555(0,0,0)
#define WHITE gfx_RGBTo1555(255, 255, 255)
int getNumberKeyPress(int min, int max) {
    kb_Scan();
    int current = -1;
    if (kb_Data[3] & kb_1) {
        current = 1;
    }
    else if (kb_Data[3] & kb_4) {
        current = 4;
    }
    else if (kb_Data[3] & kb_7) {
        current = 7;
    }
    else if (kb_Data[4] & kb_2) {
        current = 2;
    }
    else if (kb_Data[4] & kb_5) {
        current = 5;
    }
    else if (kb_Data[4] & kb_8) {
        current = 8;
    }
    else if (kb_Data[5] & kb_3) {
        current = 3;
    }
    else if (kb_Data[5] & kb_6) {
        current = 6;
    }
    else if (kb_Data[5] & kb_9) {
        current = 9;
    }
    else if (kb_Data[3] & kb_0) {
        current = 0;
    }

    if (current >= min && current <= max) {
        return current;
    }
    return -1;
}
int newNumberedPrompt(prompt p, vector2 titleStart, vector2 optionsStart, vector2 choiceDest, char* choiceText) {
    int exitAmt = 0;
    gfx_SetTextFGColor(3);
    // Render title text & option texts onto the screen.
    gfx_PrintStringXY(p.title, titleStart.x, titleStart.y);
    gfx_PrintStringXY(choiceText, choiceDest.x, choiceDest.y);
    char temp[4];
    for (int i = 0; i < p.options.length; i++) {
        promptOption* op = (promptOption*)(List_GetElement(&p.options, i)->data);
        gfx_PrintStringXY(op->text, optionsStart.x+24, optionsStart.y+(8*i));
        // Prints out the "x." like in actual oregon trail.
        sprintf(temp, "%d. ", i+1);
        gfx_PrintStringXY(temp, optionsStart.x, optionsStart.y+(8*i));
        temp[0] = ' ';
        temp[1] = ' ';
        temp[2] = ' ';
        temp[3] = ' ';
    }
    // Wait until the user selects a valid option.
    int currentChoice = -1;
    kb_Scan();
    char numTemp[2];
    gfx_SetColor(2);
    while ( 1==1) {
        msleep(TYPEDELAY);
        int newChoice = getNumberKeyPress(1, p.options.length);
        if (newChoice != currentChoice && newChoice != -1) {
            sprintf(numTemp, "%d", newChoice);
            gfx_FillRectangle(choiceDest.x+(7*strlen(choiceText)), choiceDest.y, 8, 8);
            gfx_PrintStringXY(numTemp, choiceDest.x+(7*strlen(choiceText)), choiceDest.y);
            numTemp[0] = ' ';
            numTemp[1] = ' ';
        }
        currentChoice = (newChoice != currentChoice && newChoice != -1) ? newChoice : currentChoice;
        if (kb_Data[6] & kb_Enter) {
            if (currentChoice != -1) break;
        }
        if (kb_Data[1] & kb_2nd) {
            exitAmt++;
            if (exitAmt == 2) exit(0);
        }
    }
    return currentChoice;
}   
int newNumberedPromptStruct(numberPromptParams n) {
    return newNumberedPrompt(n.p, n.titleStart, n.optionsStart, n.choiceDest, n.choiceText);
}
List* splitStrLines(char* ogstr, int lnSize) {
    List splitBySpaces = NewList();
    char *token;
    char *str = strdup(ogstr);
    token = strtok(str, " ");
    while (token != NULL) {
        char* toAdd = malloc(strlen(token) + 1);
        strcpy(toAdd, token);
        List_AppendElement(&splitBySpaces, toAdd);
        token = strtok(NULL, " ");
    }
    free(str);
    List* toDisp = malloc(sizeof(List));
    *toDisp = NewList();
    int currentSum = 0;
    char* currentLine = malloc(lnSize+1);
    currentLine[0] = '\0';
    for (int i = 0; i < splitBySpaces.length; i++) {
        char* currentToken = (char*)List_GetElement(&splitBySpaces, i)->data;
        if (strlen(currentToken) + currentSum + 1 > lnSize) {
            char* toAlloc = malloc(lnSize+1);
            strcpy(toAlloc, currentLine);
            List_AppendElement(toDisp, toAlloc);
            currentSum = 0;
            currentLine[0] = '\0';
            i -= 1;
        }
        else {
            strcat(currentLine, currentToken);
            strcat(currentLine, " ");
            currentSum += strlen(currentToken) + 1;
        } 
    }
    if (currentSum > 0) {
        char* toAlloc = malloc(strlen(currentLine) + 1);
        strcpy(toAlloc, currentLine);
        List_AppendElement(toDisp, toAlloc);
        free(currentLine);
    }
    else {
        free(currentLine);
    }
    List_Free_List(&splitBySpaces);
    return toDisp;

}
void newTextPrompt(List *p, vector2 startPos, int cpl, bool spaceToContinueDisclaimer, promptFunc pf) {
    dbg_printf("Prompt:");
    dbg_printf("\nPrompt: %s\n", (char*)List_GetElement(p, -1)->data);
    int exitAmt = 0;
    for (int i = 0; i < p->length; i++) {
        
        char* text = (char*)List_GetElement(p, i)->data;
        List* textSpaces = splitStrLines(text, cpl);
        gfx_SetTextFGColor(3); 
        for (int j = 0; j < textSpaces->length; j++) {
            char* txtPart = (char*)List_GetElement(textSpaces, j)->data;
            dbg_printf("Line %d/%d:  %s", j, textSpaces->length, txtPart);
            gfx_PrintStringXY(txtPart, startPos.x, startPos.y+(j*8));
        }
        if (spaceToContinueDisclaimer) gfx_PrintStringXY("Press ENTER to continue", 61, 213);
        while (1==1) {
            msleep(TYPEDELAY);
            kb_Scan();
            if (kb_Data[6] & kb_Enter) break;
            if (kb_Data[1] & kb_2nd) {
                exitAmt++;
                if (exitAmt == 2) {
                    exit(0);
                }
            }
        }
        pf();
        List_Free_List(textSpaces);
    }
    
}
void newTextPromptStruct(textPromptParams t, promptFunc p) {
    newTextPrompt(t.txtPrompts, t.startPos, t.charsPerLine, t.spaceToContinueDisclaimer, p);
}
char* newUserTextPrompt(char* txtPrompt, vector2 position, int maxChars, promptFunc p, bool allowSpaces) {
    char* charBuffer = malloc(maxChars+1);
    charBuffer[0] = '\0';
    uint8_t key;
    gfx_SetColor(3);
    gfx_PrintStringXY(txtPrompt, position.x, position.y);
    bool shift = true;
    int exitAmt = 0;
    while (1==1) {
        key = os_GetCSC();
        if (key == sk_Enter && strlen(charBuffer) > 0) break;
        if (key == sk_Alpha) shift = !shift;
        if (TYPECHARS[key] && strlen(charBuffer) < maxChars && key != sk_Add) {
            int len = strlen(charBuffer);
            charBuffer[len] = (TYPECHARS[key] ^ (shift ? 32 : 0));
            charBuffer[len+1] = '\0';
            gfx_SetColor(2);
            gfx_FillRectangle(position.x+(strlen(txtPrompt)*8),position.y,maxChars*8,8);
            gfx_SetColor(3);
            gfx_PrintStringXY(charBuffer, position.x+(strlen(txtPrompt)*8), position.y);
        }
        else if (strlen(charBuffer) < maxChars && key == sk_Add && allowSpaces) {
            int len = strlen(charBuffer);
            charBuffer[len] = ' ';
            charBuffer[len+1] = '\0';
            gfx_SetColor(2);
            gfx_FillRectangle(position.x+(strlen(txtPrompt)*8),position.y,maxChars*8,8);
            gfx_SetColor(3);
            gfx_PrintStringXY(charBuffer, position.x+(strlen(txtPrompt)*8), position.y);
        }
        if (key == sk_Del && strlen(charBuffer) > 1) {
            charBuffer[strlen(charBuffer)-2] = '\0';
            
            dbg_printf("Del req; %s", charBuffer);
            gfx_SetColor(2);
            gfx_FillRectangle(position.x+(strlen(txtPrompt)*8),position.y,maxChars*8,8);
            gfx_SetColor(3);
            gfx_PrintStringXY(charBuffer, position.x+(strlen(txtPrompt)*8), position.y);
        }
        if (key == sk_2nd) {
            exitAmt++;
            if (exitAmt == 2) exit(0);
        }
    }
    if (p) p();
    return charBuffer;
}
bool newBoolPrompt(char* prompt, vector2 pos) {
    gfx_SetColor(3);
    gfx_PrintStringXY(prompt, pos.x, pos.y);
    int decision = -1;
    uint8_t key;
    int exitAmt = 0;
    while (1==1) {
        key = os_GetCSC();
        if (key == sk_Down) {
            decision = 0;
            gfx_SetColor(2);
            gfx_FillRectangle(pos.x+(strlen(prompt)*8), pos.y, 8, 8);
            gfx_SetColor(3);
            gfx_PrintStringXY("N", pos.x+(strlen(prompt)*8), pos.y);
        }
        else if (key == sk_Up) {
            decision = 1;
            gfx_SetColor(2);
            gfx_FillRectangle(pos.x+(strlen(prompt)*8), pos.y, 8, 8);
            gfx_SetColor(3);
            gfx_PrintStringXY("Y", pos.x+(strlen(prompt)*8), pos.y);
        }
        if (key == sk_Enter && decision != -1) return (bool)decision;
        if (key == sk_2nd) {
            exitAmt++;
            if (exitAmt == 2) exit(0);
        }
    }
}
int newUserNumberPrompt(char* txtPrompt, vector2 promptPosition, char* choiceText, vector2 choicePos, int minChar, int maxChar, promptFunc p) {
    gfx_SetColor(3);
    gfx_PrintStringXY(txtPrompt, promptPosition.x, promptPosition.y);
    gfx_PrintStringXY(choiceText, choicePos.x, choicePos.y);
    char* userChoice = malloc((maxChar+1));
    userChoice[0] = '\0';
    int currentlength = 0;
    int exitAmt = 0;
    while (1==1) {
        uint8_t key = os_GetCSC();
        int len = strlen(userChoice);
        if ( (key == sk_Enter) && len <= maxChar) break;
        if ( (key == sk_1) && len < maxChar) strcat(userChoice, "1");
        if ( (key == sk_2) && len < maxChar) strcat(userChoice, "2");
        if ( (key == sk_3) && len < maxChar) strcat(userChoice, "3");
        if ( (key == sk_4) && len < maxChar) strcat(userChoice, "4");
        if ( (key == sk_5) && len < maxChar) strcat(userChoice, "5");
        if ( (key == sk_6) && len < maxChar) strcat(userChoice, "6");
        if ( (key == sk_7) && len < maxChar) strcat(userChoice, "7");
        if ( (key == sk_8) && len < maxChar) strcat(userChoice, "8");
        if ( (key == sk_9) && len < maxChar) strcat(userChoice, "9");
        if ( (key == sk_0) && len < maxChar) strcat(userChoice, "0");
        if ( (key == sk_Del) && len >= minChar) userChoice[len - 1] = '\0';
        if (currentlength != len) {
            int newlen = strlen(choiceText);
            gfx_SetColor(2);
            gfx_FillRectangle(choicePos.x+(newlen*8), choicePos.y, maxChar*8,8);
            gfx_SetColor(3);
            gfx_PrintStringXY(userChoice, choicePos.x+(newlen*8), choicePos.y);
            currentlength = strlen(userChoice);
        }
        if (key == sk_2nd) {
            exitAmt++;
            if (exitAmt == 2) exit(0);
        }
    }
    int toReturn = atoi(userChoice);
    free(userChoice);
    return toReturn;    

}


