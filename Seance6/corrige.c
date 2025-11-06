#include <linux/seccomp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Pour strlen
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/wait.h> // Pour waitpid
#include <unistd.h>

#define BUFFER_SIZE 1024

/**
 * @brief C'est la fonction que le processus fils sandoxé va exécuter.
 * Elle lit une seule commande depuis son stdin (qui sera un pipe),
 * la traite, et écrit le résultat sur son stdout (l'autre pipe).
 */
void run_interpreter() {
  // 1. Appliquer le sandbox STICTEMENT
  // Seuls read, write, et exit (via syscall) sont autorisés
  prctl(PR_SET_SECCOMP, SECCOMP_MODE_STRICT);

  char buffer[BUFFER_SIZE];
  ssize_t bytes_read =
      read(0, buffer, BUFFER_SIZE - 1); // Lit depuis le pipe parent

  if (bytes_read <= 0) {
    syscall(SYS_exit, 1); // Quitte si erreur de lecture
  }
  buffer[bytes_read] = '\0'; // Assure la terminaison de la chaîne

  // 2. Logique de l'interpréteur
  if (buffer[0] == '+') {
    int a, b;
    char result[32];
    if (sscanf(buffer, "+%d,%d", &a, &b) == 2) {
      int len = sprintf(result, "%d\n", a + b);
      write(1, result, len); // Écrit le résultat dans le pipe vers le parent
    }
  } else if (buffer[0] == '-') { // Ajout de la commande de Q1 [cite: 21]
    int a, b;
    char result[32];
    if (sscanf(buffer, "-%d,%d", &a, &b) == 2) {
      int len = sprintf(result, "%d\n", a - b);
      write(1, result, len); // Écrit le résultat dans le pipe vers le parent
    }
  } else if (buffer[0] == 'e') {
    // Cette commande va être tentée [cite: 30]
    system(buffer + 1);

    // IMPORTANT: L'appel system() ci-dessus va échouer.
    // system() a besoin d'appels système (comme fork, execve, wait)
    // qui sont INTERDITS par SECCOMP_MODE_STRICT.
    // Le noyau va tuer ce processus fils (SIGKILL ou SIGSYS).
    // Il n'atteindra jamais la ligne suivante.
    // C'est exactement la protection désirée.
    char *msg = "ECHEC SECCOMP\n";
    write(1, msg, strlen(msg)); // N'sera jamais atteint
  }

  // 3. Quitter proprement (obligatoire avec seccomp strict)
  syscall(SYS_exit, 0);
}

int main(int argc, char *argv[]) {
  int pipe_to_fils[2];   // Parent écrit [1], Fils lit [0]
  int pipe_to_parent[2]; // Fils écrit [1], Parent lit [0]

  // 1. Créer les DEUX pipes [cite: 47]
  if (pipe(pipe_to_fils) == -1 || pipe(pipe_to_parent) == -1) {
    perror("pipe");
    exit(EXIT_FAILURE);
  }

  // 2. Créer le processus fils [cite: 49]
  pid_t pid = fork();

  if (pid == -1) {
    perror("fork");
    exit(EXIT_FAILURE);
  }

  if (pid == 0) {
    // --- PROCESSUS FILS --- [cite: 50]

    // 3. Fermer les bouts de pipe inutilisés par le fils
    close(pipe_to_fils[1]);   // Le fils ne écrit pas ici
    close(pipe_to_parent[0]); // Le fils ne lit pas ici

    // 4. Rediriger stdin (0) et stdout (1)
    dup2(pipe_to_fils[0],
         0); // Le stdin du fils devient le pipe venant du parent
    dup2(pipe_to_parent[1],
         1); // Le stdout du fils devient le pipe allant au parent

    // 5. Fermer les anciens descripteurs (maintenant dupliqués)
    close(pipe_to_fils[0]);
    close(pipe_to_parent[1]);

    // 6. Exécuter l'interpréteur sandoxé
    run_interpreter(); // Cette fonction ne retournera jamais

  } else {
    // --- PROCESSUS PARENT ---

    char buffer[BUFFER_SIZE];
    char result_buffer[BUFFER_SIZE];

    // 3. Fermer les bouts de pipe inutilisés par le parent
    close(pipe_to_fils[0]);   // Le parent ne lit pas ici
    close(pipe_to_parent[1]); // Le parent n'écrit pas ici

    // 4. Lire la commande depuis le *vrai* stdin (fd 0)
    ssize_t bytes_read = read(0, buffer, BUFFER_SIZE - 1);
    if (bytes_read <= 0) {
      // Pas d'entrée, on quitte
      close(pipe_to_fils[1]);
      close(pipe_to_parent[0]);
      waitpid(pid, NULL, 0);
      return 0;
    }
    buffer[bytes_read] = '\0';

    // 5. Envoyer la commande au fils via le pipe [cite: 53]
    write(pipe_to_fils[1], buffer, bytes_read);
    // Fermer le pipe après écriture (envoie EOF au fils)
    close(pipe_to_fils[1]);

    // 6. Lire le résultat du fils depuis l'autre pipe
    ssize_t result_bytes =
        read(pipe_to_parent[0], result_buffer, BUFFER_SIZE - 1);
    close(pipe_to_parent[0]); // Fermer le pipe après lecture

    // 7. Afficher le résultat sur le *vrai* stdout (fd 1)
    if (result_bytes > 0) {
      result_buffer[result_bytes] = '\0';
      write(1, result_buffer, result_bytes);
    }
    // Si la commande était 'e', result_bytes sera 0 car le fils
    // a été tué avant de pouvoir écrire quoi que ce soit.

    // 8. Attendre la fin du fils
    waitpid(pid, NULL, 0);
  }

  return 0; // Seul le parent arrive ici
}