#ifndef IAL_H_INCLUDED
#define IAL_H_INCLUDED

// prototypy funkci

// funkce pro find_string
int StringCheck(char* text);
void ComputeCharJumps(char* pattern, int* charJump);
void ComputeMatchJumps(char *pattern, int *matchJump);
int find_string(char* pattern, char* text);

// funkce pro sort_string
void Merge(char* text, char* left, char* right);
void sort_string(char* text);

#endif // IAL_H_INCLUDED
