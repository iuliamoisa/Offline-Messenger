#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>

extern int errno;//codul de eroare returnat de anumite apeluri 
int port;

int main (int argc, char *argv[])
{
  int sd;			// descriptorul de socket
  struct sockaddr_in server;	// structura folosita pentru conectare 
  int nr = 0;
  char comanda[50], raspuns[1000], mesaj[500];
  char nume_user[25];
  char parola[25];
  char destinatar[25], idMesaj[25];
   int exista, da = 0, nr_useri = 0, ok2, idOK = 0;;
  if (argc != 3)
    {
      printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
      return -1;
    }
  port = atoi (argv[2]);  // stabilim portul 
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("Eroare la socket().\n");
      return errno;
    }

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr(argv[1]);//adresa IP a serverului */
  server.sin_port = htons (port);
  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
      perror ("[client]Eroare la connect().\n");
      return errno;
    }
  int autentificat = 0;
  while(1){
    if (read (sd, &autentificat, sizeof(autentificat)) < 0){
          perror ("[client]Eroare la citire stare client de la server.\n");
          return errno;
        }
    if(autentificat == 0){
      printf("\n\nOFFLINE MESSENGER\n\n");
      printf("--- Bine ati venit! ---\n");
      printf("- Alegeti una din optiuni: \n\n~ Inregistrare \n~ Autentificare \n~ Iesire \n\n~ ~ ~\n\n");
    }
    else printf("\n- Alegeti una din optiuni: \n\n~ Trimite_Mesaj \n~ Utilizatori_Online \n~ Utilizatori \n~ Afiseaza_Istoric\n~ Raspunde_La_Mesaj\n~ Deconectare \n~ Iesire \n\n~ ~ ~\n\n");
    bzero(comanda, sizeof(comanda));
    fflush (stdout);
    scanf("%s", comanda);
     // trimiterea mesajului la server 
    if(write(sd, &comanda, sizeof(comanda)) <= 0){
        perror ("[client]Eroare la write() spre server.\n");
        return errno;
    }
  //system("clear");
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
        if(write(sd, &nume_user, sizeof(nume_user)) <= 0){
          perror ("[client]Eroare la trimitere username spre server.\n");
          return errno;
          }
        if(write(sd, &parola, sizeof(parola)) <= 0){
          perror ("[client]Eroare la trimitere parola spre server.\n");
          return errno;
        }
        printf("\n\n");
        if (read (sd, &da, sizeof(da)) < 0){//pot primi mesaj sau nu
          perror ("[client]Eroare la read() de la server.\n");
          return errno;
        }
          //else printf("Pot primi mesaje: %d\n", da);

        bzero(raspuns, sizeof(raspuns));//sunt deja autentificat?
        fflush (stdout);
        if (read (sd, &raspuns, sizeof(raspuns)) < 0){
          perror ("[client]Eroare la read() de la server.\n");
          return errno;
        }
          else printf("%s\n", raspuns);

          if(da == 1){
            bzero(raspuns, sizeof(raspuns));
            fflush (stdout);
            if (read (sd, &raspuns, sizeof(raspuns)) < 0){
              perror ("[client]Eroare la read() de la server.\n");
              return errno;
            }
              else printf("%s\n", raspuns);
        }

    }
    else if(strcmp(comanda, "Utilizatori") == 0 || strcmp(comanda, "Utilizatori_Online") == 0){
      if (read (sd, &nr_useri, sizeof(nr_useri)) < 0){
          perror ("[client]Eroare la read() de la server.\n");
          return errno;
        }
          else printf("-- Numar utilizatori: %d\n", nr_useri);
      if(nr_useri != 0){
        bzero(raspuns, sizeof(raspuns));
        fflush (stdout);
        if (read (sd, &raspuns, sizeof(raspuns)) < 0){
          perror ("[client]Eroare la read() de la server.\n");
          return errno;
        }
          else printf("%s\n", raspuns);
      }
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
      }
      else if(exista == 1){ //destinatarul exista, mesajul poate fi transmis
        printf("Destinatar identificat cu succes.\n\n");
        printf("- Introduceti mesajul: \n");
        bzero(mesaj, 500);
        read(0, &mesaj, 500);
        mesaj[strlen(mesaj) - 1] = '\0';
        if(write(sd, &mesaj, sizeof(mesaj)) <= 0){
          perror ("[client]Eroare la trimitere mesaj\n");
          return errno;
          }
      }    
    }//if trimite mesaj
    else if(strcmp(comanda, "Afiseaza_Istoric") == 0){
       printf("-- Introduceti numele utilizatorului cu care vreti sa vedeti istoricul conversatiilor:");
      bzero(nume_user, sizeof(nume_user));
      fflush (stdout);
      scanf("%s", nume_user);
      printf("\n");

      if(write(sd, &nume_user, sizeof(nume_user)) <= 0){
        perror ("[client]Eroare la trimitere username spre server.\n");
        return errno;
      }
      da = 0;
      if (read (sd, &da, sizeof(da)) < 0){//pot vedea istoricul sau nu
          perror ("[client]Eroare la read() de la server.\n");
          return errno;
        }
      if(da == 1){
            bzero(raspuns, sizeof(raspuns));
            fflush (stdout);
            if (read (sd, &raspuns, sizeof(raspuns)) < 0){
              perror ("[client]Eroare la read() de la server.\n");
              return errno;
            }
              else printf("%s\n", raspuns);
      }
    
    }
    else if(strcmp(comanda, "Raspunde_La_Mesaj") == 0){
      printf("-- Introduceti numele utilizatorului caruia vreti sa ii raspundeti: ");
      bzero(nume_user, sizeof(nume_user));
      fflush (stdout);
      scanf("%s", nume_user);
      printf("\n");

      if(write(sd, &nume_user, sizeof(nume_user)) <= 0){
        perror ("[client]Eroare la trimitere username spre server.\n");
        return errno;
      }
      ok2 = 0;
      if (read (sd, &ok2, sizeof(ok2)) < 0){//exista userul sau nu am cui sa ii rasp basically
          perror ("[client]Eroare la read() de la server.\n");
          return errno;
        }
      if(ok2 == 1){ //am cui raspunde
            bzero(raspuns, sizeof(raspuns));
            fflush (stdout);
            if (read (sd, &raspuns, sizeof(raspuns)) < 0){//comanda
              perror ("[client]Eroare la rcitire comanda de la server.\n");
              return errno;
            }
            else printf("%s\n", raspuns);

        bzero(idMesaj, sizeof(idMesaj));
        read(0, &idMesaj, sizeof(idMesaj));//idul mesajului
        idMesaj[strlen(idMesaj) - 1] = '\0';
        if(write(sd, &idMesaj, sizeof(idMesaj)) <= 0){
          perror ("[client]Eroare la trimitere idMesaj\n");
          return errno;
          }
          
          if (read (sd, &idOK, sizeof(idOK)) < 0){
            perror ("[client]Eroare la read() de la server.\n");
            return errno;
          }
          //printf("ID valid: %d\n", idOK);
        if(idOK == 1){//trimit mesajul!!
            printf("- Introduceti mesajul: \n");
            
            bzero(mesaj, sizeof(mesaj));
            read(0, &mesaj, sizeof(mesaj));
            mesaj[strlen(mesaj) - 1] = '\0';
            printf("reply === %s\n", mesaj);
            if(write(sd, &mesaj, sizeof(mesaj)) <= 0){
              perror ("[client]Eroare la trimitere mesaj\n");
              return errno;
              }
            
          }
          else printf("Numarul introdus nu este valid!\n");
      }
      else //nu am cui raspunde 
         printf("- Nu s-a gasit vreun utilizator cu acest nume.\n");
    }//if princp reply
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
