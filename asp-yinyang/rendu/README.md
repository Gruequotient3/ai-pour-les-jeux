# Solveur ASP Yin-Yang - MegaBonk256

Ce dépôt contient trois itérations d'un solveur ASP pour le puzzle Yin-Yang, exécutées via Python avec la bibliothèque `clingo`. 


## Fichiers Logiques (.lp)

Les trois versions illustrent l'optimisation progressive d'un programme ASP :

* **`yinyang_001.lp` (Basique)** : Implémentation stricte des règles du jeu. Utilise un choix libre pour déterminer la racine des régions connectées, ce qui entraîne une grande lenteur sur les grandes grilles en raison des symétries.
* **`yinyang_002.lp` (Optimisation Locale)** : Ajoute des contraintes redondantes ("early failures"). Le solveur rejette instantanément les motifs impossibles comme les damiers 2x2 et les cellules totalement isolées sans lancer le calcul lourd de connectivité globale.
* **`yinyang_003.lp` (Optimisation Globale)** : Casse la symétrie massive du choix de la racine. La racine de chaque couleur est désormais définie de manière déterministe (la case avec le plus petit index), ce qui réduit drastiquement l'arbre de recherche et permet de résoudre des grilles complexes comme des 25x25.

## Exécution

Utilisez le script Python correspondant à la version souhaitée. Par défaut, le script charge une grille codée en dur.

```bash
# Exemple pour lancer la version la plus rapide sur la grille par défaut
python run_yinyang_003.py

# Exemple pour lancer avec une taille spécifique et une grille personnalisée
python run_yinyang_003.py 6 "WW......W..WBB.W..B......B..B"
