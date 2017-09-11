#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"


//-------------------------------------------------------------------------------------------
/**
* Funkce pro inicializaci pole ukazatelu na BVS
**/
void BVSArrayInit (BVSArray* ptr)
{
    if ((*ptr) == NULL)
    {
        fprintf(stderr, "CHYBA: chybi pole ukazatelu na BVS ve funkci BVSArrayInit\n");
        return;
    }
    else
    {
        for (int i = 0; i < BVSARRAY_SIZE; i++)
        {
            ((*ptr)[i]) = NULL; // inicializace pole ukzatelu
        }
    }
}
//-------------------------------------------------------------------------------------------
/**
* Funkce pro inicializaci prvku BVS
**/
void InitVarItem(VarItem *L)
{
    L->left = NULL;
    L->right = NULL;
    L->parent = NULL;
    L->c = '\0';
    L->str = NULL;
    L->NumDouble = 0;
    L->NumInt = 0;
    L->status = -1;
    L->VarName[0] = '\0';
    L->VarNameOld[0] = '\0';
    L->index = -1;
}
//-------------------------------------------------------------------------------------------
/**
* Funkce pro vyhledani prvku v BVS
**/
VarItem* BVSSearh (BVSArray* ptr, char* nazev_promenne, int index_pole)
{
    if (index_pole < -1)
    {
        fprintf(stderr, "CHYBA: index pole na BVS je -1 (BVSArrayTop == -1) ve funkci BVSSearch\n");
        return NULL;
    }

    if ((*ptr) == NULL)
    {
        fprintf(stderr, "CHYBA: chybi pole ukzatelu na BVS ve funkci BVSSearch\n");
        return NULL;
    }

    VarItem* pomItem = (*ptr)[BVSArrayTop];
    if (pomItem == NULL)
    {
        fprintf(stderr, "CHYBA: ukazatel pomItem je NULL ve funkci BVSSearch\n");
        return NULL;
    }
    else
    {
        // hledani polozky podle jmena
        while (pomItem != NULL)
        {
            if (strcmp(pomItem->VarName, nazev_promenne) == 0) // shoda
            {
                return pomItem;
            }
            else if (nazev_promenne[0] < pomItem->VarName[0]) // neshoda - jdu vlevo (kvuli hodnote <)
            {
                pomItem = pomItem->left;
            }
            else // neshoda - pokud je nazev_promenne[0] < nebo == jdu doprava
            {
                pomItem = pomItem->right;
            }
        }
    }

    return NULL; // shoda nenalezena
}
//-------------------------------------------------------------------------------------------
/**
* Funkce pro vlozeni prvku do BVS
**/
void BVSInsert (BVSArray* ptr, char* nazev_promenne, int index_pole)
{
    if ((*ptr) == NULL)
    {
        fprintf(stderr, "CHYBA: chybi pole ukazatelu na BVS ve funkci BVSInsert\n");
        return;
    }

    if (index_pole < 0)
    {
        fprintf(stderr, "CHYBA: index pole na BVS je -1 (BVSArrayTop == -1) ve funkci BVSInsert\n");
        return;
    }

    int delka_promenne = strlen(nazev_promenne);
    if (delka_promenne > 63)
    {
        fprintf(stderr, "CHYBA: prilis dlouhy nazev promenne ve funkci BVSInsert\n");
        return;
    }

    if ((*ptr)[index_pole] == NULL) // ukazatel na BVS je NULL vlozim novy/prvni prvek
    {
        VarItem* newItem = malloc(sizeof(VarItem)); // alokace noveho prvku
        if (newItem == NULL) // kontrola spravne alokace
        {
            fprintf(stderr, "CHYBA: nepovedla se alokace noveho prvku BVS ve funkci BVSInsert\n");
            return;
        }

        InitVarItem(newItem); // inicialzace noveho prvku

        // ukladani nazvu promenne
        for (int i = 0; i < 64; i++)
        {
            newItem->VarName[i] = nazev_promenne[i];
        }

        (*ptr)[index_pole] = newItem;
        newItem->parent = NULL;

        return;
    }
    else // BVS neni prazdny
    {
        VarItem* pomItem = (*ptr)[index_pole];

        pomItem = BVSSearh(ptr, nazev_promenne, index_pole); // kontrola zda prvek je jiz v tabulce
        if (pomItem != NULL) // prvek byl nalezen
        {
            fprintf(stderr, "CHYBA: prvek je jiz v BVS ve funkci BVSInsert\n");
            return;
        }
        else // prvek jeste v tabulce neni
        {
            pomItem = (*ptr)[index_pole];

            while (1)
            {
                if ((pomItem->VarName[0] > nazev_promenne[0]) && (pomItem->left == NULL)) // hodnota nazev_promenne[0] je < - pujdu doleva
                {
                    VarItem* newItem2 = malloc(sizeof(VarItem)); // alokace noveho prvku
                    if (newItem2 == NULL) // kontrola spravne alokace
                    {
                        fprintf(stderr, "CHYBA: nepovedla se alokace noveho prvku BVS ve funkci BVSInsert\n");
                        return;
                    }

                    InitVarItem(newItem2);

                    // ukladani nazvu promenne
                    for (int i = 0; i < 64; i++)
                    {
                        newItem2->VarName[i] = nazev_promenne[i];
                    }

                    pomItem->left = newItem2;
                    newItem2->parent = pomItem; // navazu parent na predesly prvek

                    return;
                }
                else if ((pomItem->VarName[0] <= nazev_promenne[0]) && (pomItem->right == NULL)) // hodnota nazev_promenne[0] je => - pujdu doprava
                {

                    VarItem* newItem2 = malloc(sizeof(VarItem)); // alokace noveho prvku
                    if (newItem2 == NULL) // kontrola spravne alokace
                    {
                        fprintf(stderr, "CHYBA: nepovedla se alokace noveho prvku BVS ve funkci BVSInsert\n");
                        return;
                    }

                    InitVarItem(newItem2);

                    // ukladani nazvu promenne
                    for (int i = 0; i < 64; i++)
                    {
                        newItem2->VarName[i] = nazev_promenne[i];
                    }

                    pomItem->right = newItem2;
                    newItem2->parent = pomItem; // navazu parent na predesly prvek

                    return;
                }

                // urceni smeru postupu stromem
                if (pomItem->VarName[0] > nazev_promenne[0])
                {
                    pomItem = pomItem->left;
                    continue;
                }
                else if (pomItem->VarName[0] <= nazev_promenne[0])
                {
                    pomItem = pomItem->right;
                    continue;
                }
            }
        }
    }

    return;
}
//---------------------------------------------------------------------------------------------
/**
* Funkce zrusi cely BVS, ukazatel na nej nastavi na NULL a snizi hodnotu "zasobniku" BVSArrayStop
**/
void BVSDeleteAll (BVSArray* ptr, int index_pole)
{
    if ((*ptr) == NULL) // kontrola ukzatele na pole s BVS
    {
        fprintf(stderr, "CHYBA: chybi pole ukazatelu na BVS ve funkci BVSDeleteAll\n");
        return;
    }

    if (index_pole < 0)
    {
        fprintf(stderr, "CHYBA: index pole na BVS je -1 (BVSArrayTop == -1) ve funkci BVSDeleteAll\n");
        return;
    }

    VarItem* pomItem = (*ptr)[index_pole]; // ukazatel na prvni prvek
    if (pomItem == NULL) // neni co mazat
    {
        return;
    }

    VarItem* pomItem2 = NULL; // pomocny ukazatel

    // mazu BVS + hledam nejlevejsi
    while (pomItem != NULL)
    {
        if ((pomItem->left == NULL) && (pomItem->right == NULL)) // uvolneni uzlu stromu
        {
            pomItem2 = pomItem->parent;
            if (pomItem2 != NULL)
            {
                if (pomItem2->left == pomItem) // nastaveni leveho na NULL
                {
                    pomItem2->left = NULL;
                }
                else
                {
                    pomItem2->right = NULL; // nastaveni praveho na NULL
                }
            }

            // uvolneni prvku
            free(pomItem);
            pomItem = pomItem2;

        }
        else if ((pomItem->left != NULL) && (pomItem->right != NULL)) // kdyz jsou oba !=NULL jdu nejvic vlevo
        {
            pomItem = pomItem->left;
        }
        else if ((pomItem->left == NULL) && (pomItem->right != NULL))
        {
            pomItem = pomItem->right;
        }
        else
        {
            pomItem = pomItem->left;
        }
    }

    // uvoleneni pomocneho ukazatele pomItem2 a nastaveni ukazatele na BVS na NULL
    (*ptr)[index_pole] = NULL;
    free(pomItem2);

    BVSArrayTop--; // snizeni "zasobniku" BVSArrayStop

    return;
}

//----------------------------------------------------
//********************ODDELAT MAIN********************
//----------------------------------------------------
int main()
{
    BVSArrayTop = 16;
    char* hledana_promenna = "zuzka";

    BVSArray* pole_uk = malloc(sizeof(BVSArray));
    BVSArrayInit(pole_uk);

    VarItem* hledany = NULL;
    //VarItem* hledany2 = NULL;

    BVSInsert(pole_uk, "danek", BVSArrayTop);
    BVSInsert(pole_uk, "adam", BVSArrayTop);
    BVSInsert(pole_uk, "eva", BVSArrayTop);
    BVSInsert(pole_uk, "radek", BVSArrayTop);
    BVSInsert(pole_uk, "hanice", BVSArrayTop);
    BVSInsert(pole_uk, "zuzka", BVSArrayTop);

    hledany = BVSSearh(pole_uk, hledana_promenna, BVSArrayTop);
    if(hledany == NULL)
    {
        printf("nic sem nenasel\n");
        return 0;
    }
    else
    {
        printf("hledal si promennou s nazvem: |%s| nasel si promennou s nazvem: |%s|\n",hledana_promenna, hledany->VarName);

        if (hledany->parent == NULL)
        {
            printf("hledany je hlavni koren stromu\n");
        }
    }

    printf("parent uzlu |%s| je uzel s nazvem |%s|\n", hledana_promenna, hledany->parent->VarName);


    printf("BVSArrayStop = %d\n", BVSArrayTop);
    BVSDeleteAll(pole_uk, BVSArrayTop);
    printf("Vse se smazalo\n");
    printf("BVSArrayStop = %d\n", BVSArrayTop);


    return 0;
}
