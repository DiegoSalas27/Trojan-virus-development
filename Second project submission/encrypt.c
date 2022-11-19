#include<stdio.h>

void encrypt(char* fname);
int main()
{
    char fname[20];
    printf("Enter Filename: ");
    gets(fname);
    encrypt(fname);
    return -1;
}
//encrypt should be of type int. return 0 when successful, -1 when not
void encrypt(char* fname)
{
    char ch;
    FILE *fps, *fpt;
    fps = fopen(fname, "r");
    if(fps == NULL)
        return;
    fpt = fopen("swapper.txt", "w");
    if(fpt == NULL)
        return;
    ch = fgetc(fps);
    while(ch != EOF)
    {
        ch = ch+99;
        fputc(ch, fpt);
        ch = fgetc(fps);
    }
    fclose(fps);
    fclose(fpt);
    fps = fopen(fname, "w");
    if(fps == NULL)
        return;
    fpt = fopen("swapper.txt", "r");
    if(fpt == NULL)
        return;
    ch = fgetc(fpt);
    while(ch != EOF)
    {
        ch = fputc(ch, fps);
        ch = fgetc(fpt);
    }
    fclose(fps);
    fclose(fpt);
    printf("\nFile %s Encrypted Successfully!", fname);
}
