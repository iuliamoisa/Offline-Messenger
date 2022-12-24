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
  char nume_user[25];
  char parola[25];
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
    printf("Alegeti una din optiuni: \nInregistrare \nAutentificare \nIesire\n~ ~ ~\n");
    bzero(comanda, sizeof(comanda));
    scanf("%s", comanda);
     // trimiterea mesajului la server 
    if(write(sd, &comanda, sizeof(comanda)) <= 0){
        perror ("[client]Eroare la write() spre server.\n");
        return errno;
    }

    if (strcmp(comanda, "Inregistrare") == 0 || strcmp(comanda, "Autentificare") == 0)
        {
            printf("username:");
            bzero(nume_user, sizeof(nume_user));
            scanf("%s", nume_user);
            printf("password:");
            bzero(parola, sizeof(parola));
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
            if (read (sd, &raspuns, sizeof(raspuns)) < 0)
            {
              perror ("[client]Eroare la read() de la server.\n");
              return errno;
            }
            else printf("%s\n", raspuns);
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
