#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ial.h"

// pomocne makra pro utrceni lepsiho skoku v pripade pouziti 2. heuristiky (pouzito jen MAX)
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

//-------------------------------------------------------------------------------------------
/**
* Funkce pro kontrolu retezce
**/
int StringCheck(char* text)
{
    return(text[0] == '\0') ? 1 : 0;
}
//-------------------------------------------------------------------------------------------
/**
* Funkce pro vypocet skoku pri hledani substringu (1. Heuristika)
**/
void ComputeCharJumps(char* pattern, int* charJump)
{
    int delka_vzoru = strlen(pattern);

    // inicializace pole charJump
    for (int i = 0; i < 128; i++)
    {
        charJump[i] = delka_vzoru;
    }

    // zjisteni delky o kolik se bude moct skocit
    for (int i = 0; i < delka_vzoru; i++)
    {
        charJump[(int)pattern[i]] = delka_vzoru - i - 1;
    }
}
//-------------------------------------------------------------------------------------------
/**
* Funkce pro vypocet skoku pri hledani substringu (2. Heuristika)
**/
void ComputeMatchJumps(char *pattern, int *matchJump)
{
    int k, q, qq;
    int delka_vzoru= strlen(pattern);
    int backup[2*delka_vzoru];

    for (k = 0; k < 2*delka_vzoru; k++)
    {
        backup[k] = 0;
        matchJump[k] = 0;
    }

    for (k = 0; k < delka_vzoru; k++)
    {
        matchJump[k] = 2*delka_vzoru - k - 1;
    }

    k = delka_vzoru-1 ;
    q = delka_vzoru;

    while (k >=0)
    {
        backup[k] = q;

        while((q < delka_vzoru) && (pattern[k] != pattern[q]))
        {
            if ((delka_vzoru - k - 1) < matchJump[q])
            {
                matchJump[q] = delka_vzoru- k - 1;
            }

            q = backup[q];
        }

        k--;
        q--;
    }

    for (k = 0; k <= q; k++)
    {
        if ((delka_vzoru+q-k) < matchJump[k])
        {
            matchJump[k] =  delka_vzoru+q-k;
        }
    }

    qq = backup[q];

    while(q < delka_vzoru)
    {
        while(q <= qq)
        {
            if ((delka_vzoru+qq-q) < matchJump[q])
            {
                matchJump[q] = delka_vzoru+qq-q;
            }

            q++;
        }

        qq = backup[qq];
    }
}
//-------------------------------------------------------------------------------------------
/**
* Funkce pro nalezeni prvniho vyskytu hledaneho podretezce
**/
int find_string(char* text, char* pattern)
{
    int charJump[128]; // zatim jen 128 ale jinak pro libovolnou velikost abecedy
    int matchJump[128];
    int j, k;
    int delka_textu;
    int delka_vzoru;

    if((StringCheck(text) != 0) || (StringCheck(pattern) != 0))
    {
        return -1;
    }

    delka_textu = strlen(text);
    delka_vzoru = strlen(pattern);

    if (delka_vzoru > delka_textu)
    {
        return -1;
    }

    ComputeCharJumps(pattern, charJump); // inicializace pole charJump
    ComputeMatchJumps(pattern, matchJump); // inicializace pole matchJump

    k = delka_vzoru - 1;
    j = delka_vzoru - 1;

    while((j < delka_textu) && (k >= 0))
    {
        if (text[j] == pattern[k])
        {
             j--;
             k--;
        }
        else
        {
            j = j + MAX(charJump[(int)text[j]], matchJump[k]);
            k = delka_vzoru - 1;
        }
    }

    if(k < 0)
    {
        return (j+1); // nalezli sme shodu
    }
    else
    {
        return -1; // shoda nenalezena
    }
}
//-------------------------------------------------------------------------------------------
/**
* Funkce pro sedrideni prvku v pravem a levem poli
**/
void Merge(char* text, char* left, char* right)
{
    int i = 0;
    int j = 0;

    int delka_left = strlen(left);
    int delka_right = strlen(right);

    while((i < delka_left) && (j < delka_right)) // dokud nevypadneme z jednoho z poli
    {
        // razeni podle ordinalni hodnoty jednotlivych znaku
        if (left[i] < right[j])
        {
            text[i + j] = left[i];
            i++;
        }
        else
        {
            text[i + j] = right[j];
            j++;
        }
    }

    // je potreba "dolit" zbyvajici prvky
    if(i < delka_left)
    {
        while(i < delka_left)
        {
            text[i + j] = left[i];
            i++;
        }
    }
    else
    {
        while(j < delka_right)
        {
            text[i + j] = right[j];
            j++;
        }
    }

    // uvolneni pameti pro prave a leve retezce
    free(left);
    free(right);
}
//-------------------------------------------------------------------------------------------
/**
* Funkce pro rozdeleni puvodniho pole na prave a leve (rekurzivne) - Merge_Sort
**/
void sort_string(char* text)
{
    int i = 0;
    int j = 0;
    char* left;
    char* right;
    int delka_text;
    int stred;

    if(StringCheck(text) != 0) // kontrola vstupniho retezce
    {
        fprintf(stderr,"CHYBA: prazdny retezec ve funkci sort_string");
        return;
    }

    delka_text = strlen(text);
    if (delka_text <= 1) // podminka ukonceni rekurze
    {
        return;
    }

    stred = delka_text / 2; //nalezeni stredu retezce text

    left = malloc(sizeof(char) * stred + 1); // alokace pameti pro levou cast retezce
    right = malloc(sizeof(char) * ((delka_text - stred)) + 1); // alokace pameti pro pravou cast retezce

    // plneni leve casti
    for ( i = 0; i < stred; i++)
    {
        left[i] = text[i];
    }
    left[i] = '\0'; // pridani koncove nuly u leve casti (u prave neni potreba - koncova nula s puvodniho retezce)

    // plneni prave casti
    for (j = stred; j <= delka_text; j++)
    {
        right[j - stred] = text[j];
    }

    // rekurzivni volani deleni leve a prave casti
    sort_string(left);
    sort_string(right);
    Merge(text, left, right);
}


//----------------------------------------------------
//********************ODDELAT MAIN********************
//----------------------------------------------------
int main()
{
    int vyskyt;

    char* string_in =  "aaa_bbb_ccc";
    char string_to_sort[] = "zihgfedcba";


    vyskyt = find_string(string_in, "bbb");
    printf("%d\n", vyskyt);

    sort_string(string_to_sort);
    printf("%s\n", string_to_sort);



    return 0;
}
