#ifndef _my_menu_h_
#define _my_menu_h_

#include "my_display.h"

/* my constants */
#define MAX_WINDOW_NUM 32
#define MAX_WINDOW_ITEM_NUM 16
#define COL_NUM 16
#define ROW_NUM 2

#define selectChar   0x00
#define unselectChar 0x01

/* my structures */
typedef struct menu Menu;
typedef struct menu_window MenuWindow;
typedef struct menu_item MenuItem;

typedef struct menu
{
  Display *display;

  int WindowNum;
  MenuWindow *currWindow;
  MenuWindow *menuWindows[MAX_WINDOW_NUM];
} Menu;

typedef struct menu_item
{
  MenuWindow *myMW;
  MenuWindow *nextMW;
  
  uint8_t posCol;
  uint8_t posRow; 

  char label[16];
} MenuItem;

typedef struct menu_window
{
  int state;
  MenuWindow *prevMW;

  int currItemNum, ItemNum;
  MenuItem *menuItems[MAX_WINDOW_ITEM_NUM];
  MenuItem back;

  MenuItem windowText[2];

  void (*customPrintPtr[2])(Display *);
  uint8_t customPosCol[2];
  uint8_t customPosRow[2]; 

  uint8_t primaryWidnow;

  uint8_t radioWindow;
  int selectedItemNum;
} MenuWindow;

/* my functions */
void InitMenu(Menu *M, MenuWindow *startMW, Display *d);
void addMenuWindow(Menu *M, MenuWindow *MW);
void next(Menu *M);
void right(Menu *M);
void left(Menu *M);
void down(Menu *M);
void up(Menu *M);
int enter(Menu *M);
void startMenu(Menu *M);
int getSelectNum(Menu *M);
int getState(Menu *M);
void reprintCustom(Menu *M);

void InitMenuWindow(MenuWindow *MW, int st, MenuWindow *pMW, uint8_t primary);
void addMenuItem(MenuWindow *MW, MenuItem *item);
void nextItem(MenuWindow *MW, Display *d);
void previousItem(MenuWindow *MW, Display *d);
void lowerItem(MenuWindow *MW, Display *d);
void upperItem(MenuWindow *MW, Display *d);
MenuWindow* enterItem(MenuWindow *MW, Display *d);
void printWindow(MenuWindow *MW, Display *d);
void setWindowText(MenuWindow *MW, uint8_t c, uint8_t r, char *text);
void updateWindowText(MenuWindow *MW, uint8_t c, char *text);
void setAsRadioWindow(MenuWindow *MW, int select);
MenuItem* selectedItem(MenuWindow *MW);
uint8_t isBack(MenuWindow *MW, MenuItem *MI);
void setCustomPrint(MenuWindow *MW, uint8_t c, uint8_t r, void (*printFunc)(Display *));
void printCustom(MenuWindow *MW, Display *d);

void InitMenuItem(MenuItem *MI, MenuWindow *nMW, uint8_t c, uint8_t r, char *l);
void printCursor(MenuItem *MI, Display *d);
void eraseCursor(MenuItem *MI, Display *d);
MenuWindow* getNextWindow(MenuItem *MI);
void printItem(MenuItem *MI, Display *d);
void setItemText(MenuItem *MI, char *l);
uint8_t checkItemText(MenuItem *MI);
void writeSelection(MenuItem *MI, Display *d);

#endif /* _my_menu_h_ */
