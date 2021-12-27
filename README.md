# Optimalisatie_Project

## interessante Punten Optimalisatie

#### Collision systeem
We hebben de objecten zoals Tank, Rocket, Particle laten erven van een nieuwe class: Collidable.
De simulatie maakt al gebruik van een grid(~ 40x80) En die gebruiken wij opnieuw door de Collidables elke frame in de bijpassende tile te zetten.
Bij collision berekeningen worden de omliggende tiles gebruikt.

#### Merge sort ([`game.cpp`](./game.cpp#L383))
Voor de health bar werd insertion sort gebruikt. Dit hebben wij aangepast naar merge sort.

#### Graham Scan Convex hull
Voor de convex hull werd een (volgens ons) brute force manier gebruikt om de hull te berekenen.

#### For loops hergebruiken
Er zaten veel verschillende for loops in Game.cpp, deze zijn gecombineerd om minder for-loops te krijgen.
