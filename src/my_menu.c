#include "my_menu.h"
#include <string.h>

void InitMenu(Menu *M, MenuWindow *startMW, Display *d){
    M->currWindow = M->menuWindows[0] = startMW;
    M->WindowNum = 1;
    M->display = d;
}

void addMenuWindow(Menu *M, MenuWindow *MW){
    if (M->WindowNum >= MAX_WINDOW_NUM-1) return; // overflow
    M->menuWindows[M->WindowNum] = MW;
    M->WindowNum++;
}

void next(Menu *M){
    nextItem(M->currWindow, M->display);
}

void right(Menu *M){
    nextItem(M->currWindow, M->display);
}

void left(Menu *M){
    previousItem(M->currWindow, M->display);
}

void down(Menu *M){
    lowerItem(M->currWindow, M->display);
}

void up(Menu *M){
    upperItem(M->currWindow, M->display);
}

int enter(Menu *M){
    MenuWindow* nW = enterItem(M->currWindow, M->display);
    if (nW != NULL){
        M->currWindow = nW;
        clear(M->display);
        printWindow(M->currWindow, M->display);
        return M->currWindow->state;
    }
    else return -1;
}

void startMenu(Menu *M){
    clear(M->display);
    printWindow(M->currWindow, M->display);
}

int getSelectNum(Menu *M){
    return M->currWindow->selectedItemNum;
}

int getState(Menu *M){
    return M->currWindow->state;
}

void reprintCustom(Menu *M){
    printCustom(M->currWindow, M->display);
}
/******************************************************************/
void InitMenuWindow(MenuWindow *MW, int st, MenuWindow *pMW, uint8_t primary){
    MW->state = st;
    MW->prevMW = pMW;
    
    MW->currItemNum = 0;
    int i;
    for(i = 0; i<MAX_WINDOW_ITEM_NUM; i++) MW->menuItems[i] = NULL;
    if (primary == 0){
        char b[2] = {0xC4,'\0'};
        InitMenuItem(&(MW->back), MW->prevMW, 14, 1, b);
        addMenuItem(MW, &(MW->back));
    }
    MW->primaryWidnow = primary;

    setItemText(&(MW->windowText[0]), NULL);
    setItemText(&(MW->windowText[1]), NULL);

    MW->customPrintPtr[0] = MW->customPrintPtr[1] = NULL;

    MW->radioWindow = 0;
    MW->selectedItemNum = 0;
}

void addMenuItem(MenuWindow *MW, MenuItem *item){
    if (MW->ItemNum >= MAX_WINDOW_ITEM_NUM-1) return; // overflow 
    if ((MW->ItemNum > 0) && (MW->primaryWidnow == 0))
    {
        MW->menuItems[MW->ItemNum] = MW->menuItems[MW->ItemNum-1]; //put back at the end
        MW->menuItems[MW->ItemNum-1] = item;
    }
    else {
        MW->menuItems[MW->ItemNum] = item;
    }
    MW->ItemNum++;
    item->myMW = MW;
}

void nextItem(MenuWindow *MW, Display *d){
    if (MW->ItemNum == 1) return;
    eraseCursor(MW->menuItems[MW->currItemNum], d);
    MW->currItemNum = (MW->currItemNum + 1) % MW->ItemNum;
    printCursor(MW->menuItems[MW->currItemNum], d);
}

void previousItem(MenuWindow *MW, Display *d){
    if (MW->ItemNum == 1) return;
    eraseCursor(MW->menuItems[MW->currItemNum], d);
    MW->currItemNum = (MW->currItemNum - 1 + MW->ItemNum) % MW->ItemNum;
    printCursor(MW->menuItems[MW->currItemNum], d);
}

void lowerItem(MenuWindow *MW, Display *d){
    if (MW->ItemNum == 1) return;
    eraseCursor(MW->menuItems[MW->currItemNum], d);
    /*********************************************/
    int i, cnt = 0, newNum = (MW->currItemNum + 1) % MW->ItemNum;
    int oldNum = MW->currItemNum;
    int newRow, curRow = MW->menuItems[MW->currItemNum]->posRow;
    newRow = curRow;
    int dd, minD = COL_NUM * COL_NUM;

    for(i = newNum; ;i = (i+1) % MW->ItemNum){
        if(cnt >= MW->ItemNum)
        {
            newNum = oldNum;
            break;
        }
        if(newRow == curRow) // 1. find next non-empty row number
        {
            if(MW->menuItems[i]->posRow != curRow)
            {
                newRow = MW->menuItems[i]->posRow;
                i = (i - 1 + MW->ItemNum) % MW->ItemNum; // return i for the next step
            }
            cnt++;
        }
        else{ // 2. find lowest deviation in row
            if(MW->menuItems[i]->posRow != newRow){
                break; // row change: end of item search
            }
            dd = MW->menuItems[MW->currItemNum]->posCol - 
                MW->menuItems[i]->posCol;
            dd *= dd;
            if(dd < minD){
                minD = dd;
                newNum = i;
            }
        }
    }
    MW->currItemNum = newNum;
    /*********************************************/
    printCursor(MW->menuItems[MW->currItemNum], d);
}

void upperItem(MenuWindow *MW, Display *d){
    if (MW->ItemNum == 1) return;
    eraseCursor(MW->menuItems[MW->currItemNum], d);
    /*********************************************/
    int i, cnt = 0, newNum = (MW->currItemNum - 1 + MW->ItemNum) % MW->ItemNum;
    int oldNum = MW->currItemNum;
    int newRow, curRow = MW->menuItems[MW->currItemNum]->posRow;
    newRow = curRow;
    int dd, minD = COL_NUM * COL_NUM;

    for(i = newNum; ;i = (i - 1 + MW->ItemNum) % MW->ItemNum){
        if(cnt >= MW->ItemNum) // check if there is only one row
        {
            newNum = oldNum;
            break;
        }
        if(newRow == curRow) // 1. find next non-empty row number
        {
            if(MW->menuItems[i]->posRow != curRow)
            {
                newRow = MW->menuItems[i]->posRow;
                i = (i + 1) % MW->ItemNum; // return i for the next step
            }
            cnt++;
        }
        else{ // 2. find lowest deviation in row
            if(MW->menuItems[i]->posRow != newRow){
                break; // row change: end of item search
            }
            dd = MW->menuItems[MW->currItemNum]->posCol - 
                MW->menuItems[i]->posCol;
            dd *= dd;
            if(dd < minD){
                minD = dd;
                newNum = i;
            }
        }
    }
    MW->currItemNum = newNum;
    /*********************************************/
    printCursor(MW->menuItems[MW->currItemNum], d);
}

MenuWindow* enterItem(MenuWindow *MW, Display *d){
    if ((MW->radioWindow != 1) || (MW->currItemNum) == (MW->ItemNum-1)) {
        MenuWindow* nextWindow = getNextWindow(MW->menuItems[MW->currItemNum]);
        if (((MW->currItemNum) == (MW->ItemNum-1)) && (MW->primaryWidnow != 1)) 
            MW->currItemNum = MW->selectedItemNum;
        return nextWindow;
    }
    else {
        int old = MW->selectedItemNum;
        MW->selectedItemNum = MW->currItemNum;
        writeSelection(MW->menuItems[MW->selectedItemNum], d);
        writeSelection(MW->menuItems[old], d);
        return NULL;
    }
}

void printWindow(MenuWindow *MW, Display *d){
    int i;
    /* print window text */
    if (checkItemText(&(MW->windowText[0])) != 0) printItem(&(MW->windowText[0]), d);
    if (checkItemText(&(MW->windowText[1])) != 0) printItem(&(MW->windowText[1]), d);
    /* custom print */
    printCustom(MW, d);
    /* print window items */
    for (i = 0; i<MW->ItemNum; i++){
        printItem(MW->menuItems[i], d);
    }
    printCursor(MW->menuItems[MW->currItemNum], d);
}

void setWindowText(MenuWindow *MW, uint8_t c, uint8_t r, char *text){
    if (r == 0){
        InitMenuItem(&(MW->windowText[0]), NULL, c-1, r, text);
    }
    else{
        InitMenuItem(&(MW->windowText[1]), NULL, c-1, r, text);
    }
}

void updateWindowText(MenuWindow *MW, uint8_t r, char *text){
    if (r == 0){
        setItemText(&(MW->windowText[0]), text);
    }
    else{
        setItemText(&(MW->windowText[1]), text);
    }
}

void setAsRadioWindow(MenuWindow *MW, int select){
    MW->radioWindow = 1;
    if (select < 0) select = 0;
    MW->currItemNum = MW->selectedItemNum = select;
}

MenuItem* selectedItem(MenuWindow *MW){
    return MW->menuItems[MW->selectedItemNum];
}

uint8_t isBack(MenuWindow *MW, MenuItem *MI){
    if (MI == MW->menuItems[MW->ItemNum-1]) return 1;
    else return 0;
}

void setCustomPrint(MenuWindow *MW, uint8_t c, uint8_t r, void (*printFunc)(Display *)){
    if (r == 0){
        MW->customPrintPtr[0] = printFunc;
        MW->customPosCol[0] = c;
        MW->customPosRow[0] = r;
    }
    else{
        MW->customPrintPtr[1] = printFunc;
        MW->customPosCol[1] = c;
        MW->customPosRow[1] = r;
    }
}

void printCustom(MenuWindow *MW, Display *d){
    if (MW->customPrintPtr[0] != NULL){
        setCursor(d, MW->customPosCol[0], MW->customPosRow[0]);
        MW->customPrintPtr[0](d);
    }
    if (MW->customPrintPtr[1] != NULL){
        setCursor(d, MW->customPosCol[1], MW->customPosRow[1]);
        MW->customPrintPtr[1](d);
    }
}
/******************************************************************/
void InitMenuItem(MenuItem *MI, MenuWindow *nMW, uint8_t c, uint8_t r, char *l){
    MI->nextMW = nMW;

    MI->posCol = c;
    MI->posRow = r;

    setItemText(MI, l);
}

void printCursor(MenuItem *MI, Display *d){
    setCursor(d, MI->posCol, MI->posRow);
    write(d, 0xF6);
    setCursor(d, MI->posCol, MI->posRow);
}

void eraseCursor(MenuItem *MI, Display *d){
    setCursor(d, MI->posCol, MI->posRow);
    write(d, ' ');
}

MenuWindow* getNextWindow(MenuItem *MI){
    return MI->nextMW;
}

void printItem(MenuItem *MI, Display *d){
    setCursor(d, MI->posCol+1, MI->posRow);
    
    if (!isBack(MI->myMW,MI)) writeSelection(MI, d);

    printStr(d, MI->label);
}

void setItemText(MenuItem *MI, char *l){
    if (l == NULL){
        MI->label[0] = '\0';
    }
    else{
        int i;
        for(i = 0; l[i] != '\0'; i++){
            MI->label[i] = l[i];
        }
        MI->label[i] = '\0';
    }
}

uint8_t checkItemText(MenuItem *MI){
    if (MI == 0) return 0;
    if (MI->label == 0) return 0;
    return strlen(MI->label);
}

void writeSelection(MenuItem *MI, Display *d){
    setCursor(d, MI->posCol+1, MI->posRow);
    if ((MI->myMW->radioWindow) == 1)
    {
       if (MI == selectedItem(MI->myMW)) write(d, selectChar);
       else write(d, unselectChar);
    }
}
