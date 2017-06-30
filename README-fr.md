# [Compilateur Pasclang pour Pseudo-Pascal](https://gitlab.com/abonnet/pasclang/)

[English version/Version Anglaise](README.md)

## Le projet

Pasclang est un compilateur pour le langage éducatif Pseudo-Pascal utilisant LLVM pour générer le code machine. L'ambition est de fournir un exemple d'implémentation d'un simple langage impératif utilisant CMake et LLVM et évitant les bibliothèques extérieures autant que possible.

## Le langage Pseudo-Pascal

Pseudo-Pascal est un dialecte de Pascal possédant une syntaxe et une sémantique opérationelle similaires mais limitées. Par exemple il ne supporte que la compilation de fichiers uniques (une seule unité de translation). Seules quelques fonctions de base sont implémentées : `writeln()`, `write()` et `readln()`. Les types supportés sont les entiers et les booléens. Le dossier [test](test/) contient plusieurs examples de programmes utilisant le langage.
Pour ceux qui ne connaissent pas du tout Pascal, voici un exemple de programme calculant des termes de la suite de Fibonacci et s'arrêtant lorsque l'on donne 0 :

```pascal
program
var
    a : integer;
function fibonacci(n : integer) : integer;
begin
    if n = 0 then
        fibonacci := 0
    else if n = 1 then
        fibonacci := 1
    else
        fibonacci := fibonacci(n-1) + fibonacci(n-2)
end;
begin
    a := 1;
    while a > 0 do
        begin
            a := readln();
            writeln(fibonacci(a))
        end
end.
```

## Le compilateur

La version stable actuelle est [Pasclang 1.1](https://gitlab.com/abonnet/pasclang/tree/1.1). La version en développement se trouve dans la branche [master](https://gitlab.com/abonnet/pasclang/tree/master).

Le compilateur agit par passages successives comme c'est le cas de la plupart des compilateurs modernes. La première étape correspond à celle des analyses lexicales et syntaxiques, ensuite vient la vérification des types. L'arbre de syntaxe abstraite est directement utilisé pour la génération de code même si ce n'est pas idéal, utiliser une structure non-typée serait superflu dans notre cas puisque LLVM s'occupe des transformations intermédiaires.

Un des objectifs principaux est de fournir des diagnostiques utiles, par exemple voici un programme syntaxiquement incorrect et ses diagnostiques :
```pascal
$ cat syntax.pp 
program
var a: array of integer;
procedure f(i : integer; a : array of integer; b : boolean);
begin
    writeln(a)
end
begin
    b := true
    a := new array of array of integer[a];
    a := f(156, a,)
end.
$ bin/pasclang syntax.pp -f
error: at line 7
        unexpected token begin when expecting any of the following: ;

begin
^^^^^
note: 
        Pasclang will now look for additional syntax errors. However since the input already contains an error, some reports may be wrong.

error: at line 10
        unexpected token ) when expecting any of the following: boolean literal, int literal, identifier, (

    a := f(156, a,)
                  ^
```
de même avec la vérification des types :
```pascal
$ cat type.pp 
program
var a: array of integer;
procedure f(i : integer; a : array of integer; b : boolean);
begin
    writeln(a, i)
end;
begin
    b := true;
    a := new array of array of integer[a];
    a := f(156, a, b)
end.
$ bin/pasclang type.pp -f
error: at line 5
        wrong number of arguments in call to writeln

    writeln(a, i)
    ^^^^^^^^^^^^^
error: at line 5
        unexpected type int[1] instead of int[0] 

    writeln(a, i)
            ^^
warning: 
        unused variable b in function f

warning: 
        unused variable i in function f

error: at line 8
        undefined symbol b

    b := true;
    ^^^^^^
error: at line 9
        unexpected type int[1] instead of int[0] 

    a := new array of array of integer[a];
                                       ^^
error: at line 9
        unexpected type int[2] instead of int[1] 

    a := new array of array of integer[a];
         ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: at line 10
        invalid call to procedure or function f

    a := f(156, a, b)
         ^^^^^^^^^^^^
error: at line 10
        unexpected type int[2] instead of int[1] 

    a := f(156, a, b)
         ^^^^^^^^^^^^
```

## Compiler

Pasclang utilise [CMake](https://cmake.org) comme système de compilation car c'est également celui requis pour LLVM.
Afin de compiler Pasclang, il est nécessaire d'installer [LLVM](http://llvm.org/docs/CMake.html) en le compilant ou le téléchargeant. Il faut ensuite s'assurer que LLVM soit visible par CMake, ce qui est habituellement le cas lorsqu'installé dans l'arborescence classique `/usr/...` ou dans le répertoire ciblé par la variable d'environnement `LLVM_DIR`. Ensuite les commandes sont très simples, par exemple depuis le répertoire racine :

```bash
mkdir build
cd build
cmake ..
make -j<number of jobs>
```

Vous pouvez ensuite exécuter les tests en tapant `make runtests` et `make runmoretests` pour les tests issus du [compilateur MIPS](http://pauillac.inria.fr/~fpottier/X/INF564/) de François Pottier.

## Usage

L'usage en ligne de commande est documenté lorsque Pasclang est exécuté sans argument (l'exécutable est présent par défaut dans le répertoire bin/ du répertoire de compilation).

## Support

Le développement et les tests ont lieu sur une distribution Linux sur architecture amd64 et passe tous les tests sur Debian 9 et Fedora 25. Les compilateurs testés sont habituellement clang >= 4.0.0 avec LLVM >= 4.0.0. Les versions stables sont testées sur une distribution Linux utilisant musl (Alpine Linux) et les systèmes OpenBSD 6.1 et FreeBSD 11.0 (avec clang et LLVM 3.9 pour ces derniers). Certaines distrubtions (par exemple Fedora) requierent le téléchargement et l'installation de bibliothèques de développement statiques, par exemple en cas d'erreur `cannot find -lc/-lstdc++/-lm` lors de l'édition des liens. Autrement, le compilateur devrait fonctionner sans modification sur les systèmes Unix mais demandera des ajustements autrement, notamment pour les lignes sous le `#warning` dans `src/main.cpp`.

### Problèmes connus dans la dernière version stable

La dernière révision stable est Pasclang 1.1. Voici une liste de problèmes connus en train d'être résolus pour la prochaine révision.

* Un mauvais usage du compilateur mène parfois à des erreurs de segmentation ou des boucles infinies, par exemple lorsqu'on donne un fichier qui n'existe pas en entrée. Des routines sont en train d'être implémentées pour ces cas de mauvais usage.
* Les opérateurs logiques ne court-circuitent pas lorsque le résultat est connu avant l'évaluation de tous les termes. Ce comportement est incorrect pour la sémantique opérationnelle de Pseudo-Pascal est et en train d'être corrigé. En attendant, le test correspondant a été désactivé.

## Arborescence des sources

### [include](include/)

Le dossier `include` contient les fichiers d'en-tête du compilateur. Ils peuvent également être copiés dans un autre projet pour utiliser Pasclang comme une bibliothèque, mais la cible `install` ne le copie pour l'instant pas.

### [src](src/)

Ce répertoire contient la plupart des fonctions et autres données se trouvant finalement dans le code objet.

### [rt](rt/)

La bibliothèque `rt` contient les routines nécéssaires par les programmes Pseudo-Pascal pour s'exécuter correctement, c'est à dire les fonctions d'entrée-sortie et de gestion de la mémoire.

### [test](test/)

Les tests sont exécutés en compilant chaque fichier .pp, puis en comparant la sortie du programme testé auquel on a donné l'entrée .in avec la sortie de référence .out.

## Remerciements

* [Le cours de Luc Maranget](http://gallium.inria.fr/~maranget/X/compil/poly/index.html) a été la raison de mon choix de ce langage.
* [François Pottier](http://cristal.inria.fr/~fpottier) qui a eu l'amabilité de me permettre d'utiliser les tests de son [compilateur pour MIPS](http://cristal.inria.fr/~fpottier/X/INF564/petit.tar.gz). Il s'agit des tests disponibles dans la cible `make runmoretests`.
* Les auteurs du tutoriel [LLVM Kaleidoscope](http://llvm.org/docs/tutorial/index.html) pour fournir une introduction détaillée à la bibliothèque LLVM.

