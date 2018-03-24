#include "defines.h"
#include "errors.h"

#include "stack.h"
#include "parse.h"
#include "main.h"
#include "output.h"
#include "operator.h"
#include "functions.h"
#include "routines.h"

static const char *errors[] = {
    "This token/function is not implemented (yet)",
#ifdef CALCULATOR
    "This token cannot be used at the start of the   line",
#else
    "This token cannot be used at the start of the line",
#endif
    "This token doesn't have a condition",
    "You used 'Else' outside an If-statement",
    "You used 'End' outside a condition block",
    "You have an invalid \")\", \",\", \"(\", \")\", \"}\" or \"]\"",
    "You have an invalid expression",
    "Your icon should start with a quote",
    "Invalid hexadecimal",
    "ICE ERROR: please report it!",
    "You have the wrong number or arguments",
#ifdef CALCULATOR
    "Unknown C function. If you are sure this              function exists, please contact me!",
#else
    "Unknown C function",
#endif
    "Subprogram not found",
    "Compiling subprograms not supported",
    "Invalid program name",
    "Not enough memory for Lbl and Goto",
    "Error: not an ICE program\n",
    "Warning: Unknown char in the string!",
    "Warning: string has been automatically squish-ed!",
};

void displayLabelError(char *label) {
    char buf[30];

    sprintf(buf, "Label %s not found", label);
#ifdef COMPUTER_ICE
    fprintf(stdout, "%s\n", buf);
#elif defined(__EMSCRIPTEN__)
    ice_error(buf, 0);
#else
    gfx_SetTextFGColor(224);
    displayMessageLineScroll(buf);
#endif
}

void displayError(uint8_t index) {
#ifdef COMPUTER_ICE
    fprintf(stdout, "%s\n", errors[index]);
    fprintf(stdout, "Error at line %u\n", ice.currentLine);
#elif defined(__EMSCRIPTEN__)
    char buf[30];
    
    strcpy(buf, errors[index]);
    ice_error(buf, ice.currentLine);
#else
    char buf[30];

    gfx_SetTextFGColor(index < W_WRONG_CHAR ? 224 : 227);
    displayMessageLineScroll(errors[index]);

    gfx_SetTextFGColor(0);
    sprintf(buf, "Error at line %u", ice.currentLine);
    displayMessageLineScroll(buf);
#endif
}
