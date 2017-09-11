#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <gmp.h>


#define TRIAL_DIVISION 1000000
#define ITERATION 100


//----------------------------------------------------------------------------------
/**
* Funkce myGcd vypocte nejmensiho spolecneho delitele cisel a, b
**/
char * myGcd(mpz_t a, mpz_t b)
{
    char *str;
    mpz_t r, A, B;
    mpz_init(r);
    mpz_init(A);
    mpz_init(B);

    mpz_set(A,a);
    mpz_set(B,b);

    while (1)
    {
        if (mpz_cmp_ui(B,0) == 0) break;

        mpz_set(r,B);
        mpz_mod(B,A,B);
        mpz_set(A,r);
    }

    str = mpz_get_str(NULL,10,A);

    mpz_clear(r);
    mpz_clear(A);
    mpz_clear(B);
    return str;
}
//----------------------------------------------------------------------------------
/**
* Funkce printHex vytiskne cislo v hexadecimalnim tvaru
**/
void printHex(mpz_t number)
{
    printf("0x");
    mpz_out_str (stdout, 16, number);
    printf("\n");
}
//----------------------------------------------------------------------------------
/**
* Funkce printHexNoNewLine vytiskne cislo v hexadecimalnim tvaru
**/
void printHexNoNewLine(mpz_t number)
{
    printf("0x");
    mpz_out_str (stdout, 16, number);
    printf(" ");
}
//----------------------------------------------------------------------------------
/**
* Funkce rabinMiller urci zda-li je zadane cislo prvocislo
**/
int rabinMiller(mpz_t n, int iteration)
{
    mpz_t r, s, N, mod, a, x, pow, x1, r1, n1;
    long int range;

    mpz_init(r);
    mpz_init(s);
    mpz_init(N);
    mpz_init(mod);
    mpz_init(a);
    mpz_init(x);
    mpz_init(pow);
    mpz_init(x1);
    mpz_init(r1);
    mpz_init(n1);

    mpz_set(N,n);

    if (mpz_cmp_ui(n,2) < 0) return 1;

    mpz_set_ui(r,0);
    mpz_sub_ui(s,N,1);

    while(1)
    {
        mpz_mod_ui(mod,s,2);
        if (mpz_cmp_ui(mod,0) != 0) break;

        mpz_add_ui(r,r,1);
        mpz_cdiv_q_ui(s,s,2);
    }

    for (int i = 0; i < iteration; i++)
    {
        gmp_randstate_t gmpRandState;
        gmp_randinit_default(gmpRandState);
        gmp_randseed_ui(gmpRandState,(time(NULL) + getpid()));

        mpz_urandomm(a,gmpRandState,N);

        mpz_powm(pow,a,s,N);
        mpz_set(x,pow);
        mpz_sub_ui(x1,x,1);

        if ((mpz_cmp_ui(x,1) == 0) || (mpz_cmp(x,x1) == 0))
        {
            continue;
        }

        mpz_sub_ui(r1,r,1);
        range = mpz_get_ui(r1);

        for (int i = 0; i < range; i++)
        {
            mpz_powm_ui(pow,x,2,N);
            mpz_set(x,pow);

            mpz_sub_ui(n1,N,1);
            if (mpz_cmp(x,n1) == 0) break;
        }

        return 1;
    }

    mpz_clear(r);
    mpz_clear(s);
    mpz_clear(N);
    mpz_clear(mod);
    mpz_clear(a);
    mpz_clear(x);
    mpz_clear(pow);
    mpz_clear(x1);
    mpz_clear(r1);
    mpz_clear(n1);

    return 0;
}
//----------------------------------------------------------------------------------
/**
* Funkce generateKeys vygeneruje klice P Q N E D pro sifru RSA
**/
void generateKeys(int bits)
{
    mpz_t randQ, randP, modQ, modP, N, Q1, P1, phiN, E, D, invExp, gmpGcd;
    int isPrimeQ, isPrimeP;
    char *strGcd;

    // Inicializace - mpz_inits(x,y,z,NULL) nefunguje?
    mpz_init(randQ);
    mpz_init(randP);
    mpz_init(modQ);
    mpz_init(modP);
    mpz_init(N);
    mpz_init(Q1);
    mpz_init(P1);
    mpz_init(phiN);
    mpz_init(E);
    mpz_init(D);
    mpz_init(invExp);
    mpz_init(gmpGcd);

    gmp_randstate_t gmpRandState;
    gmp_randinit_default(gmpRandState);
    gmp_randseed_ui(gmpRandState,(time(NULL) + getpid()));

    // Generovani dvou nahodnych cisel Q a P v rozsahu bits/2 bitu
    mpz_urandomb(randQ,gmpRandState,(bits+1)/2);
    mpz_urandomb(randP,gmpRandState,bits-(bits+1)/2);

    // Kontrola spravneho poctu bitu
    mpz_setbit(randQ,((bits+1)/2) - 1);
    mpz_setbit(randQ,(bits-(bits+1)/2) - 2);
    mpz_setbit(randP,((bits+1)/2) - 1);
    mpz_setbit(randP,(bits-(bits+1)/2) - 2);

    // Kontrola zda-li je vygenerovane cislo prvocislo,
    // pokud neni, pricita se k tomuto cislu 1,
    // pokud je toto nove cislo liche, provede se nova kontrola.
    // (efektivnejsi nez kontrola pri kazde inkrementaci)
    isPrimeQ = rabinMiller(randQ, ITERATION);
    if (isPrimeQ > 0)
    {
        while(1)
        {
            mpz_add_ui(randQ,randQ,1);
            mpz_mod_ui(modQ,randQ,2);

            if (mpz_cmp_ui(modQ,0) != 0)
            {
                isPrimeQ = rabinMiller(randQ, ITERATION);
                if (!isPrimeQ)
                {
                    break;
                }
            }
        }
    }
    // To same pro P
    isPrimeP = rabinMiller(randP, ITERATION);
    if (isPrimeP > 0)
    {
        while(1)
        {
            mpz_add_ui(randP,randP,1);
            mpz_mod_ui(modP,randP,2);

            if (mpz_cmp_ui(modP,0) != 0)
            {
                isPrimeP = rabinMiller(randP, ITERATION);
                if (!isPrimeP)
                {
                    break;
                }
            }
        }
    }
    printHexNoNewLine(randP);
    printHexNoNewLine(randQ);

    // Gnerovani modulu N
    mpz_mul(N,randQ,randP);
    printHexNoNewLine(N);

    mpz_sub_ui(Q1,randQ,1); // q-1
    mpz_sub_ui(P1,randP,1); // p-1
    mpz_mul(phiN,Q1,P1);    // phiN = (q-1)*(p-1)

    // gcd(E,phiN) = 1; E > 0 && E < phiN-1
    while(1)
    {
         mpz_urandomm(E,gmpRandState,phiN);
         if (mpz_cmp_ui(E,0) == 0) continue;

         strGcd = myGcd(E,phiN);
         mpz_set_str(gmpGcd,strGcd,10);

         if (mpz_cmp_ui(gmpGcd,1) == 0) break;
    }
    printHexNoNewLine(E);

    // D = (E^-1) mod phiN
    mpz_set_ui(invExp,1);
    mpz_neg(invExp,invExp);
    mpz_powm(D,E,invExp,phiN);

    printHex(D);

    mpz_clear(randQ);
    mpz_clear(randP);
    mpz_clear(modQ);
    mpz_clear(modP);
    mpz_clear(N);
    mpz_clear(Q1);
    mpz_clear(P1);
    mpz_clear(phiN);
    mpz_clear(E);
    mpz_clear(D);
    mpz_clear(invExp);
    mpz_clear(gmpGcd);
}
//----------------------------------------------------------------------------------
/**
* Funkce encrypt zasifruje danou zpravu M
**/
void encrypt(mpz_t E, mpz_t N, mpz_t M)
{
    mpz_t C;
    mpz_init(C);

    // C = M^E mod N
    mpz_powm(C, M, E, N);
    printHex(C);

    mpz_clear(C);
}
//----------------------------------------------------------------------------------
/**
* Funkce decrypt desifruje na zaklade soukromeho klice
**/
void decrypt(mpz_t D, mpz_t N, mpz_t C)
{
    mpz_t M;
    mpz_init(M);

    // M = C^D mod N
    mpz_powm(M, C, D, N);
    printHex(M);

    mpz_clear(M);
}
//----------------------------------------------------------------------------------
/**
* Funkce pollardRho provede faktorizaci na zaklade metody Pollard Rho
**/
int pollardRho(mpz_t N)
{
    mpz_t x, y, c, g, abs, mod, xy, r, n;
    char *str;

    mpz_init(x);
    mpz_init(y);
    mpz_init(c);
    mpz_init(g);
    mpz_init(abs);
    mpz_init(mod);
    mpz_init(xy);
    mpz_init(r);
    mpz_init(n);

    mpz_mod_ui (mod,N,2);
    if (mpz_cmp_ui(mod, 0) == 0) return 2;

    gmp_randstate_t gmpRandState;
    gmp_randinit_default(gmpRandState);
    gmp_randseed_ui(gmpRandState,(time(NULL) + getpid()));

    mpz_urandomm(x, gmpRandState, N);
    mpz_set(y,x);
    mpz_urandomm(c, gmpRandState, N);
    mpz_set_ui(g,1);

    while (mpz_cmp_ui(g, 1) == 0)
    {
        mpz_mul(x,x,x);
        mpz_mod(x,x,N);
        mpz_add(x,x,c);
        mpz_mod(x,x,N);

        mpz_mul(y,y,y);
        mpz_mod(y,y,N);
        mpz_add(y,y,c);
        mpz_mod(y,y,N);

        mpz_mul(y,y,y);
        mpz_mod(y,y,N);
        mpz_add(y,y,c);
        mpz_mod(y,y,N);

        mpz_sub(xy,x,y);
        mpz_abs(abs,xy);

        str = myGcd(abs,N);
        mpz_set_str(g,str,10);
    }

    printHex(g);

    mpz_clear(x);
    mpz_clear(y);
    mpz_clear(c);
    mpz_clear(g);
    mpz_clear(abs);
    mpz_clear(mod);
    mpz_clear(xy);

    return 0;
}
//----------------------------------------------------------------------------------
/**
* Funkce trialDivision provede faktorizaci zadaneho N zkusmim delenim
**/
int trialDivision(mpz_t N)
{
    mpz_t retc, divisior;

    mpz_init(retc);
    mpz_init(divisior);

    // Zkusme deleni pro prvnich 1 000 000 cisel
    for (int i = 2; i < TRIAL_DIVISION; i++)
    {
        mpz_set_ui(divisior, i);
        mpz_mod(retc, N, divisior);

        if (mpz_cmp_ui(retc, 0) == 0)
        {
            printHex(divisior);
            return 0; // Uspech
        }
    }

    mpz_clear(retc);
    mpz_clear(divisior);
    return 1; // Neuspech
}
//----------------------------------------------------------------------------------
/**
* Funkce broke provede prolomeni sifry na zaklade faktorizace
**/
void broke(mpz_t N)
{
    int retc;

    // Jednoducha funkce na faktorizaci
    retc = trialDivision(N);
    if (retc > 0)
    {
        // Sofistikovana funkce na faktorizaci
        retc = pollardRho(N);
        if (retc > 0) printf("0x2\n");
    }
}

//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    int bits; // Bitova velikost modula n
    mpz_t E, N, M, D, C;

    // Kontrola parametru
    if (strcmp(argv[1], "-g") == 0)       // Generovani klicu
    {
        // Nacteni parametru pro generovani klicu
        sscanf(argv[2],"%d", &bits);
        generateKeys(bits);
    }
    else if (strcmp(argv[1], "-e") == 0)  // Sifrovani
    {
        // Nacteni parametru pro sifrovani
        mpz_init_set_str(E, argv[2] + 2, 16);
        mpz_init_set_str(N, argv[3] + 2, 16);
        mpz_init_set_str(M, argv[4] + 2, 16);

        encrypt(E, N, M);

        mpz_clear(E);
        mpz_clear(N);
        mpz_clear(M);
    }
    else if (strcmp(argv[1], "-d") == 0)  // Desifrovani
    {
        // Nacteni parametru pro desifrovani
        mpz_init_set_str(D, argv[2] + 2, 16);
        mpz_init_set_str(N, argv[3] + 2, 16);
        mpz_init_set_str(C, argv[4] + 2, 16);

        decrypt(D, N, C);

        mpz_clear(D);
        mpz_clear(N);
        mpz_clear(C);
    }
    else if (strcmp(argv[1], "-b") == 0)  // Prolomeni RSA
    {
        // Nacteni parametru pro prolomeni
        mpz_init_set_str(N, argv[2] + 2, 16);
        broke(N);

        mpz_clear(N);
    }
    else                                  // Neznamy parametr programu
    {
        fprintf(stderr, "Chyba: Neznamy argument programu\n");
        EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
