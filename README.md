# 🖥️ Simulateur de CPU (Projet SDD)

Un simulateur de CPU modulaire et interactif écrit en C. Ce projet implémente une architecture informatique simplifiée, incluant la gestion de la mémoire, un parseur d'assembleur et une unité centrale de traitement (CPU) capable d'exécuter des instructions.

## 📋 Table des matières
- [Fonctionnalités](#-fonctionnalités)
- [Architecture](#-architecture)
- [Démarrage](#-démarrage)
- [Utilisation](#-utilisation)
- [Structure du projet](#-structure-du-projet)

## ✨ Fonctionnalités
- **Parseur d'Assembleur Personnalisé** : Lit et analyse du code de type assembleur (instructions, labels, variables).
- **Gestion de la Mémoire** : Simule la RAM avec des segments de données et de code.
- **Simulation du CPU** : Exécute des instructions (MOV, ADD, JMP, etc.) et gère les registres (AX, BX, CX, DX, IP).
- **Exécution Interactive** : Exécution pas-à-pas des programmes.
- **Table des Symboles** : Utilise des tables de hachage pour gérer les labels et les adresses des variables.

## 🏗️ Architecture

Le projet est organisé en plusieurs modules indépendants travaillant ensemble :

```mermaid
graph TD
    subgraph Core [Cœur]
        CPU[CPU (cpu.c)]
        MEM[Mémoire (memory.c)]
        PAR[Parseur (parser.c)]
    end
    
    subgraph Utils [Utilitaires]
        HASH[Table de Hachage (hash.c)]
    end

    subgraph Tests
        TR[test_run]
        TP[test_parser]
        TC[test_cpu]
    end

    CPU --> MEM
    CPU --> PAR
    PAR --> HASH
    MEM --> HASH
    
    TR --> CPU
    TR --> PAR
    TP --> PAR
    TC --> CPU
```

## 🚀 Démarrage

### Prérequis
- **GCC** (GNU Compiler Collection)
- **Make**

### Installation
1. Clonez le dépôt (si applicable) ou téléchargez le code source.
2. Ouvrez un terminal dans le répertoire du projet.
3. Compilez le projet en utilisant `make` :

```bash
make
```

Cela générera les exécutables suivants :
- `test_run` : Le simulateur interactif principal.
- `test_parser` : Pour tester l'analyse des fichiers assembleur.
- `test_cpu`, `test_memory`, `test_setup` : Tests unitaires pour des composants spécifiques.

Pour nettoyer les fichiers de compilation :
```bash
make clean
```

## 🎮 Utilisation

### Lancer une Simulation
Pour lancer une simulation complète d'un programme défini dans `test_run.txt` :

```bash
./test_run
```

**Mode Interactif** : Le programme affichera l'état du CPU et de la mémoire. Appuyez sur **Entrée** pour exécuter l'instruction suivante ou sur **'q'** pour quitter.

### Tester le Parseur
Pour voir comment le parseur analyse un fichier (par exemple, `test_parser.txt`) :

```bash
./test_parser
```

**Exemple de sortie :**
```text
=== Test du fichier test_parser.txt ===

--- .DATA ---
  [0] x DW 42
  [1] arr DB 20;21;22;23
  [2] y DB 10

--- .CODE ---
  [0] MOV AX x
  [1] ADD AX y
  [2] JMP loop 

--- LABELS ---
  start → 0
  loop → 1
```

## 📂 Structure du projet

| Fichier | Description |
|---------|-------------|
| `cpu.c` / `cpu.h` | Implémentation du CPU (registres, cycle d'instruction). |
| `memory.c` / `memory.h` | Simulation de la mémoire (allocation, lecture/écriture). |
| `parser.c` / `parser.h` | Analyse le code assembleur en instructions exécutables. |
| `hash.c` / `hash.h` | Implémentation générique de table de hachage pour la gestion des symboles. |
| `test_run.txt` | Exemple de programme assembleur pour le simulateur. |
| `test_parser.txt` | Fichier exemple pour tester le parseur. |
| `Makefile` | Script de construction pour compiler le projet. |

## 👥 Auteurs
- **Wa2si**
- **Florent**

