/*
 * Autor: Daniel Zurek (xzurek12)
 * Datum: 16. 2. 2016
 * Soubor: ahed.h
 * Komentar: Hlavickovy soubor obsahujici prototypy funkci a struktury pro Huffmanuv strom
 */

#ifndef __KKO_AHED_H__
#define __KKO_AHED_H__

#include <stdio.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdlib.h>

#define AHEDOK 0
#define AHEDFail -1

#define DIVIDE_BY_ONE 1
#define DIVIDE_BY_TWO 2

/* Datovy typ zaznamu o (de)kodovani */
typedef struct{
	/* velikost nekodovaneho retezce */
	int64_t uncodedSize;
	/* velikost kodovaneho retezce */
	int64_t codedSize;
} tAHED;

/* Datovy typ zaznamu pro jeden uzel stromu */
typedef struct TreeNode TreeNode;
struct TreeNode{

    uint8_t data;           // Znak
    uint32_t weight;        // Pocet vyskytu

    TreeNode *left;         // Ukzatel na leveho syna
    TreeNode *right;        // Ukzatel na praveho syna
    TreeNode *parent;       // Ukzatel na otce
};

/* Datovy typ zaznamu pro strom */
typedef struct Tree Tree;
struct Tree{

    TreeNode *root;         // Ukazatel na vrchol stromu (root uzel)
    TreeNode *zeroNode;     // Ukazatel na ZeroNode
};

// Prototypy funkci. Jednotlive funkce jsou popsany v souboru ahed.c
void vahy (TreeNode *node);
int getLevel(TreeNode *node);
void nodeInit(TreeNode *node, uint8_t data, uint32_t weight, TreeNode *left, TreeNode *right, TreeNode *parent);
TreeNode *treeSearch(TreeNode *node, uint8_t data);
TreeNode *treeWeightSearch(TreeNode *node, uint32_t weight, TreeNode *origNode, Tree *strom);
void weightsRecount(TreeNode *node, int divideFaktor);
void siblingProperty(TreeNode *node, Tree *strom);
void treeAdd(Tree *strom , uint8_t data);

/* Nazev:
 *   AHEDEncoding
 * Cinnost:
 *   Funkce koduje vstupni soubor do vystupniho souboru a porizuje zaznam o kodovani.
 * Parametry:
 *   ahed - zaznam o kodovani
 *   inputFile - vstupni soubor (nekodovany)
 *   outputFile - vystupní soubor (kodovany)
 * Navratova hodnota:
 *    0 - kodovani probehlo v poradku
 *    -1 - pri kodovani nastala chyba
 */
int AHEDEncoding(tAHED *ahed, FILE *inputFile, FILE *outputFile);


/* Nazev:
 *   AHEDDecoding
 * Cinnost:
 *   Funkce dekoduje vstupni soubor do vystupniho souboru a porizuje zaznam o dekodovani.
 * Parametry:
 *   ahed - zaznam o dekodovani
 *   inputFile - vstupni soubor (kodovany)
 *   outputFile - vystupní soubor (nekodovany)
 * Navratova hodnota:
 *    0 - dekodovani probehlo v poradku
 *    -1 - pri dekodovani nastala chyba
 */
int AHEDDecoding(tAHED *ahed, FILE *inputFile, FILE *outputFile);

#endif

