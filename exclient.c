#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>

/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;

int main (int argc, char *argv[])
{
  int sd;			// descriptorul de socket
  struct sockaddr_in server;	// structura folosita pentru conectare 
  		// mesajul trimis
  int nr=0;
  char comanda[50], raspuns[1000], mesaj[500];
  char nume_user[25];
  char parola[25];
  char destinatar[25];
   int exista;
  /* exista toate argumentele in linia de comanda? */
  if (argc != 3)
    {
      printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
      return -1;
    }

  /* stabilim portul */
  port = atoi (argv[2]);

  /* cream socketul */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("Eroare la socket().\n");
      return errno;
    }

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr(argv[1]);
  /* portul de conectare */
  server.sin_port = htons (port);
  
  /* ne conectam la server */
  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
      perror ("[client]Eroare la connect().\n");
      return errno;
    }
    
  int autentificat;
  /* citirea mesajului */
  while(1){
    if (read (sd, &autentificat, sizeof(autentificat)) < 0){
          perror ("[client]Eroare la citire stare client de la server.\n");
          return errno;
        }
    if(autentificat == 0){
      printf("\n\nOFFLINE MESSENGER\n\n");
      printf("--- Bine ati venit! ---\n");
      printf("- Alegeti una din optiuni: \nInregistrare \nAutentificare \nIesire \n\n~ ~ ~\n\n");
    }
    else printf("\n- Alegeti una din optiuni: \nTrimite_Mesaj \nDeconectare \nIesire \n\n~ ~ ~\n\n");
    bzero(comanda, sizeof(comanda));
    fflush (stdout);
    scanf("%s", comanda);
     // trimiterea mesajului la server 
    if(write(sd, &comanda, sizeof(comanda)) <= 0){
        perror ("[client]Eroare la write() spre server.\n");
        return errno;
    }
  system("clear");
    if (strcmp(comanda, "Inregistrare") == 0)
        {
            printf("\n- Completati datele necesare crearii contului dumneavoastra.");
            printf("\n- Numele de utilizator nu trebuie sa contina spatii.\n\n");
            printf("-- Nume utilizator:");
            bzero(nume_user, sizeof(nume_user));
            bzero(parola, sizeof(parola));
            fflush (stdout);
            scanf("%s", nume_user);
            printf("-- Parola:");
            scanf("%s", parola);
            printf("\n");
            if(write(sd, &nume_user, sizeof(nume_user)) <= 0){
                    perror ("[client]Eroare la trimitere username spre server.\n");
                    return errno;
                }
            if(write(sd, &parola, sizeof(parola)) <= 0){
                    perror ("[client]Eroare la trimitere parola spre server.\n");
                    return errno;
                }
            printf("\n\n");
            bzero(raspuns, sizeof(raspuns));
            fflush (stdout);
            if (read (sd, &raspuns, sizeof(raspuns)) < 0)
            {
              perror ("[client]Eroare la read() de la server.\n");
              return errno;
            }
            else printf("%s\n", raspuns);
        }
    else if(strcmp(comanda, "Autentificare") == 0){
        printf("\n- Completati datele corespunzatoare contului dumneavoastra.\n\n");
        printf("-- Nume utilizator:");
        bzero(nume_user, sizeof(nume_user));
        bzero(parola, sizeof(parola));
        fflush (stdout);
        scanf("%s", nume_user);
        printf("-- Parola:");
        scanf("%s", parola);
        printf("\n");
  //trimitem serverului usernameul si parola
        if(write(sd, &nume_user, sizeof(nume_user)) <= 0){
          perror ("[client]Eroare la trimitere username spre server.\n");
          return errno;
          }
        if(write(sd, &parola, sizeof(parola)) <= 0){
          perror ("[client]Eroare la trimitere parola spre server.\n");
          return errno;
        }
        printf("\n\n");
        bzero(raspuns, sizeof(raspuns));//sunt deja autentificat
        fflush (stdout);
        if (read (sd, &raspuns, sizeof(raspuns)) < 0){
          perror ("[client]Eroare la read() de la server.\n");
          return errno;
        }
          else printf("%s\n", raspuns);
/*
        bzero(raspuns, sizeof(raspuns));//date corecte(autentificare reusita) sau nu
        fflush (stdout);
        if (read (sd, &raspuns, sizeof(raspuns)) < 0){
          perror ("[client]Eroare la read() de la server.\n");
          return errno;
        }
          else printf("%s\n", raspuns);
  */
    }
    else if(strcmp(comanda, "Deconectare") == 0)
        {
          printf("\n");
          bzero(raspuns, sizeof(raspuns));//a avut loc autentif sau incearca iar
          fflush (stdout);
          if (read (sd, &raspuns, sizeof(raspuns)) < 0){
            perror ("[client]Eroare la read() de la server.\n");
            return errno;
          }
          else printf("%s\n", raspuns);
        }
    else if(strcmp(comanda, "Iesire") == 0)
        {
          bzero(raspuns, sizeof(raspuns));
          strcpy(raspuns, "Bye-bye!\n");
          if(write(sd, &raspuns, sizeof(raspuns)) <= 0){
            perror ("[client]Eroare la informare server despre iesire.\n");
            return errno;
          }
          close(sd);
          return 0;
        }
    else if(strcmp(comanda, "Trimite_Mesaj") == 0){
      printf("-- Nume destinatar:");
      bzero(destinatar, sizeof(destinatar));
      scanf("%s", destinatar);
      printf("\n");
      if(write(sd, &destinatar, sizeof(destinatar)) <= 0){
          perror ("[client]Eroare la trimitere nume destinatar spre server.\n");
          return errno;
          }
     
      if (read (sd, &exista, sizeof(exista)) < 0){
        perror ("[client]Eroare la read() de la server.\n");
        return errno;
      }
      else if(exista == 0) {
        printf("Acest utilizator nu exista! \n");
        //un write??????????????
      }
      else if(exista == 1){
        //destinatarul exista, mesajul poate fi transmis
        printf("Destinatar identificat cu succes.\n\n");
        printf("Introduceti mesajul: \n");
        bzero(mesaj, 500);
        read(0, &mesaj, 500);
        if(write(sd, &mesaj, sizeof(mesaj)) <= 0){
          perror ("[client]Eroare la trimitere mesaj\n");
          return errno;
          }
      }

          
    }//if trimite mesaj
    else{//comanda nerecunoscuta
      bzero(raspuns, sizeof(raspuns));
			fflush (stdout);
      if (read (sd, &raspuns, sizeof(raspuns)) < 0){
            perror ("[client]Eroare la read() de la server.\n");
            return errno;
          }
          else printf("%s\n", raspuns);
    }
  }
  close (sd);
}
