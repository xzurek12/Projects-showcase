#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <mpi.h>

using namespace std;

#define TAG 0
#define PROPAGATE 1 // Propagate
#define GENERATE 2  // Generate
#define STOP 0      // Stop

vector<char> firtsRow;  // Vektor pro prvni cislo
vector<char> secondRow; // Vektor pro druhe cislo


//-------------------------------------------------------------------------
/**
* Funkce getFirstLeaf vrati hodnotu prvniho listoveho uzlu
**/
int getFirstLeaf(int processorNumber)
{
    // Procesy se cisluji od 0
    return ((processorNumber + 1)/2) - 1;
}
//-------------------------------------------------------------------------
/**
* Funkce getParentId vrati id otcovskeho uzlu pro dany uzel
**/
int getParentId(int myid)
{
    return ((myid % 2) == 0) ? (myid - 2)/2 : (myid - 1)/2;
}
//-------------------------------------------------------------------------
/**
* Funkce scanArrayD provedene vypocet pole D dle tabulky
**/
char scanArrayD(char leftValue, char rightValue)
{
    if (leftValue == STOP)                  // O | s p g
    {                                       // ----------
        return STOP;                        // s | s s s
    }                                       // p | s p g
    else if (leftValue == PROPAGATE)        // g | g g g
    {
        return rightValue;
    }
    else if (leftValue == GENERATE)
    {
        return GENERATE;
    }
}
//-------------------------------------------------------------------------
/**
* Funkce bitOperation provedene vypocet bitoveho scitani
**/
char bitOperation(char x, char y)
{
    if (x == '0' && y == '0')               // B | 0 1
    {                                       // -------
        return STOP;                        // 0 | s p
    }                                       // 1 | p g
    else if (x == '1' && y == '1')
    {
        return GENERATE;
    }
    else
    {
        return PROPAGATE;
    }
}
//-------------------------------------------------------------------------
/**
* Funkce fillWithZeroes doplni zleva nuly u mensiho(kratsiho) z vektoru
**/
void fillWithZeroes()
{
    // Prvni cislo je kratsi
    if (firtsRow.size() < secondRow.size())
    {
        while (firtsRow.size() != secondRow.size())
        {
            firtsRow.insert(firtsRow.begin(), '0');
        }
    }
    // Druhe cislo je kratsi
    if (firtsRow.size() > secondRow.size())
    {
        while (secondRow.size() != firtsRow.size())
        {
            secondRow.insert(secondRow.begin(), '0');
        }
    }
}
//-------------------------------------------------------------------------
/**
* Funkce getNumbers nacte binarni cisla ze vstupniho souboru do vektoru
**/
int getNumbers()
{
    char c;
    FILE *fp;
    bool newLine = false;
    char const *fileName = "numbers";

    fp = fopen(fileName, "rb");
    if (fp == NULL)
    {
        fprintf(stderr, "Chyba: Nelze otevrit vstupni soubor - %s\n", fileName);
        return -1;
    }
    // Nacitani ze souboru
    while ((c = getc(fp)) != EOF)
    {
        // Nacteny znak je newline - nacteno bude druhe cislo
        if ((c != '0') && (c != '1')) newLine = true;

        if ((!newLine) && ((c == '1') || (c == '0')))
        {
            firtsRow.push_back(c); // Plneni prvniho vektoru
        }

        if ((newLine) && ((c == '1') || (c == '0')))
        {
            secondRow.push_back(c); // Plneni druheho vektoru
        }
    }
    // Vektory maji rozdilne velikosti - zleva se doplni nuly
    if (firtsRow.size() != secondRow.size())
    {
        fillWithZeroes();
    }

    fclose(fp);
    return 0;
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    int numprocs;
    int myid;
    MPI_Status stat;

    MPI_Init(&argc,&argv);                   // Inicializace MPI
    MPI_Comm_size(MPI_COMM_WORLD,&numprocs); // Zjistime, kolik procesu bezi
    MPI_Comm_rank(MPI_COMM_WORLD,&myid);     // Zjistime id sveho procesu

    int retc = 0;

    int leaf = getFirstLeaf(numprocs);       // Prvni listovy uzel
    int leftChildId = myid * 2 + 1;          // Levy syn
    int rightChildId = leftChildId + 1;      // Pravy syn

    // Proces s rankem 0 nacte vstupni data a rozesle je na listy
    if (myid == 0)
    {
        retc = getNumbers(); // Nacteni dat
        if (retc < 0)
        {
            MPI_Finalize();
            return EXIT_FAILURE;
        }

        // Prevedeme vektory na string
        char *firstNumber = &firtsRow[0];
        char *secondNumber = &secondRow[0];

        // Rozesleme listum
        while (leaf != numprocs)
        {
            MPI_Send(firstNumber, firtsRow.size(), MPI_CHAR, leaf, TAG, MPI_COMM_WORLD);    // Odeslu prvni cislo
            MPI_Send(secondNumber, secondRow.size(), MPI_CHAR, leaf, TAG, MPI_COMM_WORLD);  // Odeslu druhe cislo
            leaf++;
        }

        char leftChildValue;
        char rightChildValue;

        // Prijmu hodnotu od svych dvou synu
        MPI_Recv(&leftChildValue, 1, MPI_CHAR, leftChildId, TAG, MPI_COMM_WORLD, &stat);
        MPI_Recv(&rightChildValue, 1, MPI_CHAR, rightChildId, TAG, MPI_COMM_WORLD, &stat);

        // Vypoctu hodnotu
        char result = scanArrayD(leftChildValue, rightChildValue);
        if (result == GENERATE) printf("overflow\n"); // Doslo k preteceni - vypisu overflow

        // Nove hodnoty pro syny
        char newRightChildValue = PROPAGATE;
        char newLeftChildValue = scanArrayD(rightChildValue ,newRightChildValue);

        // Nove hodnoty odeslu zpet na syny
        MPI_Send(&newLeftChildValue, 1, MPI_CHAR, leftChildId, TAG, MPI_COMM_WORLD);
        MPI_Send(&newRightChildValue, 1, MPI_CHAR, rightChildId, TAG, MPI_COMM_WORLD);
    }

    // Oprace pro listove uzly
    if (myid >= getFirstLeaf(numprocs))
    {
        char fixingValue = 0;                   // Korelacni hodnota pri souctu
        int arraySize = (numprocs + 1) / 2;     // Velikost prijimane zpravy
        char firstNumber[arraySize + 1] = {};   // Buffer pro prvni cislo
        char secondNumber[arraySize + 1] = {};  // Buffer pro druhe cislo

        // Listove uzly prijmou zpravu od uzlu s rankem 0 - blokujici cekani
        MPI_Recv(firstNumber, arraySize, MPI_CHAR, 0, TAG, MPI_COMM_WORLD, &stat);
        MPI_Recv(secondNumber, arraySize, MPI_CHAR, 0, TAG, MPI_COMM_WORLD, &stat);

        int parentId = getParentId(myid);            // Ziskam id sveho otce
        int myIndex = myid - getFirstLeaf(numprocs); // Vypocet indexu do obou poli na zaklade id procesu

        char result = bitOperation(firstNumber[myIndex], secondNumber[myIndex]); // Vypocet
        MPI_Send(&result, 1, MPI_CHAR, parentId, TAG, MPI_COMM_WORLD);           // Vysledek odeslu na otce

        // Pockam na odpoved(novou hodnotu) od otce
        MPI_Recv(&result, 1, MPI_CHAR, parentId, TAG, MPI_COMM_WORLD, &stat);
        if (result == GENERATE) fixingValue = 1;

        // Posledni vypocet hodnoty
        char finalResult = (firstNumber[myIndex] + secondNumber[myIndex] + fixingValue) % 2;
        printf("%d:%d\n", myid, (int)finalResult);
    }

    // Operace pro nelistove uzly
    if ((myid > 0) && (myid < getFirstLeaf(numprocs)))
    {
        char leftChildValue;
        char rightChildValue;
        int parentId = getParentId(myid); // Ziskam id sveho otce

        // Ziskam hodnoty od synu
        MPI_Recv(&leftChildValue, 1, MPI_CHAR, leftChildId, TAG, MPI_COMM_WORLD, &stat);
        MPI_Recv(&rightChildValue, 1, MPI_CHAR, rightChildId, TAG, MPI_COMM_WORLD, &stat);

        // Vypoctu hodnotu
        char result = scanArrayD(leftChildValue, rightChildValue);
        MPI_Send(&result, 1, MPI_CHAR, parentId, TAG, MPI_COMM_WORLD);  // Zaslu otci

        // Prijmu zpravu od otce
        char parentValue;
        MPI_Recv(&parentValue, 1, MPI_CHAR, parentId, TAG, MPI_COMM_WORLD, &stat);

        // Pravemu synu zaslu hodnotu od otce
        MPI_Send(&parentValue, 1, MPI_CHAR, rightChildId, TAG, MPI_COMM_WORLD);
        // Levemu synu zasleme novou hodnotu
        result = scanArrayD(rightChildValue, parentValue);
        MPI_Send(&result, 1, MPI_CHAR, leftChildId, TAG, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
