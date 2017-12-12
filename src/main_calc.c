#include "defines.h"
#include "main.h"

#if !defined(COMPUTER_ICE) && !defined(SC)

#include "functions.h"
#include "errors.h"
#include "stack.h"
#include "parse.h"
#include "output.h"
#include "operator.h"
#include "routines.h"

ice_t ice;
expr_t expr;
reg_t reg;

const char *infoStr = "ICE Compiler v2.0 - By Peter \"PT_\" Tillema";
extern label_t labelStack[150];
extern label_t gotoStack[150];

void main(void) {
    uint8_t selectedProgram = 0, key, amountOfPrograms, res = VALID;
    uint24_t programDataSize, offset, totalSize;
    const char ICEheader[] = {tii, 0};
    char buf[30], *var_name;
    void *search_pos = NULL;
    
    // Install hooks
    ti_CloseAll();
    ice.inPrgm = ti_Open("ICEAPPV", "r");
    
    if (ice.inPrgm) {
        ti_SetArchiveStatus(true, ice.inPrgm);
        ti_GetDataPtr(ice.inPrgm);
        
        // Manually set the hooks
        asm("ld de, 17");
        asm("add hl, de");
        asm("call 00213CCh");
        asm("ld de, 502");
        asm("add hl, de");
        asm("call 00213F8h");
        asm("ld de, 34");
        asm("add hl, de");
        asm("call 00213C4h");
        ti_Close(ice.inPrgm);
    }
    
    asm("ld iy, 0D00080h");
    asm("set 3, (iy+024h)");
    
    // Yay, GUI! :)
    gfx_Begin();

    gfx_SetColor(189);
    gfx_FillRectangle_NoClip(0, 0, 320, 10);
    gfx_SetColor(0);
    gfx_SetTextFGColor(0);
    gfx_HorizLine_NoClip(0, 10, 320);
    gfx_PrintStringXY(infoStr, 21, 1);
    
    // Get all the programs that start with the [i] token
    while((var_name = ti_DetectVar(&search_pos, ICEheader, TI_PRGM_TYPE)) && ++selectedProgram <= 22) {
        if (*var_name < 64) {
            *var_name += 64;
        }
        gfx_PrintStringXY(var_name, 10, selectedProgram*10 + 3);
    }
    amountOfPrograms = selectedProgram;
    
    // Check if there are ICE programs
    if (!amountOfPrograms) {
        gfx_PrintStringXY("No programs found!", 10, 13);
        goto stop;
    }
    
    // Select a program
    selectedProgram = 1;
    while ((key = os_GetCSC()) != sk_Enter & key != sk_2nd) {
        uint8_t selectionOffset = selectedProgram*10 + 3;

        gfx_PrintStringXY(">", 1, selectionOffset);

        if (key) {
            gfx_SetColor(255);
            gfx_FillRectangle_NoClip(1, selectionOffset, 8, 8);

            // Stop and quit
            if (key == sk_Clear) {
                goto err;
            }

            // Select the next program
            if (key == sk_Down) {
                if (selectedProgram != amountOfPrograms) {
                    selectedProgram++;
                } else {
                    selectedProgram = 1;
                }
            }
            
            // Select the previous program
            if (key == sk_Up) {
                if (selectedProgram != 1) {
                    selectedProgram--;
                } else {
                    selectedProgram = amountOfPrograms;
                }
            }
        }
    }
    
    // Erase screen
    gfx_SetColor(255);
    gfx_FillRectangle_NoClip(0, 11, 320, 229);
    
    // Grab the right program
    search_pos = NULL;
    while(((var_name = ti_DetectVar(&search_pos, ICEheader, TI_PRGM_TYPE)) != NULL) && --selectedProgram);
    
    gfx_SetTextXY(1, 12);
    displayMessageLineScroll("Prescanning...");
    displayLoadingBarFrame();
    
    ice.inPrgm = _open(var_name);
    _seek(0, SEEK_END, ice.inPrgm);
    ice.programLength = _tell(ice.inPrgm);
    ice.programData    = (uint8_t*)0xD52C00;
    strcpy(ice.currProgName[ice.inPrgm], var_name);

    ice.programPtr     = ice.programData + SIZEOF_CHEADER;
    ice.programDataPtr = ice.programDataData;
    ice.LblPtr         = ice.LblStack;
    ice.GotoPtr        = ice.GotoStack;
    ice.CBaseAddress   = ice.programPtr;
    
    // Pre-scan program (and subprograms) and find all the GRAPHX routines
    memcpy(ice.programData, CheaderData, SIZEOF_CHEADER);
    preScanProgram(ice.GraphxRoutinesStack, &ice.amountOfGraphxRoutinesUsed, true);
    
    // If there are no GRAPHX functions, remove the GRAPHX header
    if (!ice.amountOfGraphxRoutinesUsed) {
        ice.programPtr -= 9;
        ice.CBaseAddress -= 9;
    }
    
    // Prescan the program again to detect all the FILEIOC routines
    memcpy(ice.programPtr, FileiocheaderData, 10);
    ice.programPtr += 10;
    preScanProgram(ice.FileiocRoutinesStack, &ice.amountOfFileiocRoutinesUsed, false);
    
    // If there are no GRAPHX functions, remove the GRAPHX header
    if (!ice.amountOfFileiocRoutinesUsed) {
        ice.programPtr -= 10;
        
        // No C function at all
        if (!ice.amountOfGraphxRoutinesUsed) {
            ice.programPtr = ice.programData;
        }
    }
    
    ice.CBaseAddress -= ice.programData - (uint8_t*)PRGM_START;
    
    // Clear up program before and after running
    if (ice.amountOfGraphxRoutinesUsed || ice.amountOfFileiocRoutinesUsed) {
        CALL(_RunIndicOff);
        CALL(ice.programPtr - ice.programData + (ice.amountOfGraphxRoutinesUsed ? 12 : 9) + PRGM_START);
        LD_IY_IMM(flags);
        if (ice.amountOfGraphxRoutinesUsed) {
            JP(_DrawStatusBar);
        } else {
            RET();
        }
    }
    
    // Sorry :3
    ice.freeMemoryPtr = (ice.tempStrings[1] = (ice.tempStrings[0] = pixelShadow + 2000 * ice.amountOfOSLocationsUsed) + 2000) + 2000;
    
    LD_IX_IMM(IX_VARIABLES);
   
    // Do the stuff
    if (*var_name < 64) {
        *var_name += 64;
    }
    sprintf(buf, "Compiling program %s...", var_name);
    displayMessageLineScroll(buf);
    res = parseProgram();
    
    // Create or empty the output program if parsing succeeded
    if (res == VALID) {
        uint8_t currentGoto, currentLbl;
        uint24_t previousSize = 0;
        
        // If the last token is not "Return", write a "ret" to the program
        if (!ice.lastTokenIsReturn) {
            RET();
        }
        
        // Find all the matching Goto's/Lbl's
        for (currentGoto = 0; currentGoto < ice.amountOfGotos; currentGoto++) {
            label_t *curGoto = &gotoStack[currentGoto];
            
            for (currentLbl = 0; currentLbl < ice.amountOfLbls; currentLbl++) {
                label_t *curLbl = &labelStack[currentLbl];
                
                if (!memcmp(curLbl->name, curGoto->name, 10)) {
                    w24((uint8_t*)(curGoto->addr + 1), curLbl->addr - (uint24_t)ice.programData + PRGM_START);
                    goto findNextLabel;
                }
            }
            
            // Label not found
            displayLabelError(curGoto->name);
            _seek(curGoto->offset, SEEK_SET, ice.inPrgm);
            res = 0;
            goto stop;
findNextLabel:;
        }
        
        // Get the sizes of both stacks
        ice.programSize = (uintptr_t)ice.programPtr - (uintptr_t)ice.programData;
        programDataSize = (uintptr_t)ice.programDataPtr - (uintptr_t)ice.programDataData;
        
        // Change the pointers to the data as well, but first calculate the offset
        offset = PRGM_START + ice.programSize - (uintptr_t)ice.programDataData;
        while (ice.dataOffsetElements--) {
            uint24_t *tempDataOffsetStackPtr = ice.dataOffsetStack[ice.dataOffsetElements];
            
            *tempDataOffsetStackPtr += offset;
        }
        totalSize = ice.programSize + programDataSize + 3;
        
        if (ice.startedGRAPHX && !ice.endedGRAPHX) {
            displayError(W_CLOSE_GRAPHX);
        }
        
        // Export the program
        ice.outPrgm = _open(ice.outName);
        if (ice.outPrgm) {
            previousSize = ti_GetSize(ice.outPrgm);
            ti_Close(ice.outPrgm);
        }
        ice.outPrgm = _new(ice.outName);
        if (!ice.outPrgm) {
            displayMessageLineScroll("Failed to open output file");
            goto stop;
        }
        
        // Write ASM header
        ti_PutC(tExtTok, ice.outPrgm);
        ti_PutC(tAsm84CeCmp, ice.outPrgm);
        
        // Write ICE header to be recognized by Cesium
        ti_PutC(0x7F, ice.outPrgm);
        
        // Write the header, main program, and data to output :D
        ti_Write(ice.programData, ice.programSize, 1, ice.outPrgm);
        if (programDataSize) ti_Write(ice.programDataData, programDataSize, 1, ice.outPrgm);
        
        // Yep, we are really done!
        gfx_SetTextFGColor(4);
        displayMessageLineScroll("Succesfully compiled!");
        
        // Skip line
        displayMessageLineScroll(" ");
        
        // Display the size
        gfx_SetTextFGColor(0);
        sprintf(buf, "Output size: %u bytes", totalSize);
        displayMessageLineScroll(buf);
        if (previousSize) {
            sprintf(buf, "Previous size: %u bytes", previousSize);
            displayMessageLineScroll(buf);
        }
    } else {
        displayError(res);
    }
    
stop:
    gfx_SetTextFGColor(0);
    gfx_PrintStringXY("[Press any key to quit]", 85, 230);
    while (!os_GetCSC());
err:
    // Return immediately
    gfx_End();
    prgm_CleanUp();
    if (res != VALID && !ti_IsArchived(ice.inPrgm)) {
        GotoEditor(ice.currProgName[ice.inPrgm], ti_Tell(ice.inPrgm) - 1);
    }
    ti_CloseAll();
}

void preScanProgram(uint24_t CFunctionsStack[], uint8_t *CFunctionsCounter, bool detectOSVars) {
    int token;
    
    _rewind(ice.inPrgm);
    
    // Scan the entire program
    while ((int)(token = _getc()) != EOF) {
        uint8_t tok = (uint8_t)token;
        
        if (tok == tString) {
            expr.inString = !expr.inString;
        } else if (tok == tEnter) {
            expr.inString = false;
        } else if (tok == tii && !expr.inString) {
            skipLine();
        } else if (tok == tStore) {
            expr.inString = false;
        } else if (tok == tVarLst && !expr.inString && detectOSVars) {
            if (!ice.OSLists[token = _getc()]) {
                ice.OSLists[token] = pixelShadow + 2000 * (ice.amountOfOSLocationsUsed++);
            }
        } else if (tok == tVarStrng && !expr.inString && detectOSVars) {
            if (!ice.OSStrings[token = _getc()]) {
                ice.OSStrings[token] = pixelShadow + 2000 * (ice.amountOfOSLocationsUsed++);
            }
        } else if (tok == t2ByteTok && !expr.inString) {
            // AsmComp(
            if ((uint8_t)_getc() == tAsmComp) {
                char tempName[9] = {0};
                uint8_t a = 0;
                ti_var_t tempProg = ice.inPrgm;

                while ((int)(token = _getc()) != EOF && (tok = (uint8_t)token) != tEnter && a < 9) {
                    tempName[a++] = tok;
                }
                tempName[a] = 0;
                
                if ((ice.inPrgm = _open(tempName))) {
                    preScanProgram(CFunctionsStack, CFunctionsCounter, detectOSVars);
                }
                _close(ice.inPrgm);
                ice.inPrgm = tempProg;
            }
        } else if (((tok == tDet && detectOSVars) || (tok == tSum && !detectOSVars)) && !expr.inString) {
            uint8_t tok1 = _getc();
            uint8_t tok2 = _getc();

            // Invalid det( command
            if (tok1 < t0 || tok1 > t9) {
                break;
            }
            
            // Get the det( command
            if (tok2 < t0 || tok2 > t9) {
                tok = tok1 - t0;
            } else {
                tok = (tok1 - t0) * 10 + (tok2 - t0);
            }
            
            // Insert the C routine
            if (!CFunctionsStack[tok]) {
                CFunctionsStack[tok] = ice.programPtr - ice.CBaseAddress;
                JP(tok * 3);
                (*CFunctionsCounter)++;
            }
        }
    }
    
    _rewind(ice.inPrgm);
}

#endif
