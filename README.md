# Micro Kernel Daemon - Projet d'apprentissage

## Vision du projet

### Objectif

L’objectif de ce projet est de concevoir un microkernel simulé, implémenté en C et exécuté en user space.  
Il ne s’agit pas d’un véritable noyau système, mais d’un environnement d’exécution minimal reproduisant certains mécanismes fondamentaux d’un kernel.

Le microkernel devra :

- gérer ses propres tâches (tasks),
- implémenter un ordonnanceur (scheduler) interne pour déterminer quelle tâche s’exécute et à quel moment,
- permettre la communication entre tâches via des mécanismes d’IPC,
- exposer des services internes simulés (timer, console, block device),
- fournir une API de type « syscall » servant d’interface entre les tâches et le noyau.

Dans sa version initiale (V1), le système utilisera un scheduling **coopératif** de type **round-robin**, sans gestion de priorité ni préemption.  
Ce choix vise à garantir un comportement simple, maîtrisé et déterministe.

Dans ce contexte, un “programme” correspond à une fonction ou un module C enregistré dans une table interne. L’exécution d’un programme consiste à créer une task qui lance cette fonction.

Toutes les tâches partagent le même espace mémoire : aucune isolation n’est implémentée. L’objectif est d’explorer les mécanismes fondamentaux du kernel (ordonnancement, IPC, synchronisation), et non la protection mémoire ou la sécurité.

Le système devra rester simple mais reposer sur une architecture propre, clairement documentée et accompagnée de tests reproductibles.  
Le comportement devra être déterministe afin de faciliter le débogage et l’analyse.

La finalité est de pouvoir lancer `mkd`, obtenir un shell interne, créer et observer des tâches, et exécuter plusieurs services communiquant via IPC dans un environnement d’exécution simulé.

#### Hors scope (V1)

- Virtualisation ou émulation matérielle  
- Chargement et exécution de binaires externes  
- Drivers matériels réels  
- Interface graphique  
- Isolation mémoire ou mécanismes de sécurité  

---

## Philosophie

Le principe central du projet est : **Conception avant implémentation**.

Toute nouvelle fonctionnalité commence par :

1. Définition de l’API  
2. Définition des invariants  
3. Définition des transitions d’état  

L’implémentation ne commence qu’une fois ces éléments clarifiés.

---

### Discipline d’architecture

Règles structurantes :

1. Pas de dépendances circulaires  
2. Pas d’accès direct aux structures internes d’un autre module  
3. Pas de modification d’état d’une task en dehors du Core  

Le scheduler est la seule entité autorisée à modifier l’état d’une task.

---

### Déterminisme et testabilité

- Le comportement du système doit être reproductible.  
- Les scénarios de test doivent être déterministes.  
- Les transitions d’état importantes doivent être observables (logs, introspection).

---

### Progression itérative

Le développement se fera par milestones successifs.  
Chaque étape devra produire un système minimal, fonctionnel et testable.

Aucune fonctionnalité avancée (préemption, priorités, optimisation) ne sera introduite tant que la version simple n’est pas stable.

---

### Gestion de la complexité

La simplicité est prioritaire sur l’optimisation ou la complétude.  
Toute complexité introduite devra être justifiée par un besoin clair.

---

### Documentation des décisions (ADR)

Les décisions structurantes d’architecture seront documentées sous forme d’ADR (Architecture Decision Records) dans `docs/adr/`.

Chaque ADR décrit :

- le contexte,
- la décision prise,
- les alternatives considérées,
- les conséquences (trade-offs).

Une ADR est créée dès qu’une décision impacte durablement l’architecture (API, modèle de scheduling, IPC, représentation des tasks, etc.).

---

### Variables globales

Le projet évite l’usage de variables globales non maîtrisées.

L’état du microkernel est regroupé dans une structure explicite `kernel_t`, passée aux composants ou accessible via une instance clairement identifiée et encapsulée.

Toute exception doit être justifiée dans une ADR.

---

## Architecture globale

### Vue d’ensemble

#### Core

Responsable de :

- gestion des tasks (création, destruction),
- transitions d’état,
- scheduling (round-robin en V1),
- gestion de la task courante,
- gestion du contexte d’exécution,
- gestion du temps minimal.

Le Core est l’autorité centrale sur l’état des tasks.

---

#### Synchronization

Responsable de :

- mutex,
- wait queues,
- primitives de blocage.

Ne décide pas du scheduling.  
Demande au Core de bloquer ou réveiller les tasks.

---

#### IPC

Responsable de :

- canaux de communication,
- files de messages,
- logique `send` / `recv`.

S’appuie sur les wait queues pour bloquer et réveiller les tasks.

---

#### Drivers simulés

- Implémentés comme des services internes.  
- Utilisent uniquement l’API publique du kernel.  
- Ne modifient jamais directement l’état des tasks.

---

#### Platform

- Encapsulation des appels Linux (`clock_gettime`, timer, etc.).  
- Aucune logique kernel.  
- Théoriquement interchangeable.

---

### Dépendances

Dépendances autorisées :

- `core` dépend uniquement de `platform`
- `sync` dépend de `core`
- `ipc` dépend de `core` et `sync`
- `drivers` dépend de `core`, `ipc` et `sync`
- `platform` ne dépend d’aucun module kernel

Dépendances interdites :

- `core` ne dépend jamais de `ipc` ou `sync`
- `platform` ne connaît rien du kernel
- aucune dépendance circulaire

---

### États d’une task (V1)

Une task peut être dans l’un des états suivants :

- `NEW` : créée mais pas encore planifiée  
- `READY` : éligible à l’exécution  
- `RUNNING` : actuellement exécutée  
- `BLOCKED` : en attente sur une ressource  
- `TERMINATED` : exécution terminée  

Invariants :

- À tout instant, au plus une task est `RUNNING`.
- Une task `BLOCKED` appartient à exactement une wait queue.
- Une task `READY` appartient à la ready queue.
- Une task `READY` ne doit pas être `TERMINATED`
- Une task `TERMINATED` ne doit appartenir à aucune queue

---

### Scheduling coopératif (V1)

Le scheduling est coopératif : une task conserve le CPU jusqu’à ce qu’elle :

- appelle explicitement `yield()`,
- ou appelle une primitive bloquante (`sleep`, `recv`, `mutex_lock`, etc.).

`yield()` signifie “rendre volontairement la main au scheduler”.

En round-robin :

1. La task courante passe de `RUNNING` à `READY`.
2. Elle est replacée en fin de ready queue.
3. Le scheduler sélectionne la prochaine task prête.

Une task qui ne rend jamais la main est considérée comme un bug applicatif en V1.

### Fin d’exécution d’une task

Chaque task est démarrée via une fonction trampoline interne au kernel.

Cette fonction :

1. appelle la fonction utilisateur associée à la task,
2. capture son retour,
3. marque la task en `TERMINATED`,
4. déclenche un rescheduling.

Une task ne retourne jamais directement dans le scheduler sans passer par ce mécanisme contrôlé.


---

### Modèle de blocage et réveil

Toute opération bloquante :

1. Ajoute la task à une wait queue.
2. Demande au Core de la passer en `BLOCKED`.
3. Déclenche un scheduling.

Le réveil d’une task :

1. La retire de la wait queue.
2. Demande au Core de la passer en `READY`.
3. Elle sera planifiée selon le scheduler.

---

### Structure conceptuelle de `kernel_t`

L’état global du microkernel est regroupé dans une structure `kernel_t` contenant notamment :

- le scheduler,
- la liste des tasks,
- la task courante,
- les structures globales nécessaires au fonctionnement du système.

Aucun état kernel critique ne doit exister en dehors de cette structure.

---

## Organisation du projet

### Arborescence

Le projet est structuré par modules afin de limiter le couplage et de clarifier les responsabilités :

```
mkd/
├── include/
│ ├── kernel/ # API publique (contrats)
│ └── internal/ # Détails d'implémentation (privé au projet)
├── src/
│ ├── core/
│ ├── sync/
│ ├── ipc/
│ ├── drivers/
│ └── platform/
├── tests/ # Tests unitaires / intégration / stress
├── docs/
│ └── adr/ # Architecture Decision Records
├── Makefile
└── README.md
```


Règle : les composants externes (tests blackbox, drivers/services) doivent inclure uniquement `include/kernel/`. Les headers de `include/internal/` sont réservés à l’implémentation.

### Conventions

- Langage : C (C11)
- Nommage des modules : `core`, `ipc`, `sync`, `drivers`, `platform`
- Préfixes recommandés :
  - `k_` pour l’API kernel (syscall-like)
  - `task_`, `sched_`, `chan_`, `mutex_`, `wq_` pour les sous-systèmes
- Les structures internes (définies dans `include/internal/`) ne doivent pas être manipulées directement hors de leur module.

---

## Règles d’ingénierie

### Compilation

Le projet est compilé avec **GCC** (WSL/Linux).

- Standard : `-std=c11`
- Avertissements stricts : `-Wall -Wextra -Werror -pedantic`
- Aucun warning n’est toléré : le build doit être propre (`-Werror`)
- Symboles de debug en mode développement : `-g`
- Optimisation :
  - Debug : `-O0` (ou `-Og`)
  - Release : `-O2`
- (Optionnel, debug) Sanitizers :
  - AddressSanitizer : `-fsanitize=address`
  - UndefinedBehaviorSanitizer : `-fsanitize=undefined`

Le Makefile fournit au minimum des cibles :
- `make` / `make all` : build
- `make clean` : nettoyage
- `make test` : exécution des tests
- `make run` : lancement de `mkd`


### Discipline de développement

Le développement suit une approche structurée mais légère afin de garantir la cohérence du projet sans alourdir le processus.

#### 1. Avant toute implémentation

- Définir clairement l’API du module.
- Écrire les invariants associés.
- Décrire les transitions d’état concernées.
- Si la décision impacte l’architecture : rédiger une ADR.

#### 2. Pendant l’implémentation

- Implémenter par petites étapes fonctionnelles.
- Maintenir les invariants globaux vrais en permanence.
- Ajouter des logs `DEBUG` pour tracer les transitions importantes.
- Ne jamais contourner le Core pour modifier l’état d’une task.

#### 3. Avant de considérer une milestone terminée

- Compilation sans warning (`-Werror`).
- Tests unitaires et d’intégration passent.
- Aucun invariant global violé en build debug.
- Le comportement reste déterministe.
- Toute fonctionnalité incomplète doit être désactivée explicitement (feature flag, TODO clair), jamais laissée dans un état partiellement fonctionnel.

#### 4. Refactorisation

- Autorisée uniquement si les tests passent avant et après.
- Toute modification structurelle importante doit être documentée (ADR).


### Logging & introspection

Le kernel fournit un mécanisme de logging simple afin de faciliter le débogage et l’observabilité :

- Niveaux de logs : `ERROR`, `WARN`, `INFO`, `DEBUG`
- Possibilité d’activer/désactiver certains niveaux à la compilation ou à l’exécution
- (Optionnel) Horodatage basé sur une horloge monotonic encapsulée par `platform`

Les logs doivent permettre de tracer :
- les transitions d’état des tasks (au minimum en `DEBUG`)
- les opérations de blocage/réveil (wait queues)
- les événements scheduler (sélection, yield, fin de task)

Le logging ne doit pas modifier le comportement du scheduler (pas d’allocation bloquante dans les sections critiques).


## Invariants globaux (V1)

### A. Autorité sur l’état des tâches

1. **Seul le Core** est autorisé à modifier `task->state`.  
2. Toute transition d’état se fait via une fonction du Core.
3. Les transitions d’état doivent respecter le graphe défini par le Core (ex. `READY → RUNNING`, `RUNNING → BLOCKED`, etc.).

### B. Cohérence des files et appartenance

4. Une task appartient à **au plus une** structure d’attente à la fois.  
5. Une task en état `READY` est présente exactement une fois dans la ready queue.  
6. Une task en état `BLOCKED` est présente exactement une fois dans une wait queue.  
7. Une task en état `RUNNING` n’est dans aucune file.

### C. Unicité de l’exécution

8. À tout instant, au plus une task est `RUNNING`.  
9. Le scheduler ne sélectionne que des tasks `READY`.

### D. Modèle coopératif

10. Le changement de task active ne peut survenir que si la task appelle `yield()` ou une primitive bloquante.  
11. Une task qui ne rend jamais la main est considérée comme un bug applicatif en V1.
12. Une task `TERMINATED` ne peut plus revenir dans un état exécutable.



---

## Milestones

### Milestone 0 : Boot minimal 

#### Objectif

Mettre en place la base technique du projet sans logique kernel complexe

#### Contenu

* Arborescence complète du projet
* Makefile fonctionnel
* `kernel_t` minimal
* Initialisation/Destruction du kernel (`k_init`, `k_shutdown`)
* Système de logging opérationnel
* Mode `mkd --selftest` fonctionnel

#### Tests

* Build sans warning (`-Werror`)
* Exécution de `mkd` sans crash
* Self-test valide l'initialisation et l'arrêt du kernel

#### Definition Of Done (DoD)

* `make`, `make clean`, `make test`, `make run` fonctionnent
* Logging opérationnel avec niveaux
* Aucun état global hors `kernel_t`

---

### Milestone 1 : Tasks coopératives  

#### Objectif

Implémenter un modèle minimal de task + round-robin coopératif.

#### Contenu

* Structure `task`
* Etats: `NEW`, `READY`, `RUNNING`, `TERMINATED`
* Ready queue
* `yield()`
* Scheduler round-robin simple
* Création d'une task via API publique

#### Tests

* 2 tasks qui alternent via `yield()`
* Vérification des transitions d'état
* Invariant: une seule task `RUNNING`

#### Definition Of Done (DoD)

* Alternance correcte et déterministe
* Aucune task dupliquée dans la ready queue
* Tous les invariants respectés

---

### Milestone 2 : Blocage / Réveil (wait queue)

#### Objectif

Introduire le mécanisme générique de blocage/réveil

#### Contenu

* Implémentation des wait queues
* Transition `RUNNING -> BLOCKED`
* Réveil correct vers `READY`
* Intégration au scheduler

#### Tests

* Task A bloque, task B la réveille
* Vérification de l'appartenance unique au queues
* Stress test blocage/réveil

#### Definition Of Done (DoD)

* aucun wakeup perdu
* aucune task `BLOCKED` sans wait queue
* Invariants cohérents en debug

---

### Milestone 3 : IPC  

#### Objectif

Implémenter la communication inter-task.

#### Contenu

* Structure `channel`
* `send()` / `recv()`
* Blocage si buffer vide / plein
* Intégration avec wait queues

#### Tests

* Producteur / Consommateur
* Plusieurs tasks sur un même channel
* Tests de concurrences simples

#### Definition Of Done (DoD)

* Pas de deadlock non intentionnel
* pas de wakeup perdu
* Communication déterministe

---


### Milestone 4 : Mutex

#### Objectif

Implémenter l'exclusion mutuelle

#### Contenu

* Structure `mutex`
* `mutex_lock` / `mutex_unlock`
* Blacage si déjà verrouillé
* Politique simple (pas de priorité)

#### Tests

* Section critique protégée
* Tentative double lock
* Test d’interblocage volontaire

#### Definition Of Done (DoD)

* Pas d’accès concurrent non protégé
* Pas de corruption d’état
* Invariants respectés

---


### Milestone 5 : Timer  

#### Objectif

Ajouter la notion de temps et `sleep()`.

#### Contenu
* Intégration tick logique interne (`platform`)
* Sleeping list
* Réveil basé sur réveil après N ticks

#### Tests

* Task qui sleep puis reprend
* Plusieurs sleeps simultanés
* Vérification du déterminisme

#### Definition Of Done (DoD)

* Réveils corrects
* Pas de starvation
* Aucun busy-wait inutile

---

## Stratégie de test

### Objectifs

- Valider les invariants globaux à chaque milestone.
- Garder des tests reproductibles (déterministes) pour faciliter le débogage.
- Détecter tôt les régressions sur les transitions d’état, le scheduling et les mécanismes de blocage/réveil.

### Organisation

Le projet propose deux niveaux de tests :

- **Tests externes** dans `tests/` : exécutables dédiés (unitaires / intégration / stress).
- **Self-tests** intégrés à `mkd` (ex. `mkd --selftest`) pour valider rapidement le cœur du kernel en conditions réalistes.

Aucun milestone n’est considéré comme terminé tant que ses tests associés ne passent pas de manière reproductible.

### Types de tests

#### Tests unitaires (structures et logique locale)

Exemples :
- création/destruction de task
- transitions d’état valides / invalides
- opérations sur ready queue et wait queue (insertion, retrait, unicité)
- invariants de base (ex. `RUNNING` jamais dans une queue)

#### Tests d’intégration (scénarios kernel)

Exemples :
- round-robin coopératif entre N tasks via `yield()`
- blocage/réveil via wait queue (ex. `recv()` bloque, `send()` réveille)
- mutex : exclusion mutuelle sur une section critique partagée
- `sleep()` : mise en sommeil puis réveil correct (quand la partie timer est implémentée)

#### Tests de robustesse / négatifs

Exemples :
- task qui ne `yield()` jamais (détection via logs / watchdog debug)
- double `unlock()` / mauvais owner (si choisi dans la sémantique du mutex)
- utilisation invalide des API (paramètres nuls, tailles invalides, etc.)
- stress tests : enchaînements rapides de blocage/réveil pour détecter les wakeups perdus

### Validation des invariants

En build debug, le kernel peut activer des vérifications internes (assertions) sur :
- cohérence état ↔ appartenance aux queues
- unicité des éléments dans les queues
- transitions d’état autorisées uniquement via le Core

Les tests doivent déclencher ces vérifications et échouer explicitement en cas de violation.


---

## Limitations actuelles

*(À compléter au fil du projet)*

---

## Journal d’apprentissage

*(Notes personnelles, bugs marquants, décisions techniques, erreurs et corrections)*
