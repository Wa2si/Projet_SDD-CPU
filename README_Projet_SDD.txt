
README — Projet SDD
====================

Ce fichier explique comment utiliser notre code ainsi que les fichiers de test fournis.

1. Compilation
--------------
Le projet peut être compilé avec `gcc`. Voici quelques exemples de compilation manuelle :

  gcc -o test_cpu main_test_cpu.c cpu.c memory.c hash.c parser.c -Wall
  gcc -o test_parser main_test_parser.c parser.c hash.c -Wall
  gcc -o test_setup main_test_setup.c cpu.c memory.c hash.c parser.c -Wall
  gcc -o test_run main_test_run.c cpu.c memory.c hash.c parser.c -Wall
  gcc -o test_memory main_test_memory.c memory.c hash.c -Wall

Un `Makefile` est également fourni pour automatiser la compilation :
  make            # compile tous les tests
  make clean      # supprime les exécutables

2. Exécution des tests
-----------------------
Voici les exécutables disponibles :

- `./test_cpu` :
  Vérifie l’allocation de la mémoire et l’initialisation du CPU avec un fichier `.DATA`.

- `./test_parser` :
  Analyse le fichier `test_parser.txt`, affiche les instructions extraites, les labels et les variables.

- `./test_setup` :
  Effectue des transferts de données avec tous les modes d’adressage (`immediate`, `register`, `direct`, `indirect`).

- `./test_run` :
  Simule un programme complet à partir du fichier `test_run.txt`, avec exécution interactive instruction par instruction.

- `./test_memory` :
  Teste spécifiquement la gestion de la mémoire : allocation, création de segments, lecture et écriture dans les blocs mémoire.

3. Fichiers de test utilisés
-----------------------------
- `test_parser.txt` :
  Contient un programme d’exemple avec des données (`.DATA`) et du code (`.CODE`) pour tester l’analyse syntaxique.

- `test_run.txt` :
  Un programme complet exécuté par `main_test_run.c`.

4. Dépendances
--------------
Le projet utilise les headers suivants :
- `cpu.h`, `memory.h`, `hash.h`, `parser.h`

Aucun fichier externe n'est nécessaire pour lancer les tests à part les fichiers `.txt`.

