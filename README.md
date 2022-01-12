# Optimalisatie Project
#### Groep 98
##### Rik Wildschut (4894332) en Wim Stienstra (4915755)
Het optimaliseren van een tank simulatie.


## Interessante Punten Optimalisatie
---
### Note
> Alle tijd en ruimte complexiteiten bij de Big-O analyses zijn we uitgegaan van de gemiddelde complexiteit. 

### **Collision detectie ([`game.cpp`](./game.cpp#L389))**
Voor de eerste optimalisatie hadden we een Quadtree gemaakt. Deze bleek echter niet heel erg snel omdat het opbouwen en opvragen erg intensief is.
Daarna hebben we een uniform grid aangemaakt. Deze was vele malen sneller. Single threaded uniform was evensnel als multithreaded Quadtree.
Daarom zijn we doorgegaan met de uniform grid. 
De uniform grid heeft een [`constante lookup`](./collision_grid.cpp#L73). 

We hebben de objecten zoals *Tank*, *Rocket* en *Particle Beam*. laten erven van een nieuwe class: *Collidable*. De uniform grid bestaat uit tiles die allemaal collidables kunnen vasthouden. Voor collision worden de omliggende tiles gepakt en met alle objecten gecheckt voor collision.

De Multithreaded Quadtree is nog werkend te zien in de branch: [`QuadtreeWorking`](https://github.com/NHLStenden-HBO-ICT-SE/periode-2---optimalisatie-p2-groep-98/tree/QuadtreeWorking)

| Algoritme        | Tijd Complexiteit          | Ruimte Complexiteit  |
| ------------- |:-------------:| -----:|
| Origineel     | `O(N^2) `                     | `O(1)`
| Nieuwe        | `Best: O(1) Worst: O(N^2)`                      | `O(W*H)` |


### **Merge sort ([`game.cpp`](./Sorting.cpp#L163))**
Voor de health bar werd *insertion sort* gebruikt. Dit hebben wij aangepast naar *merge sort*.
We kwamen er achter dat vectoren het process erg vertraagden. Daarom hebben wij arrays gebruikt. De merge sort wordt gebruikt voor de [`health bar`](./game.cpp#L0011) en voor het sorteren van vec2 in de [`convex hull`](./game.cpp#L0000).

#### Big-O:

| Algoritme        | Tijd Complexiteit         | Ruimte Complexiteit  |
| ------------- |:-------------:| -----:|
| Origineel     | `O(N^2)`        | `O(1)`  |
| Nieuwe        | `O(N log N)`    | `O(N)`  |

### **Graham Scan Convex hull ([`game.cpp`](./game.cpp#L221))**
Voor de *convex hull* werd een (volgens ons) brute force manier gebruikt om de hull te berekenen.
Deze is vervangen door *Graham Scan* in combinatie met *Merge Sort*.

#### Big-O:

| Algoritme        | Tijd Complexiteit         | Ruimte Complexiteit  |
| ------------- |:-------------:| -----:|
| Origineel     | `O(N(V+E))` | `O(N)` |
| Nieuwe        | `O(N log N)` |   `O(N)` |


### **For loops hergebruiken**
Er zaten veel verschillende for loops in Game.cpp die hetzelfde deden (loopen door alle tanks bijvoorbeeld), deze zijn gecombineerd om minder for-loops te krijgen.

### **Pathfinding**
We hebben geprobeerd A* pathfinding toe te passen, deze probeersels zijn nog te vinden in de branch: [`pathfinding`](https://github.com/NHLStenden-HBO-ICT-SE/periode-2---optimalisatie-p2-groep-98/tree/pathfinding). Omdat dit niet gelukt is hebben we de BFS pathfinding gemultithread.


### Multithreading
Ook hebben wij bij grote for loops de lijst gesplit in het aantal te gebruiken threads om zo de workload te verspreiden. Bijvoorbeeld bij de [`draw`](./game.cpp#631) functie starten we op 1 thread de merge sort en gaan dan op de main thread verder met schrijven naar de canvas. We gebruiken in dit soort momenten maar 1 thread omdat de overhead anders te groot wordt. De merge sort is al heel erg snel dus het aanmaken van allemaal threads is het niet waard maar op deze manier is het toch net wat sneller. Ditzelfde gebeurt ook bij de [`convex hull`](./game.cpp#503)

> Note
In visual studio worden out of range errors in de "output" gezet. Dit zijn errors die [`opgevangen worden`]().