/*
 * Autor: Daniel Zurek (xzurek12)
 * Datum: 16. 2. 2016
 * Soubor: main.c
 * Komentar: Soubor s hlavni funkci programu
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "ahed.h"

//--------------------------------------------------------------------------------
/** Nazev:
 *   printHelp
 * Cinnost:
 *   Funkce tiskne napovedu k programu.
 * Parametry:
 *   -
 * Navratova hodnota:
 *   -
 **/
void printHelp()
{
    printf("Aplikace pro adpaptivni Huffmanovo kodovani a dekodovani.\n");
    printf("---------------------------------------------------------\n");
    printf("Parametry programu:\n");
    printf("-i: <ifile> nazev vstupniho souboru\n");
    printf("-o: <ofile> nazev vystupniho souboru\n");
    printf("-l: <logfile> nazev souboru vystupni zpravy\n");
    printf("-c: aplikace bude vstupni soubor kodovat\n");
    printf("-x: aplikace bude vstupni soubor dekodovat\n");
    printf("-h: tiskne napovedu k programu\n");
    printf("Pokud neni zadan nektery z parametru -i nebo -o, bude vyuzit standartni I/O.\n");
}

//--------------------------------------------------------------------------------
//--------------------------------------MAIN--------------------------------------
//--------------------------------------------------------------------------------
int main(int argc, char **argv)
{
    int c;
    FILE *fp;
    uint8_t znak;

    // Parametry programu
    int isFileIn = 0;
    char *ifile = NULL;
    int isFileOut = 0;
    char *ofile = NULL;
    int isFileLog = 0;
    char *logfile = NULL;
    int isCode = 0;
    int isDecode = 0;
    int isHelp = 0;

    Tree Strom = {NULL, NULL};
    TreeNode root = {0,0,NULL,NULL,NULL};

    Strom.zeroNode = &root;
    Strom.root = &root;


    while((c = getopt(argc, argv, "i:o:l:cxh")) != -1)
    {
        switch (c)
        {
            case 'i':
                ifile = optarg; isFileIn = 1; break;
            case 'o':
                ofile = optarg; isFileOut = 1; break;
            case 'l':
                logfile = optarg; isFileLog = 1; break;
            case 'c':
                isCode = 1; break;
            case 'x':
                isDecode = 1; break;
            case 'h':
                isHelp = 1; break;
            case '?':
                if (optopt == 'i')
                {
                    fprintf (stderr, "Chyba: -%c nezadano jmeno souboru infile.\n", optopt);
                }
                else if (optopt == 'o')
                {
                    fprintf (stderr, "Chyba: -%c nezadano jmeno souboru outputfile.\n", optopt);
                }
                else if (optopt == 'l')
                {
                    fprintf (stderr, "Chyba: -%c nezadano jmeno souboru logfile.\n", optopt);
                }
                else
                {
                    fprintf(stderr, "Chyba: Neznamy parametr -%c.\n", optopt);
                }
                return EXIT_FAILURE;
            default:
                abort();
        }
    }

    if (isCode && isDecode)
    {
        fprintf(stderr, "Chyba: Nelze zaroven kodovat i dekodovat soubor.\n");
        return EXIT_FAILURE;
    }

    if (isHelp)
    {
        printHelp();
    }

    printf("IFILE = %s\n", ifile);
    printf("OFILE = %s\n", ofile);
    printf("LOGFILE = %s\n", logfile);
    printf("OPERACE_COD = %d\n", isCode);
    printf("OPERACE_DEC = %d\n", isDecode);
    printf("OPERACE_HLP = %d\n", isHelp);

    if ((fp = fopen(ifile, "rb")) == NULL)
    {
        fprintf(stderr, "Chyba: Nepodarilo se otevrit soubor - %s.\n", ifile);
        return EXIT_FAILURE;
    }

    // Nacitani ze vstupniho streamu
    while (1)
    {
        znak = fgetc(fp);

        // Konec streamu
        if (feof(fp)) break;

        //printf("%c", (char)znak);
        treeAdd(&Strom, znak);
    }
    printf("\n");


    printf("-----------------------KONEC------------------------\n");
    printf("********************PRAVY STROM*********************\n");
    vahy(Strom.root->right);
    printf("********************LEVY STROM**********************\n");
    vahy(Strom.root->left);
    printf("----------------------------------------------------\n");
    printf("levy: %u, pravy: %u, soucet vah: %u\n", Strom.root->left->data, Strom.root->right->data, Strom.root->weight);









	return EXIT_SUCCESS;
}



