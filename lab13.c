#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libsocket/libinetsocket.h>
#include <ctype.h>
#include <unistd.h>

struct lF
{
    int num;
    char *fname;
};

FILE * connect_to_server();
void menu();
char get_choice();
void write_file(FILE *s, char * name);
void list_files(FILE *s);
void download(FILE *s, struct lF * list, int n);
void quit(FILE *s, struct lF *list, int n);
struct lF *get_Files(FILE *s, int *n);
int OK(char *s);

int main()
{
    // Connect
    FILE * s = connect_to_server();
    int n; 
    struct lF *list = get_Files(s, &n);
    // Menu
    while(1){
        menu();
        
        // Get choice
        char choice = get_choice();
        
        // Handle choice
        switch(choice)
        {
            case 'l':
            case 'L':
                list_files(s);
                break;
            
            case 'd':
            case 'D':
                download(s, list, n);
                break;
                
            case 'q':
            case 'Q':
                quit(s, list, n);
                exit(0);
                break;
                
            default:
                printf("Choice must be d, l, or q\n");
        }
        
    }
    
}

/*
 * Connect to server. Returns a FILE pointer that the
 * rest of the program will use to send/receive data.
 */
void error(char *); 
 
FILE * connect_to_server()
{
    int sockfd = create_inet_stream_socket("runwire.com", "1234", LIBSOCKET_IPv4, 0);
    
    if (!sockfd)
    {
        error("Can't connect to server");
    }
    
    FILE *s = fdopen(sockfd, "r+");
    
    return s;
}

/*
 * Display menu of choices.
 */
void menu()
{
    printf("(L) List files\n");
    printf("(D) Download a file\n");
    printf("(Q) Quit\n");
    printf("\n");
}

/*
 * Get the menu choice from the user. Allows the user to
 * enter up to 100 characters, but only the first character
 * is returned.
 */
char get_choice()
{
    printf("Your choice: ");
    char buf[100];
    fgets(buf, 100, stdin);
    return buf[0];
}

void write_file(FILE *s, char *fname)
{
    char b[1000];
    char r[1000];
    
    fprintf(s, "SIZE %s\n", fname);
    fgets(b, 1000, s);
    int size = 0; 
    sscanf(b, "%s %d", r, &size);
    
    if (!OK(r))
    {
        printf("Error: No File Detected\n");
        fgets(b, 1000, s);
        return;
    }
    
    fprintf(s, "GET %s\n", fname);
    fgets(b, 1000, s);
    sscanf(b, "%s", r);
    
    if (!OK(r))
    {
        printf("Error: Failure To Recover File Contents\n");
        return;
    }
    
    FILE *w = fopen(fname, "w");
    int dataSize = 100000;
    int rd = 0;
    int initSize = size;
    float nd = 0;
    
    
    while (size > 0)
    {
        char d[dataSize];
        memset(d, 0, dataSize);
        if (dataSize >= size)
        {
            rd = fread(d, 1, size, s);
        }
        else 
        {
            rd = fread(d, 1, dataSize, s);
        }
        
        fwrite(d, 1, rd, w);
        size -= rd;
    }
        
        fclose(w);
}

/*
 * Display a file list to the user.
 */
void list_files(FILE *s)
{
    char b[1000];
    char r[1000];
    
    fprintf(s, "LIST\n");
    fgets(b, 1000, s);
    sscanf(b, "%s", r);
    if (!OK(r))
    {
        printf("Error: must be OK\n");
        exit(1);
    }
    printf("FILES:\n");
    int i = 1;
    while (fgets(b, 1000, s))
    {
        if (strcmp(b, ".\n") == 0)
        {
            printf("\n");
            return;
        }
        
        int size = 0;
        char n[64];
        sscanf(b, "%d %s", &size, n);
        printf("(%d) %s: %.2fKB\n", i, n, (double)size/1000);
        i++;
    }
    
    return;
}

struct lF *get_Files(FILE *s, int *n)
{
    char b[1000];
    char r[1000];
    
    struct lF *list = malloc(15 * sizeof(struct lF));
    
    fprintf(s, "LIST\n");
    fgets(b, 1000, s);
    sscanf(b, "%s", r);
    if (!OK(r))
    {
        printf("Error: must be OK\n");
        exit(1);
    }
    int i = 0;
    
    while (fgets(b, 1000, s))
    {
        if (strcmp(b, ".\n") == 0)
        {
            *n = i;
            return list;
        }
        
        int size = 0;
        char *fname = malloc(100);
        sscanf(b, "%d %s", &size, fname);
        list[i].num = i + 1;
        list[i].fname = fname;
        i++;
    }
    
    *n = i;
    return list;
}

int OK(char *s)
{
    if (!strcmp(s, "+OK"))
    {
        return 1;
    }
    return 0;
}
/*
 * Download a file.
 * Prompt the user to enter a filename.
 * Download it from the server and save it to a file with the
 * same name.
 */
void download(FILE *s, struct lF * list, int n)
{
   printf("Enter file name or file number: ");
   
   char *fname = malloc(100);
   int num = -1;
   fgets(fname, 100, stdin);
   
   if (isdigit(fname[0]))
   {
       sscanf(fname, "%d", &num);
   }
   
   if (num >= 1 && num <= n)
   {
       for (int i = 0; i < n; i++)
       {
           if (list[i].num == num)
           {
               fname = list[i].fname;
           }
       }
   }
   
   
   write_file(s, fname);
}

/* 
 * Close the connection to the server.
 */
void quit(FILE *s, struct lF *list, int n)
{
    for (int i = 0; i < n; i++)
    {
        free(list[i].fname);
    }
    free(list);
    fclose(s);
}