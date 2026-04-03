#ifndef PROMPT_H
#define PROMPT_H
#include "Tundora.h"
#include "vector2.h"

typedef void (*promptFunc)();
typedef struct {
    char* text;
} promptOption;
typedef struct {
    char* title;
    List options;
} prompt;
typedef struct {
    prompt p;
    vector2 titleStart;
    vector2 optionsStart;
    vector2 choiceDest;
    char* choiceText;
} numberPromptParams;
typedef struct {
    List* txtPrompts;
    vector2 startPos;
    int charsPerLine;
    bool spaceToContinueDisclaimer;
} textPromptParams;
int getNumberKeyPress(int min, int max);
int newNumberedPrompt(prompt p, vector2 titleStart, vector2 optionsStart, vector2 choiceDest, char* choiceText);
int newNumberedPromptStruct(numberPromptParams n);
List* splitStrLines(char* str, int lnSize);
void newTextPrompt(List* p, vector2 startPos, int cpl, bool spaceToContinueDisclaimer, promptFunc pf);
void newTextPromptStruct(textPromptParams t, promptFunc p);
char* newUserTextPrompt(char* txtPrompt, vector2 position, int maxChars, promptFunc p, bool allowSpace);
bool newBoolPrompt(char* prompt, vector2 pos);
int newUserNumberPrompt(char* txtPrompt, vector2 promptPosition, char* choiceText, vector2 choicePos, int minChar, int maxChar, promptFunc p);
#endif