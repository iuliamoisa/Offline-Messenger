/* cliTCPIt.c - Exemplu de client TCP
   Trimite un numar la server; primeste de la server numarul incrementat.
         
   Autor: Lenuta Alboaie  <adria@infoiasi.ro> (c)2009
*/
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
  char comanda[1000], raspuns[1000];

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

    printf("\n\nOFFLINE MESSENGER\n\n");
    printf("---Bine ati venit!---\n");
    
  /* citirea mesajului */
  while(1){
    printf("Buna! Alege una din optiuni: \nInregistrare \nAutentificare \nIesire\n");
    bzero(comanda, 1000);
        scanf("%s", comanda);
     // trimiterea mesajului la server 
    if(write(sd, comanda, sizeof(comanda)) <= 0){
        perror ("[client]Eroare la write() spre server.\n");
        return errno;
    }

    if (strcmp(comanda, "Inregistrare") == 0 || strcmp(comanda, "Autentificare") == 0)
        {
            printf("username:");
            char nume_user[20];
            char parola[25];
            scanf("%s", nume_user);
            printf("password:");
            scanf("%s", parola);
            if(write(sd, &nume_user, sizeof(nume_user)) <= 0){
                    perror ("[client]Eroare la write() spre server.\n");
                    return errno;
                }
            if(write(sd, &parola, sizeof(parola)) <= 0){
                    perror ("[client]Eroare la write() spre server.\n");
                    return errno;
                }

            bzero(raspuns, sizeof(raspuns));
            if (read (sd, raspuns, sizeof(raspuns)) < 0)
            {
              perror ("[client]Eroare la read() de la server.\n");
              return errno;
            }
        }
/*

    // citirea raspunsului dat de server 
    //  (apel blocant pina cind serverul raspunde) 
    
    // afisam mesajul primit 
    printf ("[client]Mesajul primit este: %d\n", nr);
 */
  }
 
  /* inchidem conexiunea, am terminat */
  close (sd);
}