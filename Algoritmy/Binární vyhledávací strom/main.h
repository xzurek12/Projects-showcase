#define BVSARRAY_SIZE 1024

int BVSArrayTop; // ukazatel na vrchol zasobniku ("zasobnik" je pole BVSArray)

// deklarace prvku binarniho stromu
typedef struct VarItem VarItem;
struct VarItem{

    char VarNameOld[64];
    char VarName[64];
    int NumInt;
    double NumDouble;
    char c;
    char *str;
    unsigned status;    //  Hodnota 1 = int, 2 = double, 3 = char, 4 = string
    VarItem *left;
    VarItem *right;
    VarItem *parent;
    int index;
};

// deklarace pole ukazatelu na jednotlive stromy
typedef VarItem* BVSArray[BVSARRAY_SIZE];

