## **TP2 : Gestion de la mémoire – Le tas (heap)**
**Objectif** : Implémenter deux fonctions, `memalloc` et `memfree`, qui simulent le fonctionnement de `malloc` et `free` en C, **sans utiliser les fonctions standards**. L’idée est de gérer manuellement un espace mémoire de 16 Ko (le "tas") où chaque octet est soit **libre** (disponible pour une future allocation), soit **alloué** (réservé par un appel à `memalloc`).

---

### **1. Allocation mémoire triviale (sans libération)**
**But** : Créer une première version de `memalloc` qui alloue de la mémoire **sans jamais la libérer**. Utile pour des programmes qui allouent tout au début et n’ont plus besoin de libérer ensuite.

#### **À faire** :
- **Allouer statiquement** un tableau de 16 Ko (16 × 1024 octets) : ce sera ton "tas".
- **Gérer une seule zone libre** : Au début, tout le tas est libre. Chaque allocation prend un morceau de cette zone, qui rétrécit.
- **Gérer les erreurs** : Si la taille demandée dépasse la mémoire libre, afficher une erreur et quitter le programme.
- **`memfree` ne fait rien** : Dans cette version, la libération n’est pas implémentée.

**Exemple** :
```c
char heap[16 * 1024]; // Tas de 16 Ko
size_t free_space = sizeof(heap); // Espace libre initial

void *memalloc(int size) {
    if (size > free_space) {
        printf("Erreur : plus de mémoire disponible !\n");
        exit(1);
    }
    void *ptr = &heap[sizeof(heap) - free_space]; // Pointeur vers la zone libre
    free_space -= size;
    return ptr;
}

void memfree(void *ptr) {
    // Ne fait rien dans cette version
}
```

---

### **2. Allocation mémoire avec liste chaînée**
**But** : Implémenter un allocateur complet qui gère **l’allocation ET la libération**, en utilisant une **liste chaînée des zones libres** (free list).

#### **2.1 Initialisation (`meminit`)**
- **Au démarrage** : Le tas contient une seule zone libre de 16 Ko.
- **Structure d’une zone libre** : Chaque zone libre commence par :
  - Un champ `size` (taille de la zone).
  - Un pointeur `next` vers la zone libre suivante.

**À faire** :
Écrire `void meminit(void)` pour initialiser le tas avec une seule zone libre.

---

#### **2.2 Allocation (`memalloc` avec stratégie "first-fit")**
**Stratégie "first-fit"** : Parcourir la free list et prendre la **première zone assez grande** pour satisfaire la demande.
- Si la zone trouvée est **plus grande** que nécessaire, la scinder en deux :
  - Une partie allouée (taille demandée + métadonnées).
  - Une partie libre (le reste).
- Si aucune zone n’est assez grande, retourner `NULL`.

**À faire** :
- Parcourir la free list pour trouver une zone libre.
- Scinder la zone si nécessaire.
- Retirer la zone allouée de la free list.

---

#### **2.3 Libération (`memfree`) et métadonnées**
**Problème** : `memfree` ne connaît pas la taille de la zone à libérer. Il faut donc **stocker cette information** dans des métadonnées.
**Solution** :
- **Allouer plus que demandé** : `taille demandée + taille des métadonnées`.
- **Stocker les métadonnées** au début de la zone allouée (ex : taille, flag "libre/alloué").
- **Retourner un pointeur décalé** : L’utilisateur écrit après les métadonnées.

**À faire** :
1. Modifier `memalloc` pour ajouter les métadonnées.
2. Implémenter `memfree` :
   - Retrouver le début de la zone à partir du pointeur utilisateur.
   - Réinsérer la zone dans la free list.

**Risques** :
- Libérer un pointeur déjà libéré → **corruption mémoire**.
- Libérer une zone non allouée → **comportement imprévisible**.

---

#### **2.4 Coalescence (fusion des zones libres)**
**Problème** : La **fragmentation** (zones libres non contiguës) réduit l’efficacité du tas.
**Solution** : Fusionner les zones libres adjacentes lors de la libération.

**À faire** :
- Lors de `memfree`, parcourir la free list pour vérifier si la zone **précédente ou suivante** est libre.
- Si oui, fusionner les zones.

**Limitations** :
- La coalescence simple ne fusionne que les zones **directement adjacentes**.
- **Améliorations possibles** :
  - Parcourir toute la free list à chaque libération (coûteux).
  - Utiliser des **listes séparées** par taille de zone.

---

### **Résumé des étapes clés**
| Étape | Tâche | Fonctions à implémenter |
|-------|-------|-------------------------|
| 1     | Allocation triviale | `memalloc` (sans libération) |
| 2.1   | Initialisation du tas | `meminit` |
| 2.2   | Allocation first-fit | `memalloc` (avec free list) |
| 2.3   | Libération + métadonnées | `memfree` |
| 2.4   | Coalescence | Modifier `memfree` |

---

### **Exemple de structure de zone libre/allouée**
```c
typedef struct free_block {
    size_t size;          // Taille de la zone (incluant cette structure)
    struct free_block *next; // Pointeur vers la zone libre suivante
} free_block;

// Pour une zone allouée :
typedef struct alloc_block {
    size_t size;          // Taille de la zone allouée
    char data[1];         // Début des données utilisateur (flexible array)
} alloc_block;
```

---

### **Questions pour vérifier ta compréhension**
1. **Allocation triviale** : Comment gères-tu l’espace libre ? Que se passe-t-il si on alloue 17 Ko ?
2. **Free list** : Pourquoi utiliser une liste chaînée pour les zones libres ?
3. **Métadonnées** : Où sont-elles stockées ? Pourquoi ne pas les mettre à la fin de la zone ?
4. **Coalescence** : Quel est l’avantage de fusionner les zones libres ?

---
### **Conseils pour coder**
- Utilise des `#define` pour les tailles (ex : `#define HEAP_SIZE (16 * 1024)`).
- Dessine un schéma du tas pour visualiser les zones libres/allouées.
- Teste avec des allocations/libérations simples avant de passer à la coalescence.
