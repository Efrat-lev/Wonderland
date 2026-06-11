#include <stdio.h>
#include <stdlib.h>

void __stdcall drawRectangle(int base, int height, char* block) {
    int index = 0; 

    for (int i = height; i > 0; i--) {
        for (int j = base; j > 0; j--) {
            if (i == height || i == 1 || j == base || j == 1) {
                block[index] = '*'; 
            } else {
                block[index] = ' '; 
            }
            index++;
        }
        block[index] = '\n'; 
        index++;
    }
    block[index] = '\0'; 
}

int __cdecl main() {
    int base;     
    int height;   
    int size;     
    char* block;  

    printf("Enter base: ");
    scanf("%d", &base);
    
    printf("Enter height: ");
    scanf("%d", &height);

    size = ((base + 1) * height) + 1;

    block = (char*)malloc(size);
    if (block == NULL) {
        printf("Memory allocation failed\n");
        return 1;
    }

    drawRectangle(base, height, block);

    printf("%s", block);

    free(block);

    return 0;
}