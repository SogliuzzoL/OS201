# TP1 – Structure d’un système de fichiers et organisation mémoire

**Objectif**
L’objectif de ce TP est d’apprendre à analyser (parser) un fichier binaire contenant l’image d’un système de fichiers au format **ROMFS**.

---

## 1. Exploration d’un système de fichiers

### 1.1 Préparation

Créez un nouveau fichier C contenant le code suivant :

```c
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>

void decode(struct fs_header *p, size_t size) {
    // Décodage à implémenter
}

int main(void) {
    int fd = open("fs.romfs", O_RDONLY);
    assert(fd != -1);

    off_t fsize = lseek(fd, 0, SEEK_END);

    char *addr = mmap(NULL, fsize, PROT_READ, MAP_SHARED, fd, 0);
    assert(addr != MAP_FAILED);

    decode((struct fs_header *)addr, fsize);
    return 0;
}
```

Ce programme :

* ouvre le fichier `fs.romfs`,
* obtient sa taille,
* le mappe dans l’espace d’adressage du processus (chaque octet du fichier est alors accessible via `addr[i]`).

---

### 1.2 Format global du système de fichiers

Le format ROMFS commence par :

| Offset | Contenu                                                     |
| ------ | ----------------------------------------------------------- |
| 0      | ASCII `-rom`                                                |
| 4      | ASCII `1fs-`                                                |
| 8      | Taille totale du FS (en octets)                             |
| 12     | Checksum (des 512 premiers octets)                          |
| 16     | Nom du volume terminé par `\0` (padded à un multiple de 16) |
| xx     | File headers (en-têtes des fichiers)                        |

Précisions :

* **Padded to 16 byte boundary** : le nom du volume est complété de zéros pour que le premier file header commence à un offset multiple de 16.
* Tous les entiers multi-octets sont en **big-endian**.

---

### 1.3 Travail demandé sur l’en-tête du FS

1. **Structure `fs_header`**
   Définir une structure C reflétant exactement ces offsets (ex. champ `checksum` de 4 octets à l’offset 12). Utiliser `<stdint.h>` et les types `uint8_t`, `uint16_t`, `uint32_t` pour garantir les tailles.

2. **Vérification du magic number**
   Vérifier que les 8 premiers octets du fichier correspondent à `"-rom1fs-"`.

3. **Lecture d’un entier 32 bits big-endian**
   Écrire une fonction :

   ```c
   uint32_t read32(char ptr[4]);
   ```

   qui lit un mot 32 bits en big-endian. Utilisez-la pour vérifier que la taille du FS dans l’en-tête est ≤ la taille réelle du fichier.

4. **Arrondi au multiple de 16**
   Écrire une fonction qui arrondit un nombre au multiple de 16 supérieur (ou le laisse inchangé si déjà multiple de 16).
   Utilisez-la pour calculer l’offset du premier file header après le nom du volume.

---

## 2. Format des fichiers

Chaque fichier est décrit par un **file header** :

| Offset | Contenu                                          |
| ------ | ------------------------------------------------ |
| 0      | Offset du header suivant + infos mode (bits 0–3) |
| 4      | `spec.info` (infos dir/liens/devices)            |
| 8      | Taille du fichier en octets                      |
| 12     | Checksum (métadonnées + nom + padding)           |
| 16     | Nom du fichier terminé par `\0` (padded à 16)    |
| xx     | Données du fichier                               |

Remarques :

* Les headers commencent toujours à un multiple de 16.
* Les 4 bits de poids faible du champ « next filehdr » codent le type et les permissions.
* L’offset du header suivant est relatif au début du FS.

---

### 2.1 Travail demandé sur les file headers

5. **Structure de données**
   Définir une structure C correspondant à ce format.

6. **Affichage du contenu d’un répertoire**
   Écrire une fonction `ls` qui parcourt la liste chaînée des file headers d’un répertoire et affiche leurs noms.
   Supprimez d’abord les 4 bits de poids faible du champ « next filehdr » avant de suivre le pointeur.
   Appliquez `ls` au premier file header (racine).

7. **Recherche d’un fichier**
   Écrire une fonction `find` qui parcourt récursivement le FS pour chercher un fichier dont le nom est donné. Elle renvoie le file header du premier fichier trouvé.
   Les fichiers spéciaux `.` et `..` pointent respectivement sur le répertoire courant et le parent.

---

### 2.2 Types de fichiers

Les 3 bits de poids faible du champ « next filehdr » indiquent le type de fichier :

| Valeur | Type          | `spec.info`               |
| ------ | ------------- | ------------------------- |
| 0      | Hard link     | destination du lien       |
| 1      | Directory     | header du premier fichier |
| 2      | Regular file  | doit être 0 (MBZ)         |
| 3      | Symbolic link | MBZ, données = cible      |
| 4      | Block device  | n° majeur/minor 16/16     |
| 5      | Char device   | n° majeur/minor 16/16     |
| 6      | Socket        | MBZ                       |
| 7      | FIFO          | MBZ                       |

---

### 2.3 Application

8. Utilisez votre fonction `find` pour chercher le fichier `message.txt`, affichez son contenu et montrez la réponse à votre encadrant.

---

## Résumé des étapes

1. Créer le squelette du programme (fichier C, `decode`).
2. Définir `fs_header`, vérifier les premiers octets.
3. Écrire `read32` pour lire en big-endian.
4. Écrire l’arrondi au multiple de 16 pour trouver le premier file header.
5. Définir la structure du file header.
6. Écrire `ls` pour afficher les fichiers d’un répertoire.
7. Écrire `find` pour chercher un fichier.
8. Tester sur `message.txt`.