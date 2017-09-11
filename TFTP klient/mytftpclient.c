#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <sys/time.h>
#include <time.h>
#include <strings.h>


#define h_addr h_addr_list[0] // kvuli prekladu ve virtualce
#define DEFAULT_BUFLEN 66000
#define MAX_DELKA_RADKY 256

//----------------------------------------------------------------------------------------
/**
*   Funkce zpracuje parametry prikazove radky
**/
int rozdelParametry(char *str, char *addrAport, char *path, char *time, char *block, int *param_w_r, char *mode)
{
    int param_s = 0;
    int param_a = 0;
    int param_d = 0;
    int param_t = 0;
    int param_c = 0;

    int c_uvozovka = 0;
    int index2 = 0;
    char *uvozovka2;
    char path0[256] = "";
    int c_predchozi = 0;

    int retc = 0;
    int c;
    int index = 0;
    int pc = 0;
    int pozice = 0;
    int pozicePrvni = -1;

    //path0 = path;
    strcpy(path0, str);

    while (1)
    {
        // preskoceni mezer
        if (pc == 0)
        {
            c = str[index++];
            if (c == '\0')
            {
                break;
            }
            if (!isspace(c))
            {
                pc = 10;
            }
        }
        // prvni znakk musi byt '-'
        if (pc == 10)
        {
            if (c == '-')
            {
                pc = 20;
            }
            else
            {
                // prvni znak neni '-'
                retc = 2;
                break;
            }
        }
        if (pc == 20)
        {
            c = str[index++];
            if (c == '\0')
            {
                retc = 5;
                break;
            }
            switch(c)
            {
                case 'W': // parametr -W
                    if (*param_w_r > 0)
                    {
                        retc = 3;
                        break;
                    }
                    else
                    {
                        *param_w_r = 2;
                        pc = 0;
                    }
                    break;
                case 'R': // parametr -R
                    if (*param_w_r > 0)
                    {
                        retc = 3;
                        break;
                    }
                    else
                    {
                        *param_w_r = 1;
                        pc = 0;
                    }
                    break;
                case 'd': // parametr -d
                    if (param_d > 0)
                    {
                        retc = 3;
                        break;
                    }
                    else
                    {
                        param_d = 1;
                        pc = 30;
                    }
                    break;
                case 'a': // parametr -a
                    if (param_a > 0)
                    {
                        retc = 9;
                        break;
                    }
                    else
                    {
                        param_a = 1;
                        pc = 2000;
                    }
                    break;
                case 's': // parametr -s
                    if (param_s > 0)
                    {
                        retc = 11;
                        break;
                    }
                    else
                    {
                        param_s = 1;
                        pc = 3000;
                    }
                    break;
                case 't': // parametr -t
                    if (param_t > 0)
                    {
                        retc = 12;
                        break;
                    }
                    else
                    {
                        param_t = 1;
                        pc = 4000;
                    }
                    break;
                case 'c': // parametr -c
                    if (param_c > 0)
                    {
                        retc = 20;
                        break;
                    }
                    else
                    {
                        param_c = 1;
                        pc = 5000;
                    }
                    break;

                 default: // neznamy parametr
                     retc = 4;
                     break;
            }
        }
        // nacitani cesty !!!
        if (pc == 30)
        {
            c = str[index++];
            if (c == '\0')
            {
                retc = 6;
                break;
            }
            // vynechani mezer
            if (!isspace(c))
            {
                pc = 40;
            }
        }
        if (pc == 40)
        {
            if ((char)c == '"')
            {
                c_uvozovka = '"';
                pc = 100;
            }
            else if (c == 39)
            {
                c_uvozovka = '\'';
                pc = 100;
            }
            else
            {
                c_uvozovka = 0;
                pc = 1010;
            }
        }

        // nacitani cestz s " nebp '
        if (pc == 100)
        {
            pozicePrvni = index - 1;
            index2 = index;
            pc = 110;
        }

        if (pc == 110)
        {
            if (index2 == pozice - 1)
            {
                c_predchozi = c_uvozovka;
                index = index2;
                c = str[index++];
                pc = 1000;
            }

            c = str[index2++];
            if (c == '\0')
            {
                pc = 1000;
                continue;
            }

            uvozovka2 = strrchr (path0, c_uvozovka);
            if (uvozovka2 == NULL)
            {
                retc = 45;
                break;
            }

            pozice = (int)(uvozovka2 - path0);
            if (pozice == pozicePrvni)
            {
                retc = 46;
                break;
            }

            *path = (char)c;
            path++;
            *path = 0;
        }

        if (pc == 1000)
        {
            index++;
            pc = 1010;
        }

        if (pc == 1010)
        {
            if (isspace(c) && (c_predchozi != '\\'))
            {
                pc = 0;
                continue;
            }
            if (c == '\0')
            {
                break;
            }

            if (c_uvozovka == 0)
            {
                *path = (char)c;
                path++;
                *path = 0;
            }

            c_predchozi = c;
            c = str[index++];
        }

        // paramert -a, adresaAport
        if (pc == 2000)
        {
            c = str[index++];
            if (c == '\0')
            {
                retc = 10;
                break;
            }
            if (!isspace(c))
            {
                pc = 2010;
            }
        }

        if (pc == 2010)
        {
            if ((isspace(c_predchozi)) && (c == '-'))
            {
                pc = 20;
                continue;
            }
            if (c == '\0')
            {
                break;
            }

            *addrAport = (char)c;
            addrAport++;
            *addrAport = 0;

            c_predchozi = c;
            c = str[index++];
        }

        if (pc == 3000)
        {
            c = str[index++];
            if (c == '\0')
            {
                retc = 10;
                break;
            }
            if (!isspace(c))
            {
                pc = 3010;
            }
        }

        // parametr -s block
        if (pc == 3010)
        {
            if ((isspace(c_predchozi)) && (c == '-'))
            {
                pc = 20;
                continue;
            }
            if (c == '\0')
            {
                break;
            }

            *block = (char)c;
            block++;
            *block = 0;

            c_predchozi = c;
            c = str[index++];
        }

        if (pc == 4000)
        {
            c = str[index++];
            if (c == '\0')
            {
                retc = 10;
                break;
            }
            if (!isspace(c))
            {
                pc = 4010;
            }
        }

        // parametr -t time
        if (pc == 4010)
        {
            if ((isspace(c_predchozi)) && (c == '-'))
            {
                pc = 20;
                continue;
            }
            if (c == '\0')
            {
                break;
            }

            *time = (char)c;
            time++;
            *time = 0;

            c_predchozi = c;
            c = str[index++];
        }

        if (pc == 5000)
        {
            c = str[index++];
            if (c == '\0')
            {
                retc = 21;
                break;
            }
            if (!isspace(c))
            {
                pc = 5010;
            }
        }

        // parametr -c mode
        if (pc == 5010)
        {
            if ((isspace(c_predchozi)) && (c == '-'))
            {
                pc = 20;
                continue;
            }
            if (c == '\0')
            {
                break;
            }

            *mode = (char)c;
            mode++;
            *mode = 0;

            c_predchozi = c;
            c = str[index++];
        }

        if (pc == 9999)
        {
            retc = 1;
            break;
        }

    }// while pc


    if (param_d <= 0)
    {
        retc = 15;
        return retc;
    }
    if (*param_w_r <= 0)
    {
        retc = 16;
        return retc;
    }

    return retc;
}
//----------------------------------------------------------------------------------------
/**
*   Funcke zjisti minimalni hodnotu MTU ze vsech dostupnych rozhranni
*/
int findMinMTU(int sockfd)
{
    struct ifreq *ifr;
    struct ifconf ifc;
    char mtu_buf[2048]; // MTU buffer
    int  interface_number; // pocet rozhrani
    int min = 100000;

    // zjisteni poctu MTU pomoci funkce ioctl
    ifc.ifc_len = sizeof(mtu_buf);
    ifc.ifc_buf = mtu_buf;
    if(ioctl(sockfd, SIOCGIFCONF, &ifc) < 0)
    {
        fprintf(stderr, "CHYBA: Nepodarilo se zjistit pocet MTUs\n");
        printf("--------------------------------------\n");
        return -1;
    }

    ifr = ifc.ifc_req;
    interface_number = ifc.ifc_len / sizeof(struct ifreq);

    // iterace pres vsechny rozhrani
    for(int i = 0; i < interface_number; i++)
    {
        // pomocny ukazatel na strukturu
        struct ifreq *pomUk = &ifr[i];

        // zjisteni velikosti MTU pro danne rozhrani
        if (ioctl(sockfd, SIOCGIFMTU, pomUk) < 0)
        {
            fprintf(stderr, "CHYBA: Nepodarilo se zjistit velikost MTU z interface\n");
            printf("--------------------------------------\n");
            return -1;
        }

        // zjisteni minima
        if (pomUk->ifr_mtu < min)
        {
            min = pomUk->ifr_mtu;
        }
    }

    return min;
}

//----------------------------------------------------------------------------------------
/**
*   Funcke pro kontrolu OACK packetu od serveru - kontrola options
**/
int checkOptions(char *buffer, int blksize, int timeout, int *fileSize, int *blocksize, int *t_out)
{
    int index = 0;
    char *split;
    char ** parametry = NULL;
    int pocetMezer = 0;
    int block;
    int time;
    int fsize;

    // naleznu index zacatku zpravy
    for (int i = 0; i < DEFAULT_BUFLEN; i++)
    {
        if (buffer[i] != '\0')
        {
            index = i;
            break;
        }
    }

    buffer = buffer + index;
    // zamenim vsechny '\0' za # kvuli split
    for (int i = 0; i < DEFAULT_BUFLEN; i++)
    {
        if (buffer[i] == '\0')
        {
            buffer[i] = '#';
        }
    }

    split = strtok (buffer, "#");
    while (split != NULL)
    {
        // alokace/realokace pro pole parametru
        parametry = realloc (parametry, sizeof (char*) * ++pocetMezer);
        if (parametry == NULL)
        {
            return -1;
        }

        parametry[pocetMezer - 1] = split;
        split = strtok (NULL, "#");
    }

    // alokace posleniho prvku pro NULL
    parametry = realloc (parametry, sizeof (char*) * (pocetMezer + 1));
    parametry[pocetMezer] = 0;

    sscanf(parametry[1], "%d", &block);
    sscanf(parametry[3], "%d", &time);
    sscanf(parametry[5], "%d", &fsize);

    *fileSize = fsize;

    // kontrola parametru -s a -t
    if (block != blksize)
    {
        fprintf(stderr, "UPOZORNENI: Server nepotvrdil velikost blocksize, ale prenos bude pokracovat\n");
    }
    *blocksize = block;

    if (time != timeout)
    {
        *t_out = time;
    }

    free(parametry);
    return 0;
}

//----------------------------------------------------------------------------------------
/**
*   Funcke vytiskne casove razitko + pozadovane hlaseni
**/
void tiskniCas(char *str)
{
    struct timeval currentTime;
    char buffer[256];
    char timestamp[256] = "";

    // ziskam systemovy cas
    gettimeofday(&currentTime, NULL);
    int milli = currentTime.tv_usec / 1000;

    // fortmatovaci retezec pro cas
    strftime(buffer, 256, "%Y-%m-%d %H:%M:%S", localtime(&currentTime.tv_sec));
    sprintf(timestamp, "%s.%d", buffer, milli);

    printf("[%s] %s",timestamp, str);
}

//----------------------------------------------------------------------------------------
/**
*   Funcke zjisti jaka bude skutecna velikost souboru pri prenaseni linux->win
**/
int findRealFileLenth(FILE *handle)
{
    char znak = '\0';
    char prev_char = '\0';
    int skutena_delka = 0;
    long file_len;
    int i = 0;

    // naleznu veliksot souboru
    fseek(handle, 0L, SEEK_END);
    file_len = ftell(handle);
    rewind (handle);

    // spocitam o kolik znaku bude soubor vetsi
    while(i != file_len)
    {
        fread(&znak, 1, 1, handle);

        if ((znak == '\n') && (prev_char == '\r'))
        {
            prev_char = znak;
        }
        else if ((znak == '\n') && (prev_char != '\r'))
        {
            znak = '\n';
            prev_char = znak;
            skutena_delka++;
        }
        else
        {
            prev_char = znak;
        }

        i++;
    }
    fseek(handle, 0L, SEEK_SET);
    skutena_delka += file_len;

    return skutena_delka;
}




//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
int main()
{
    // struktury pro pozadovane pakety
    // opcode: RRQ/WRQ packet 1/2
    typedef struct
    {
         short unsigned opcode;
         char msg[514];

    }reqPacket;

    // opcode: 3 DATA packet
    typedef struct
    {
        short unsigned opcode;
        short unsigned block;
        char data[DEFAULT_BUFLEN];

    }datPacket;

    // opcode: 4 ACK packet
    typedef struct
    {
        short unsigned opcode;
        short unsigned block;

    }ackPacket;

    // tisk napovedy
    printf("----- napoveda k programu TFTP klient -----\n"
           "vstup: [-R/-W] [-d] <filename> [-a] <adresa,port> [-s] <blocksize> [-t] <timeout>\n"
           "parametry:\n"
           "            -R -- READ request - zadost o zaslani souboru ze serveru (povinny)\n"
           "            -W -- WRITE request - zadost o zaslani souboru na server (povinny)\n"
           "            -d -- za timto parametrem nasleduje filename (povinny)\n"
           "            -a -- za timto prametrem nasleduje adresa hosta ve formatu <adresa, port>\n"
           "               -- pokud neni zadan, adresa = 127.0.0.1 ; port = 69 (nepovinny)\n"
           "            -s -- za timto prametrem nasleduje hodnota blocksize[8 - 65464] (nepovinny)\n"
           "            -t -- za timto parametrem nasleduje hodnota timeout[s][1 - 255] (nepovinny)\n"
           "            -c -- MODE prenosu, muze nabyvat 'octet (binary)', 'netascii (ascii)'\n"
           "MAIL mode neni podporovan\n"
           "Pokud jsou parametry nespravne zadany, program vypise hlaseni: CHYBA: Spatne zadane prametry\n");
	printf("\n");

    while(1)
    {
        printf("> ");

        int param_w_r = 0;

        // parsovane paramtetry
        char par_str_path[512] = "";
        char par_str_adrrAport[256] = "";
        char par_str_timeout[256] = "";
        char par_str_blocksize[256] = "";
        char par_str_mode[256] = "";

        // prevedene parametry
        char addr[256] = "";
        char str_port[256] = "";
        int timeout = 0;
        int klient_blocksize = -1;
        int mtu_blocksize = 0;
        int port = 0;
        int server_ack_block = 0;
        int server_ack_timeout = 0;

        int j = 0;
        int index_carka = -1;
        int retc;
        char *znak_carka;
        int ipv6 = 0;
        struct sockaddr_in sin;
        struct sockaddr_in6 sin6;
        struct addrinfo hint, *res = NULL;
        struct hostent *remote_host;
        char buffer[DEFAULT_BUFLEN];
        char prikazova_radka[MAX_DELKA_RADKY] = "";
        char msgToPrint[256] = "";
        int velikostSouboru;
        FILE *fp = NULL;
        int soubor_otevren = 0;
        int retry = 10; // pocet opakovani odesilani

        char read_mode[256] = "";
        char write_mode[256] = "";
        char znak = '\0';
        char prev_char = '\0';
        long charCount = 0;
        int realFileSize = 0;
        int ascii = 0;
        int octet = 0;

        // deklarace packet struktur
        reqPacket requestPacket;
        datPacket *dataPacket = (datPacket*)buffer;
        ackPacket *acknowPacket = (ackPacket*)buffer;

        int s; // socket
        int bytes; // prisle data
        struct sockaddr_storage address;
        socklen_t address_len = sizeof(address);


        fgets(prikazova_radka, MAX_DELKA_RADKY, stdin);

        retc = rozdelParametry(prikazova_radka, par_str_adrrAport, par_str_path, par_str_timeout, par_str_blocksize, &param_w_r, par_str_mode);
        if (retc > 0)
        {
            //printf("RETC: %d\n", retc);
            fprintf(stderr, "CHYBA: Spatne zadane prametry\n");
            continue;
        }

        // vyhodnoceni nactene adresy
        if ((strcmp(par_str_adrrAport, "")) == 0)
        {
            // zadna adresa -> localhost a port 69
            strcpy(addr, "127.0.0.1");
            port = 69;
        }
        else
        {
            // nejaka adresa byla nactena
            if ((znak_carka = strchr(par_str_adrrAport, ',')) != NULL)
            {
                // v retezci byl nalezen znak ','
                for (int i = 0; i < strlen(par_str_adrrAport); i++)
                {
                    if (isspace(par_str_adrrAport[i]))
                    {
                        continue;
                    }
                    else if (par_str_adrrAport[i] == ',')
                    {
                        index_carka = i;
                        break;
                    }
                    else
                    {
                        addr[j] = par_str_adrrAport[i];
                        j++;
                    }
                }
                // port bez adresy
                if ((strcmp(addr, "")) == 0)
                {
                    strcpy(addr, "127.0.0.1");
                }

                j = 0;
                for (int i = index_carka + 1; i < strlen(par_str_adrrAport); i++)
                {
                    if (isspace(par_str_adrrAport[i]))
                    {
                        continue;
                    }
                    else
                    {
                        str_port[j] = par_str_adrrAport[i];
                        j++;
                    }
                }
                if ((strcmp(str_port, "")) == 0)
                {
                    port = 69;
                }
                else
                {
                    sscanf(str_port, "%d", &port); // prevedeni portu na cislo
                }
            }
            else
            {
                // byla nactena jen adresa bez portu, port = 69
                for (int i = 0; i < strlen(par_str_adrrAport); i++)
                {
                    if (isspace(par_str_adrrAport[i]))
                    {
                        continue;
                    }
                    else if (par_str_adrrAport[i] == ',')
                    {
                        index_carka = i;
                        break;
                    }
                    else
                    {
                        addr[j] = par_str_adrrAport[i];
                        j++;
                    }
                }

                port = 69;
            }
        }

        // kontrola hodnoty blocksize
        if ((strcmp(par_str_blocksize, "")) == 0)
        {
            klient_blocksize = 512;
        }
        else
        {
            // parsovani hodnoty blocksize
            if ((retc = sscanf(par_str_blocksize, "%d", &klient_blocksize)) != 1)
            {
                fprintf(stderr, "CHYBA: Spatne zadana velikost blocksize\n");
                continue;
            }
        }

        // klientem zadana blocksize musi byt
        if (((klient_blocksize < 8) && (klient_blocksize >= 0)) || (klient_blocksize > 65464))
        {
            fprintf(stderr, "CHYBA: Spatne zadana velikost blocksize\n");
            continue;
        }

        // kontrola hodnoty timeout
        if ((strcmp(par_str_timeout, "")) == 0)
        {
            timeout = 1; // pokud nebyl zadan timeout nastavi se explicitne 1
        }
        else
        {
            // parsovani hodnoty timeout
            if ((retc = sscanf(par_str_timeout, "%d", &timeout)) != 1)
            {
                fprintf(stderr, "CHYBA: Spatne zadana velikost timeout\n");
                continue;
            }
        }
        // klientem zadany timeout musi byt 1 - 255
        if ((timeout < 1) || (timeout > 255))
        {
            fprintf(stderr, "CHYBA: Spatne zadana velikost timeout\n");
            continue;
        }

        // vyhodnoceni nacteneho mode
        if ((strcmp(par_str_mode, "")) == 0)
        {
            // pokud nebyl zadan parametr -c, mode prenosu bude explicitne "octet"
            strcpy(par_str_mode, "octet");
            strcpy(read_mode, "rb");
            strcpy(write_mode, "wb");
            octet = 1;
        }
        else
        {
            sscanf(par_str_mode, "%s", par_str_mode);
            int mode_len = strlen(par_str_mode);

            // mode muze byt podle RFC zadan jak chce, proto se znaky zmeni na lower case
            for (int i = 0; i < mode_len; i++)
            {
                par_str_mode[i] = tolower(par_str_mode[i]);
            }

            if (((strcmp(par_str_mode, "octet")) != 0) &&
            ((strcmp(par_str_mode, "netascii")) != 0) &&
            ((strcmp(par_str_mode, "binary")) != 0) &&
            ((strcmp(par_str_mode, "ascii")) != 0))
            {
                fprintf(stderr, "CHYBA: Spatne zadany parametr -c\n");
                continue;
            }

            // nastaveni cteni/zapisu do/z souboru
            if (((strcmp(par_str_mode, "netascii")) == 0) || ((strcmp(par_str_mode, "ascii")) == 0))
            {
                strcpy(read_mode, "r");
                strcpy(write_mode, "w");
                ascii = 1;
            }

            if (((strcmp(par_str_mode, "binary")) == 0) || ((strcmp(par_str_mode, "octet")) == 0))
            {
                strcpy(read_mode, "rb");
                strcpy(write_mode, "wb");
                octet = 1;
            }
        }

        printf("--------------------------------------\n");
        printf("OPERACE: %d\nFILE: %s\nADRESA: %s\nBLOCK: %d\nTIMEOUT: %d\nPORT: %d\nMODE: %s\n", param_w_r, par_str_path, addr, klient_blocksize, timeout, port, par_str_mode);
        printf("++++++++++++++++++++++++++++++++++++++\n");

        // zjisteni ipv adresy
        memset(&hint, '\0', sizeof hint);
        hint.ai_family = PF_UNSPEC;
        hint.ai_flags = AI_NUMERICHOST;

        retc = getaddrinfo(addr, NULL, &hint, &res);
        if (retc)
        {
            fprintf(stderr, "CHYBA: Spatne zadana adresa getaddrinfo %s\n", addr);
            printf("--------------------------------------\n");
            continue;
        }

        if (res->ai_family == AF_INET) // ipv4
        {
             // vytvoreni socketu pro UDP
            if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
            {
                fprintf(stderr, "CHYBA: Nepodarilo se vytvorit socket\n");
                printf("--------------------------------------\n");
                continue;
            }

            // naplneni struktury socketu
            sin.sin_family = AF_INET;
            sin.sin_port = htons(port);

            // zjisteni adresy hosta
            remote_host = gethostbyname(addr);
            if (remote_host == NULL)
            {
                fprintf(stderr, "CHYBA: Nepodarilo se ziskat adresu\n");
                printf("--------------------------------------\n");
                continue;
            }

            // doplneni struktury sin o polozku s adresou
            memcpy(&sin.sin_addr, remote_host->h_addr, remote_host->h_length);

        }
        else if (res->ai_family == AF_INET6) // ipv6
        {
            ipv6 = 1;

            if ((s = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP)) < 0)
            {
                fprintf(stderr, "CHYBA: Nepodarilo se vytvorit socket\n");
                printf("--------------------------------------\n");
                continue;
            }

            sin6.sin6_family = AF_INET6;
            sin6.sin6_port = htons(port);
            if (inet_pton (AF_INET6, addr, &sin6.sin6_addr) < 1)
            {
                fprintf(stderr, "CHYBA: Nepodarilo se prevest ipv6 adresu (inet_pton)\n");
                printf("--------------------------------------\n");
                continue;
            }
        }
        else
        {
            fprintf(stderr, "CHYBA: Nepodporovany format adresy\n");
            printf("--------------------------------------\n");
            continue;
        }

        // naleznu nejmensi velikost blocksize
        if ((mtu_blocksize = findMinMTU(s)) < 0)
        {
            close(s);
            fprintf(stderr, "CHYBA: Chyba ve funkci findMinMTU (ioctl)\n");
            printf("--------------------------------------\n");
            continue;
        }

        // kontrola zadane klientske blocksize
        if ((klient_blocksize > mtu_blocksize) || (klient_blocksize < 0))
        {
            klient_blocksize = mtu_blocksize;
        }

        char str_final_blocksize[2048] = "";
        char str_final_timeout[2048] = "";
        sprintf(str_final_blocksize, "%d", klient_blocksize);
        sprintf(str_final_timeout, "%d", timeout);

        struct timeval tv;
        tv.tv_sec = timeout;
        tv.tv_usec = 0;
        // nastaveni timeoutu
        if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv,sizeof(tv)) < 0)
        {
            close(s);
            fprintf(stderr,"CHYBA: Nepodarilo se vytvorit timeout\n");
            printf("--------------------------------------\n");
            continue;
        }

        // KOMUNIKACE------------------------------------------------------KOMUNIKACE
        // KOMUNIKACE------------------------------------------------------KOMUNIKACE
        // KOMUNIKACE------------------------------------------------------KOMUNIKACE


        // RRQ
        if (param_w_r == 1)
        {
            // vytvoreni RRQ packetu
            requestPacket.opcode = htons(param_w_r);
            sprintf(requestPacket.msg, "%s%c%s%c%s%c%s%c%s%c%s%c%s%c%s%c",
                    par_str_path, '\0',         // filepath
                    par_str_mode, '\0',         // mode prenosu
                    "blksize", '\0',            // blocksize
                    str_final_blocksize, '\0',  // hodnota blocksize
                    "timeout", '\0',            // timeout
                    str_final_timeout, '\0',    // hodnota timeout
                    "tsize", '\0',              // tsize
                    "0", '\0');                 // RRQ - hodnota tsize je 0


            // vypocet delky zpravy
            int msg_len = 2+strlen(par_str_path)+strlen(par_str_mode)+strlen("blksize")+strlen(str_final_blocksize)+strlen("timeout")+strlen(str_final_timeout)+strlen("tsize")+9;

            if (ipv6 > 0)
            {
                // poslu zpravu na server
                if ((retc = sendto(s,(void *)&requestPacket, msg_len, 0, (struct sockaddr *)&sin6, sizeof(sin6))) < 0)
                {
                    fprintf(stderr, "CHYBA: Nepodarilo se odeslat request na server\n");
                    printf("--------------------------------------\n");
                    continue;
                }
            }
            else // ipv4
            {
                // poslu zpravu na serveru
                if ((retc = sendto(s,(void *)&requestPacket, msg_len, 0, (struct sockaddr *)&sin, sizeof(sin))) < 0)
                {
                    fprintf(stderr, "CHYBA: Nepodarilo se odeslat request na server\n");
                    printf("--------------------------------------\n");
                    continue;
                }
            }

            int prevBytes = 0;

            sprintf(msgToPrint, "%s %s:%d", "Requesting READ from server", addr, port);
            tiskniCas(msgToPrint);
            memset(msgToPrint, 0, sizeof(msgToPrint));
            printf("\n");

            // prijmam zpravu od serveru
            while (1)
            {
                if (retry == 0)
                {
                    fprintf(stderr, "CHYBA: Vyprsel timeout\n");
                    printf("--------------------------------------\n");
                    break;
                }

                bytes = recvfrom(s, buffer, DEFAULT_BUFLEN , 0, (struct sockaddr *)&address, &address_len);
                if (bytes < 0)
                {
                    retry--;
                    continue;
                }

                // odchytavani chyb od serveru, pokud je opcode == 5
                if ((htons(dataPacket->opcode)) == 5)
                {
                    // po opcode 5 neni vyzadovan ACK packet -> ukonceni spojeni
                    fprintf(stderr, "CHYBA: %s\n", dataPacket->data);
                    printf("--------------------------------------\n");
                    break;
                }

                // opcode 6 znamena ze prisel OACK
                if ((htons(dataPacket->opcode)) == 6)
                {
                    if ((retc = checkOptions(buffer, klient_blocksize, timeout, &velikostSouboru, &server_ack_block, &server_ack_timeout)) < 0)
                    {
                        break;
                    }

                    tv.tv_sec = server_ack_timeout;
                    tv.tv_usec = 0;
                    // nastaveni timeoutu
                    if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv,sizeof(tv)) < 0)
                    {
                        fprintf(stderr,"CHYBA: Nepodarilo se vytvorit timeout\n");
                        printf("--------------------------------------\n");
                        break;
                    }

                   // vytvoreni ACK packetu
                    acknowPacket->opcode = htons(4); // ACK je pozadavek 4
                    acknowPacket->block = 0; // block ziskam z dataPacketu

                    // poslu ACK packet
                    if ((retc = sendto(s, (void *)acknowPacket, 4, 0, (struct sockaddr *)&address, address_len)) < 0)
                    {
                        printf("\n");
                        fprintf(stderr, "CHYBA: Nepodarilo se odeslat ACK packet\n");
                        printf("--------------------------------------\n");
                        retry--;
                        continue;
                    }
                    retry = 10;
                    continue;
                }

                prevBytes += bytes - 4;
                dataPacket->data[bytes - 4] = '\0';

                sprintf(msgToPrint, "%s %d B of %d B", "Receiving DATA ... ", prevBytes, velikostSouboru);
                tiskniCas(msgToPrint);
                printf("\r");
                memset(msgToPrint, 0, sizeof(msgToPrint));

                // otevreni souboru pro zapis
                if (soubor_otevren == 0)
                {
                    fp = fopen(par_str_path, write_mode);
                    if (fp == NULL)
                    {
                        printf("\n");
                        fprintf(stderr, "CHYBA: Soubor nelze otevrit - client\n");
                        printf("--------------------------------------\n");
                        break;
                    }
                    else
                    {
                        soubor_otevren = 1;
                    }
                }

                if (ascii == 1) // ascii zapis
                {
                    while (charCount != bytes - 4)
                    {
                        znak = dataPacket->data[charCount];

                        if (znak == '\r')
                        {
                            charCount++;
                            prev_char = znak;
                            continue;
                        }
                        else if ((znak == '\n') && (prev_char == '\r'))
                        {
                            prev_char = znak;
                            // zapis dat do souboru
                            if (fwrite (&znak, 1, 1, fp) != 1)
                            {
                                printf("\n");
                                fprintf(stderr, "CHYBA: ukladani dat do souboru - client\n");
                                printf("--------------------------------------\n");
                                fclose(fp);
                                break;
                            }
                        }
                        else if ((znak == '\n') && (prev_char != '\r'))
                        {
                            prev_char = znak;
                            if (fwrite (&znak, 1, 1, fp) != 1)
                            {
                                printf("\n");
                                fprintf(stderr, "CHYBA: ukladani dat do souboru - client\n");
                                printf("--------------------------------------\n");
                                fclose(fp);
                                break;
                            }
                        }
                        else
                        {
                            prev_char = znak;
                            if (fwrite (&znak, 1, 1, fp) != 1)
                            {
                                printf("\n");
                                fprintf(stderr, "CHYBA: ukladani dat do souboru - client\n");
                                printf("--------------------------------------\n");
                                fclose(fp);
                                break;
                            }
                        }

                        charCount++;
                    }
                    charCount = 0;
                }
                else if (octet == 1) // binarni zapis
                {
                    // zapis dat do souboru
                    if (fwrite (dataPacket->data, 1, bytes - 4, fp) != bytes - 4)
                    {
                        printf("\n");
                        fprintf(stderr, "CHYBA: ukladani dat do souboru - client\n");
                        printf("--------------------------------------\n");
                        fclose(fp);
                        break;
                    }
                }

                // vytvoreni ACK packetu
                acknowPacket->opcode = htons(4); // ACK je pozadavek 4
                acknowPacket->block = dataPacket->block; // block ziskam z dataPacketu

                // poslu ACK packet
                if ((retc = sendto(s, (void *)acknowPacket, 4, 0, (struct sockaddr *)&address, address_len)) < 0)
                {
                    printf("\n");
                    fprintf(stderr, "CHYBA: Nepodarilo se odeslat ACK packet\n");
                    printf("--------------------------------------\n");
                    retry--;
                    continue;
                }
                retry = 10;

                if (bytes < server_ack_block + 4) // prisly posledni data od serveru budu koncit
                {
                    printf("\n");
                    sprintf(msgToPrint, "%s", "Transfer completed without errors");
                    tiskniCas(msgToPrint);
                    memset(msgToPrint, 0, sizeof(msgToPrint));
                    printf("\n");
                    printf("--------------------------------------\n");

                    break;
                }
            }
        }
        else if (param_w_r == 2) // zapis na server
        {
            prev_char = '\0';
            char str_file_len[256] = "";
            short unsigned block = 1;
            int zbyva_bytu;
            int while_end;
            long file_len = 0;

            if (soubor_otevren == 0)
            {
                fp = fopen(par_str_path, read_mode);
                if (fp == NULL)
                {
                    printf("\n");
                    fprintf(stderr, "CHYBA: Soubor nelze otevrit - client\n");
                    printf("--------------------------------------\n");
                    continue;
                }
                else
                {
                    soubor_otevren = 1;
                }
            }

            if (ascii == 1)
            {
            	realFileSize = findRealFileLenth(fp);
            	sprintf(str_file_len, "%d", realFileSize);
            	zbyva_bytu = realFileSize;
            }

            if (octet == 1)
            {
                fseek(fp, 0L, SEEK_END);
                file_len = ftell(fp);
                realFileSize = file_len;
                sprintf(str_file_len, "%d", realFileSize);
                rewind (fp);
            }

            // vytvoreni WRQ packetu
            requestPacket.opcode = htons(param_w_r);
            sprintf(requestPacket.msg, "%s%c%s%c%s%c%s%c%s%c%s%c%s%c%s%c",
                    par_str_path, '\0',         // filepath
                    par_str_mode, '\0',         // mode prenosu
                    "blksize", '\0',            // blocksize
                    str_final_blocksize, '\0',  // hodnota blocksize
                    "timeout", '\0',            // timeout
                    str_final_timeout, '\0',    // hodnota timeout
                    "tsize", '\0',              // tsize
                    str_file_len, '\0');        // WRQ - hodnota tsize


            // vypocet delky zpravy
            int msg_len = 2+strlen(par_str_path)+
            strlen(par_str_mode)+
            strlen("blksize")+
            strlen(str_final_blocksize)+
            strlen("timeout")+
            strlen(str_final_timeout)+
            strlen("tsize")+
            strlen(str_file_len)+8;

            if (ipv6 > 0)
            {
                // poslu zpravu na server
                if ((retc = sendto(s,(void *)&requestPacket, msg_len, 0, (struct sockaddr *)&sin6, sizeof(sin6))) < 0)
                {
                    fprintf(stderr, "CHYBA: Nepodarilo se odeslat request na server\n");
                    printf("--------------------------------------\n");
                    continue;
                }
            }
            else // ipv4
            {
                 // poslu zpravu na serveru
                if ((retc = sendto(s,(void *)&requestPacket, msg_len, 0, (struct sockaddr *)&sin, sizeof(sin))) < 0)
                {
                    fprintf(stderr, "CHYBA: Nepodarilo se odeslat request na server\n");
                    printf("--------------------------------------\n");
                    continue;
                }
            }

            // nastaveni timeoutu
            if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0)
            {
                 fprintf(stderr,"CHYBA: Nepodarilo se vytvorit timeout\n");
                 printf("--------------------------------------\n");
                 continue;
            }

            int sendBytes = 0;

            sprintf(msgToPrint, "%s %s:%d", "Requesting WRITE to server", addr, port);
            tiskniCas(msgToPrint);
            memset(msgToPrint, 0, sizeof(msgToPrint));
            printf("\n");

            while(1)
            {
                if (retry == 0)
                {
                    fprintf(stderr, "CHYBA: Vyprsel timeout\n");
                    printf("--------------------------------------\n");
                    break;
                }

                bytes = recvfrom(s, buffer, DEFAULT_BUFLEN , 0, (struct sockaddr *)&address, &address_len);
                if (bytes < 0)
                {
                    retry--;
                    continue;
                }

                // opcode 6 znamena ze prisel OACK
                if ((htons(dataPacket->opcode)) == 6)
                {
                    if ((retc = checkOptions(buffer, klient_blocksize, timeout, &velikostSouboru, &server_ack_block, &server_ack_timeout)) < 0)
                    {
                        break;
                    }

                    tv.tv_sec = server_ack_timeout;
                    tv.tv_usec = 0;
                    // nastaveni timeoutu
                    if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv,sizeof(tv)) < 0)
                    {
                        fprintf(stderr,"CHYBA: Nepodarilo se vytvorit timeout\n");
                        printf("--------------------------------------\n");
                        break;
                    }

                    // kontrola velikosti souboru
                    if (velikostSouboru != realFileSize)
                    {
                        fprintf(stderr, "CHYBA: Server nepotvrdil velikost zasilaneho souboru\n");
                        break;
                    }

                    // vytvoreni DATA packetu
                    dataPacket->opcode = htons(3); // ACK je pozadavek 4
                    dataPacket->block = htons(block);

                    if (ascii == 1)
                    {
                        if (zbyva_bytu < server_ack_block)
                        {
                            while_end = zbyva_bytu;
                        }
                        else
                        {
                            while_end = server_ack_block;
                        }

                        while (charCount != while_end)
                        {
                            if (znak == '\n' && prev_char == '\r')
                            {
                                dataPacket->data[charCount] = znak;
                                prev_char = znak;
                                charCount++;
                                continue;
                            }

                            fread(&znak, 1, 1, fp);

                            if (znak == '\n')
                            {
                                dataPacket->data[charCount] = '\r';
                                prev_char = '\r';
                                charCount++;
                                continue;
                            }
                            else
                            {
                                dataPacket->data[charCount] = znak;
                                prev_char = znak;
                                charCount++;
                            }
                        }
                    }
                    else if (octet == 1)
                    {
                        charCount = fread(dataPacket->data, 1, server_ack_block, fp);
                    }

                    if ((retc = sendto(s, (void *)dataPacket, charCount + 4, 0, (struct sockaddr *)&address, address_len)) < 0)
                    {
                        printf("\n");
                        fprintf(stderr, "CHYBA: Nepodarilo se odeslat DATA packet\n");
                        printf("--------------------------------------\n");
                        retry--;
                        continue;
                    }
                    retry = 10;
                    sendBytes += charCount;
                    sprintf(msgToPrint, "%s %d B of %d B", "Sending DATA ... ", sendBytes, realFileSize);
                    tiskniCas(msgToPrint);
                    printf("\r");
                    memset(msgToPrint, 0, sizeof(msgToPrint));
                    zbyva_bytu -= charCount;
                    charCount = 0;

                    continue;
                }

                if ((htons(dataPacket->opcode)) == 4)
                {
                    if (ntohs(dataPacket->block) != block)
                    {
                        printf("\n");
                        fprintf(stderr, "CHYBA: ACK prisel s jinou hodnotou block\n");
                        printf("--------------------------------------\n");
                        break;
                    }

                    // vytvoreni DATA packetu
                    block++;
                    dataPacket->opcode = htons(3); // ACK je pozadavek 4
                    dataPacket->block = htons(block);

                    if (ascii == 1)
                    {
                        if (zbyva_bytu < server_ack_block)
                        {
                            while_end = zbyva_bytu;
                        }
                        else
                        {
                            while_end = server_ack_block;
                        }

                        while (charCount != while_end)
                        {
                            if (znak == '\n' && prev_char == '\r')
                            {
                                dataPacket->data[charCount] = znak;
                                prev_char = znak;
                                charCount++;
                                continue;
                            }

                            fread(&znak, 1, 1, fp);

                            if (znak == '\n')
                            {
                                dataPacket->data[charCount] = '\r';
                                prev_char = '\r';
                                charCount++;
                                continue;
                            }
                            else
                            {
                                dataPacket->data[charCount] = znak;
                                prev_char = znak;
                                charCount++;
                            }
                        }
                    }
                    else if (octet == 1)
                    {
                        charCount = fread(dataPacket->data, 1, server_ack_block, fp);
                    }

                    // poslu DATA packet
                    if ((retc = sendto(s, (void *)dataPacket, charCount + 4, 0, (struct sockaddr *)&address, address_len)) < 0)
                    {
                        printf("\n");
                        fprintf(stderr, "CHYBA: Nepodarilo se odeslat DATA packet\n");
                        printf("--------------------------------------\n");
                        retry--;
                        continue;
                    }
                    retry = 10;

                    sendBytes += charCount;
                    sprintf(msgToPrint, "%s %d B of %d B", "Sending DATA ... ", sendBytes, realFileSize);
                    tiskniCas(msgToPrint);
                    printf("\r");
                    memset(msgToPrint, 0, sizeof(msgToPrint));
                }

                if (charCount < server_ack_block)
                {
                    printf("\n");
                    sprintf(msgToPrint, "%s", "Transfer completed without errors");
                    tiskniCas(msgToPrint);
                    memset(msgToPrint, 0, sizeof(msgToPrint));
                    printf("\n");
                    printf("--------------------------------------\n");
                    break;
                }
                zbyva_bytu -= charCount;
                charCount = 0;
            }
        }

        if (soubor_otevren > 0)
        {
            fclose(fp);
        }

        // uzavru socket
        if (close(s) < 0)
        {
            fprintf(stderr, "CHYBA: Nepodarilo se zavrit socket s\n");
            printf("--------------------------------------\n");
            continue;
        }

        freeaddrinfo(res); // uvoleni pomocneho ukzatale

    }// konec smycky

    return 0;
}

