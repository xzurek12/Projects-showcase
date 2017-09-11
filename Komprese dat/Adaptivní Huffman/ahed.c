/*
 * Autor: Daniel Zurek (xzurek12)
 * Datum: 16. 2. 2016
 * Soubor: ahed.c
 * Komentar: Soubor obsahujici implementaci funkci pro adaptivni Huffmanovo kodovani
 */

#include "ahed.h"

void vahy (TreeNode *node)
{
    if (node != NULL)
    {
        printf("uzel: %u, vaha: %u\n", node->data, node->weight);
        vahy(node->right);
        vahy(node->left);
    }
}

//------------------------------------------------------------------------

/** Nazev:
 *   getLevel
 * Cinnost:
 *   Funkce spocita uroven(hloubku), ve ktere se nachazi uzel stromu.
 * Parametry:
 *   node - ukazatel na uzel
 * Navratova hodnota:
 *   0 - strom je prazdny
 *   > 0 - hloubka pro dany uzel
 **/
int getLevel(TreeNode *node)
{
    int level = 0;
    TreeNode *pomNodePtr = node;

    while (pomNodePtr != NULL)
    {
        level++;
        pomNodePtr = pomNodePtr->parent;
    }

    return level;
}
//------------------------------------------------------------------------
/** Nazev:
 *   nodeInit
 * Cinnost:
 *   Funkce inicializuje uzel stromu na nove hodnoty.
 * Parametry:
 *   node - ukazatel na uzel
 *   data - data pro uzel
 *   weight - vaha pro uzel
 *   left - ukazatel na leveho syna
 *   right - ukazatel na praveho syna
 *   parent - ukazatel na otce
 * Navratova hodnota:
 *   -
 **/
void nodeInit(TreeNode *node, uint8_t data, uint32_t weight, TreeNode *left, TreeNode *right, TreeNode *parent)
{
    node->data = data;
    node->weight = weight;

    node->left = left;
    node->right = right;
    node->parent = parent;
}
//------------------------------------------------------------------------
/** Nazev:
 *   treeSearch
 * Cinnost:
 *   Funkce nalezne dany uzel ve stromu podle dat.
 * Parametry:
 *   node - ukazatel na uzel
 *   data - data pro uzel
 * Navratova hodnota:
 *   TreeNode - ukazatel na nalezeny uzel
 *   NULL - uzel ve stromu neni
 **/
TreeNode *treeSearch(TreeNode *node, uint8_t data)
{
    TreeNode *pomNodePtr = NULL;

    if (node == NULL || node->data == data)
    {
        return node;
    }

    pomNodePtr = treeSearch(node->left, data);
    if (pomNodePtr == NULL)
    {
        pomNodePtr = treeSearch(node->right, data);
    }

    return pomNodePtr;
}
//------------------------------------------------------------------------
/** Nazev:
 *   treeWeightSearch
 * Cinnost:
 *   Funkce pro dany uzel, nalezne uzel s mensi vahou.
 * Parametry:
 *   node - ukazatel na uzel
 *   weight - vaha pro uzel
 *   origNode - ukazatel na uzel vuci kteremu je hledani provadeno
 *   strom - ukazatel na strom
 * Navratova hodnota:
 *   TreeNode - ukazatel na nalezeny uzel
 **/
TreeNode *treeWeightSearch(TreeNode *node, uint32_t weight, TreeNode *origNode, Tree *strom)
{
    TreeNode *pomNodePtr = NULL;

    int levelOrig = 0;
    int levelFound = 0;

    if (node != NULL)
    {
        // Nepokracuj v hledani ze sebe sama
        if (node == origNode)
        {
            return NULL;
        }
        // Byl nalezen uzel s mensi vahou nez dany uzel
        if (node->weight < weight)
        {
            // Omezeni na hloubku vyhledavani
            if ((levelOrig = getLevel(origNode)) < (levelFound = getLevel(node)))
            {
                return NULL;
            }
            // Nesmim najit zeroNode
            if (node == strom->zeroNode)
            {
                return NULL;
            }
            // Nesmim najit sveho otce ani dedu
            if ((node == origNode->parent) || (node == origNode->parent->parent))
            {
                return NULL;
            }
            // Pokud byl nalezen muj levy sourozenec vratim se zpet, protoze je zachovano poradi
            if (node == origNode->parent->left)
            {
                return NULL;
            }
            else
            {
                // Vratim uzel s mensi vahou
                return node;
            }
        }

        // Prohledani praveho podstromu
        pomNodePtr = treeWeightSearch(node->right, weight, origNode, strom);
        if (pomNodePtr != NULL)
        {
            return pomNodePtr;
        }

        // Prohledani leveho podstromu
        pomNodePtr = treeWeightSearch(node->left, weight, origNode, strom);
        if (pomNodePtr != NULL)
        {
            return pomNodePtr;
        }
    }

    return NULL;
}
//------------------------------------------------------------------------
/** Nazev:
 *   weightsRecount
 * Cinnost:
 *   Funkce provede prepocet vah pro cely strom.
 * Parametry:
 *   node - ukazatel na uzel
 *   divideFaktor - hodnota ktera vydeli vysledek souctu vah pro uzel (vyuziva se pri preteceni citace vah)
 * Navratova hodnota:
 *   -
 **/
void weightsRecount(TreeNode *node, int divideFaktor)
{
    TreeNode *pomNodePtr = node;


    if (pomNodePtr != NULL)
    {
        weightsRecount(pomNodePtr->right, divideFaktor);
        weightsRecount(pomNodePtr->left, divideFaktor);

        // Listovy uzel
        if ((pomNodePtr->right == NULL) && (pomNodePtr->left == NULL))
        {
            return;
        }
        else
        {
            pomNodePtr->weight = (pomNodePtr->right->weight + pomNodePtr->left->weight)/divideFaktor;
        }
    }
}
//------------------------------------------------------------------------
/** Nazev:
 *   siblingProperty
 * Cinnost:
 *   Funkce provede kontrolu, zda-li je zachovana Sibling property stromu.
 *   V pripade, ze neni zachovano poradi uzlu, uzly vymeni.
 * Parametry:
 *   node - ukazatel na uzel
 *   strom - ukazatel na strom
 * Navratova hodnota:
 *   -
 **/
void siblingProperty(TreeNode *node, Tree *strom)
{
    TreeNode *pomSwapPtr = NULL;
    TreeNode *pomSearchNode = NULL;
    TreeNode *pomNodePtr = node;

    int levelOrig = 0;
    int levelFound = 0;

    while (pomNodePtr != NULL)
    {
        weightsRecount(strom->root, DIVIDE_BY_ONE);

        /*vahy(strom->root->right);
        printf("--------------------------------------------------\n");
        vahy(strom->root->left);
        printf("--------------------------------------------------\n");
        printf("**************************************************\n");
        printf("--------------------------------------------------\n");*/

        // Hledani listu s mensi vahou nez dany list (dulezite pro zachovani Sibling property)
        pomSearchNode = treeWeightSearch(strom->root, pomNodePtr->weight, pomNodePtr, strom);
        // Byl nalezen uzel, ktery nesplnuje Sibling property
        if (pomSearchNode != NULL)
        {

/*printf("jsem uzel:  %c s vahou: %u a otcem: %u s vahou: %u\n", (char)pomNodePtr->data, pomNodePtr->weight, pomNodePtr->parent->data, pomNodePtr->parent->weight);
printf("je to uzel: %c s vahou: %u a otcem: %u s vahou: %u\n", (char)pomSearchNode->data, pomSearchNode->weight, pomSearchNode->parent->data, pomSearchNode->parent->weight);
printf("-------------------------------------------------------------------------\n");*/


            // Budou prohozeny dva prave uzly
            if ((pomNodePtr == pomNodePtr->parent->right) && (pomSearchNode == pomSearchNode->parent->right))
            {
                if ((levelOrig = getLevel(pomNodePtr)) == (levelFound = getLevel(pomSearchNode)))
                {
                    pomNodePtr = pomNodePtr->parent;
                    continue;
                }

                pomSwapPtr = pomNodePtr->parent;

                pomNodePtr->parent = pomSearchNode->parent;
                pomSearchNode->parent->right = pomNodePtr;

                pomSwapPtr->right = pomSearchNode;
                pomSearchNode->parent = pomSwapPtr;

                continue;
            }

            // Bude prohozen levy uzel za pravy
            if ((pomNodePtr == pomNodePtr->parent->left) && (pomSearchNode == pomSearchNode->parent->right))
            {
                if ((levelOrig = getLevel(pomNodePtr)) == (levelFound = getLevel(pomSearchNode)))
                {
                    if (pomNodePtr->parent->right->weight > pomSearchNode->weight)
                    {
                        pomNodePtr = pomNodePtr->parent;
                        continue;
                    }
                }

                pomSwapPtr = pomNodePtr->parent;

                pomNodePtr->parent = pomSearchNode->parent;
                pomSearchNode->parent->right = pomNodePtr;

                pomSwapPtr->left = pomSearchNode;
                pomSearchNode->parent = pomSwapPtr;

                continue;
            }

            // Bude prohozen pravy uzel za levy
            if ((pomNodePtr == pomNodePtr->parent->right) && (pomSearchNode == pomSearchNode->parent->left))
            {
                pomSwapPtr = pomNodePtr->parent;

                pomNodePtr->parent = pomSearchNode->parent;
                pomSearchNode->parent->left = pomNodePtr;

                pomSwapPtr->right = pomSearchNode;
                pomSearchNode->parent = pomSwapPtr;

                continue;
            }
        }

        pomNodePtr = pomNodePtr->parent;
    }
}
//------------------------------------------------------------------------
/** Nazev:
 *   treeAdd
 * Cinnost:
 *   Funkce prida do stromu novy uzel, pokud uzel jiz existuje, inkrementuje jeho vahu.
 * Parametry:
 *   strom - ukazatel na strom
 *   data - data pro uzel
 * Navratova hodnota:
 *   -
 **/
void treeAdd(Tree *strom , uint8_t data)
{
    // Pokud prvek ve stromu jiz je, inkrementuje se pocitadlo jeho vyskytu(vahy)
    TreeNode *pomNodePtr = treeSearch(strom->root, data);
    if (pomNodePtr != NULL)
    {
        pomNodePtr->weight++;
        siblingProperty(pomNodePtr, strom);

        return;
    }

    // Alokace noveho prvku a noveho Zero prvku
    TreeNode *newNode = malloc(sizeof(TreeNode));
    if (newNode == NULL)
    {
        fprintf(stderr, "Chyba: Nezdaril se malloc noveho prvku.\n");
        return;
    }

    TreeNode *newZeroNode = malloc(sizeof(TreeNode));
    if (newZeroNode == NULL)
    {
        fprintf(stderr, "Chyba: Nezdaril se malloc noveho Zero prvku.\n");
        return;
    }

    // Inicializace novych prvku
    nodeInit(newNode, data, 1, NULL, NULL, strom->zeroNode);
    nodeInit(newZeroNode, 0, 0, NULL, NULL, strom->zeroNode);

    // Navazani novych prvku na strom
    strom->zeroNode->left = newZeroNode;
    strom->zeroNode->right = newNode;
    strom->zeroNode = newZeroNode;

    siblingProperty(newNode, strom);
}
//------------------------------------------------------------------------

/** Nazev:
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
 **/
int AHEDEncoding(tAHED *ahed, FILE *inputFile, FILE *outputFile)
{
	return AHEDOK;
}

/** Nazev:
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
 **/
int AHEDDecoding(tAHED *ahed, FILE *inputFile, FILE *outputFile)
{
	return AHEDOK;
}





