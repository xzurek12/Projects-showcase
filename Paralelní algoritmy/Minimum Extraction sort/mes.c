#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>


#define TAG 0
#define NEUTRAL_VALUE -1
#define STOP_CYCLE -2

//-------------------------------------------------------------------------
/**
* Struktura Node obsahujici zaznam jednoho uzlu(procesu)
**/
typedef struct{

    int c;
    int stopFlag, stepEnd;
    int leafNumber, processorNumber;
    int leftChild, rightChild;
    int leftChildId, rightChildId, parentId;

}Node;
//-------------------------------------------------------------------------
/**
* Funkce getFirstLeaf vrati hodnotu prvniho uzlu(procesu), ktery bude listem
**/
int getFirstLeaf(int processorNumber)
{
    // Procesy se cisluji od 0
    return ((processorNumber + 1)/2) - 1;
}
//-------------------------------------------------------------------------
/**
* Funkce setNewValue priradi uzlu a levemu nebo pravemu synu novou hodnotu
**/
void setNewValue(Node *processor, int child)
{
    if (child > 0)
    {
        // Pravy syn
        processor->leafNumber = processor->rightChild;
        processor->rightChild = NEUTRAL_VALUE;
    }
    else
    {
        // Levy syn
        processor->leafNumber = processor->leftChild;
        processor->leftChild = NEUTRAL_VALUE;
    }
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    int numprocs;
    int myid;
    MPI_Status stat;

    FILE *fp;
    int retc;
    int leaf;

    //double startTime, endTime;

    MPI_Init(&argc,&argv);                   // inicializace MPI
    MPI_Comm_size(MPI_COMM_WORLD,&numprocs); // zjistime, kolik procesu bezi
    MPI_Comm_rank(MPI_COMM_WORLD,&myid);     // zjistime id sveho procesu

    Node processor = {0, 0, 0, NEUTRAL_VALUE, 0, 0, 0, 0, 0, 0};

    // Zjisteni hodnoty poctu zadanych procesoru
    processor.processorNumber = atoi(argv[2]);
    // Zjisteni hodnoty prvniho listoveho uzlu(procesoru)
    leaf = getFirstLeaf(processor.processorNumber);
    // Zjisteni hodnot synu daneho uzlu
    processor.leftChildId = myid * 2 + 1;               // Levy syn
    processor.rightChildId = processor.leftChildId + 1; // Pravy syn

    if (myid == 0)
    {
        // Otevreni vstupniho souboru (soubor <numbers> vygenerovany utilitou dd)
        fp = fopen(argv[1], "r");
        if (fp == NULL)
        {
            fprintf(stderr, "Chyba: Nelze otevrit vstupni soubor - %s\n", argv[1]);
            MPI_Finalize();
            return EXIT_FAILURE;
        }
        // Nacitani vstupu
        while ((processor.c = getc(fp)) != EOF)
        {
            printf("%d ", processor.c);

            // K serazeni byl zadan jen jeden prvek a jeden procesor
            if (!(leaf))
            {
                printf("\n");
                printf("%d\n", processor.c);

                fclose(fp);
                MPI_Finalize();
                return EXIT_SUCCESS;
            }

            // Cisla jsou zaslany na listove uzly(procesy) - zacinaji na pozici leaf
            MPI_Send(&processor.c, 1, MPI_INT, leaf, TAG, MPI_COMM_WORLD);
            leaf++;
        }
        printf("\n");

        // Pokud bylo nacteno mene hodnot nez je listovych procesoru
        // musi byt zbylym listovym procesorum zaslano neutralni cislo(NEUTRAL_VALUE)
        if (leaf != processor.processorNumber)
        {
            while (leaf != processor.processorNumber)
            {
                processor.c = NEUTRAL_VALUE;
                MPI_Send(&processor.c, 1, MPI_INT, leaf, TAG, MPI_COMM_WORLD);
                leaf++;
            }
        }
        // Zavru vstupni soubor
        retc = fclose(fp);
        if (retc != 0)
        {
            fprintf(stderr, "Chyba: Nelze zavrit vstupni soubor - %s\n", argv[1]);
        }
    }
    // Listove uzly cekaji na zpravu(hodnoty) od uzlu 0 a svou hodnotu ulozi do promenne leafNumber
    if (myid >= getFirstLeaf(processor.processorNumber))
    {
        // Obdrzeni zpravy od procesu s rankem 0
        // Operace MPI_Recv je blokujici
        MPI_Recv(&processor.leafNumber, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD, &stat);
    }

    // Zacatek mereni
    //startTime = MPI_Wtime();

    // Razeni hodnot ve stromu, teoreticky to zabere 2*n + log(n) - 1 kroku
    // n = pocet listu
    while (processor.stopFlag != STOP_CYCLE)
    {
        // Nelistovy uzel(proces)
        if (myid < getFirstLeaf(processor.processorNumber))
        {
            // Ma-li uzel s rankem 0 korektni hodnotu, muzu ji vytisknout
            if ((myid == 0) && (processor.leafNumber >= 0))
            {
                // Konec mereni
                //endTime = MPI_Wtime();
                //printf("Runtime = %f\n", endTime-startTime);

                printf("%d\n",processor.leafNumber);
                processor.leafNumber = NEUTRAL_VALUE;
            }

            if ((processor.leafNumber == NEUTRAL_VALUE) && (processor.stepEnd != STOP_CYCLE))
            {
                // Prijmu zpravy(hodnoty) od svych synu
                MPI_Recv(&processor.leftChild, 1, MPI_INT, processor.leftChildId, TAG, MPI_COMM_WORLD, &stat);
                MPI_Recv(&processor.rightChild, 1, MPI_INT, processor.rightChildId, TAG, MPI_COMM_WORLD, &stat);

                // Koreknti hodnotu ma pouze muj pravy syn
                if ((processor.leftChild == NEUTRAL_VALUE) && (processor.rightChild != NEUTRAL_VALUE))
                {
                    setNewValue(&processor, 1);
                }
                else if ((processor.rightChild == NEUTRAL_VALUE) && (processor.leftChild != NEUTRAL_VALUE))
                {
                    // Koreknti hodnotu ma pouze muj levy syn
                    setNewValue(&processor, 0);

                }
                else if ((processor.leftChild == NEUTRAL_VALUE) && (processor.rightChild == NEUTRAL_VALUE))
                {
                    // Levy i pravy syn jsou prazdni
                    processor.leftChild = STOP_CYCLE;
                    processor.rightChild = STOP_CYCLE;
                    processor.stepEnd = STOP_CYCLE;

                    if (myid == 0)
                    {
                        processor.stopFlag = STOP_CYCLE;
                    }
                }
                else if (processor.leftChild <= processor.rightChild)
                {
                    // Levy syn je mensi nez pravy
                    setNewValue(&processor, 0);
                }
                else if (processor.leftChild > processor.rightChild)
                {
                    // Pravy syn je mensi nez levy
                    setNewValue(&processor, 1);
                }

                // Otec odesle nove hodnoty pro syny zpet synum
                MPI_Send(&processor.leftChild, 1, MPI_INT, processor.leftChildId, TAG, MPI_COMM_WORLD);
                MPI_Send(&processor.rightChild, 1, MPI_INT, processor.rightChildId, TAG, MPI_COMM_WORLD);
            }
        }

        if (myid > 0)
        {
            // Vypocet ranku meho otce
            if((myid % 2) == 0)
            {
                processor.parentId = (myid - 2)/2;  // Jsem pravy syn
            }
            else
            {
                processor.parentId = (myid - 1)/2;  // Jsem levy syn
            }

            MPI_Send(&processor.leafNumber, 1, MPI_INT, processor.parentId, TAG, MPI_COMM_WORLD);
            MPI_Recv(&processor.leafNumber, 1, MPI_INT, processor.parentId, TAG, MPI_COMM_WORLD, &stat);

            if (processor.leafNumber == STOP_CYCLE)
            {
                processor.stopFlag = STOP_CYCLE;
            }
        }
	}

    MPI_Finalize();

    return EXIT_SUCCESS;
}
