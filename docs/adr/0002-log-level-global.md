# ADR-0002 — Niveau de log global hors kernel_t

## Contexte

Le projet impose que tout état kernel soit regroupé dans `kernel_t`, afin d'éviter les variables globales non maîtrisées.

Le module de logging (`src/core/log.c`) maintient un état interne : le niveau de log actif (`g_level`). Cette variable détermine quels messages sont affichés.

Deux options ont été considérées pour héberger cet état :

- le rattacher à `kernel_t`,
- le maintenir comme état interne au module de logging, hors `kernel_t`.

---

## Décision

Le niveau de log (`g_level`) reste une variable statique interne au module `log.c`, hors de `kernel_t`.

---

## Justification

Le logging est une infrastructure transversale au projet. Il doit fonctionner :

- **avant** `k_init()` : pour tracer les erreurs d'initialisation,
- **après** `k_shutdown()` : pour tracer la fin du cycle de vie du kernel,
- **en cas d'échec critique** : même si `kernel_t` est dans un état incohérent.

Rattacher `g_level` à `kernel_t` introduirait un couplage entre l'infrastructure de logging et le cycle de vie du kernel. Cela rendrait impossible tout log avant `k_init()` et fragiliserait le débogage des cas limites.

---

## Alternatives considérées

### Rattacher `g_level` à `kernel_t`

Avantages :
- cohérence stricte avec la règle "aucun état global hors `kernel_t`".

Inconvénients :
- impossible de logger avant `k_init()` ou après `k_shutdown()`,
- couplage artificiel entre logging et kernel,
- fragilise le débogage des cas d'erreur les plus critiques.

---

## Conséquences

- `log_init()` reste appelable indépendamment du cycle de vie du kernel.
- Le logging est utilisable dans tous les contextes, y compris les plus critiques.
- Cette exception est explicitement documentée et isolée à ce seul module.
- Toute autre variable globale hors `kernel_t` doit faire l'objet d'une ADR distincte.
