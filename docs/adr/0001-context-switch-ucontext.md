# ADR-0001 — Mécanisme de context switch (ucontext)

## Contexte

Le microkernel implémente un modèle d’exécution coopératif : plusieurs tasks doivent pouvoir s’exécuter de manière indépendante, chacune disposant de sa propre pile et de son propre contexte d’exécution.

Un context switch nécessite la sauvegarde et la restauration des éléments suivants :

- pointeur d’instruction (instruction pointer),
- pointeur de pile (stack pointer),
- registres généraux,
- état implicite lié à l’ABI d’appel.

Chaque task doit donc posséder :

- une stack dédiée,
- une structure représentant son contexte CPU sauvegardé.

Le mécanisme choisi doit :

- fonctionner en user-space,
- être compatible avec un scheduling coopératif,
- rester déterministe,
- limiter la complexité pour la V1.

---

## Décision

Le projet utilisera l’API POSIX `ucontext` (`getcontext`, `makecontext`, `swapcontext`) pour implémenter le mécanisme de context switch dans la V1.

Chaque task contiendra :

- une stack dédiée allouée dynamiquement,
- un `ucontext_t` représentant son contexte d’exécution.

Le scheduler utilisera `swapcontext()` pour effectuer les transitions entre tasks.

Le mécanisme sera encapsulé dans un module dédié (`core/context.*`) afin d’isoler l’implémentation et permettre un remplacement ultérieur.

Le démarrage d’une task passe par une fonction trampoline interne afin de capturer la fin d’exécution et marquer la task `TERMINATED` avant rescheduling.

---

## Alternatives considérées

### 1. Implémentation en assembleur (x86_64)

Avantages :
- contrôle total du context switch,
- plus proche d’un noyau réel,
- très formateur.

Inconvénients :
- complexité accrue dès la V1,
- risque élevé de bugs bas niveau,
- ralentissement du développement des autres modules (scheduler, IPC, sync).

Cette option est envisagée pour une évolution future du projet.

---

### 2. setjmp / longjmp

Avantages :
- API standard C,
- simple en apparence.

Inconvénients :
- gestion complexe des stacks séparées,
- risque d’Undefined Behavior,
- moins adapté à un modèle multi-stack propre.

---

### 3. pthread (threads OS)

Avantages :
- simplicité d’utilisation.

Inconvénients :
- perte du contrôle sur le scheduling,
- non déterministe,
- contraire aux objectifs pédagogiques du projet.

---

## Conséquences

- Le système reste simple et déterministe en V1.
- Le développement des modules scheduler, IPC et synchronisation peut avancer sans complexité bas niveau excessive.
- L’implémentation du context switch est isolée et pourra être remplacée ultérieurement par une version assembleur sans modifier l’architecture globale.
