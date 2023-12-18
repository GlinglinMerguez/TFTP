# Client TFTP

## Contexte

Le projet vise à créer un programme client capable d'échanger des fichiers en utilisant le protocole TFTP.

- Ce projet est développé par Léo APPOURCHAUX et Yumeng YE.
- Le code est hébergé sur Github, avec un référentiel public
- Chaque question a un fichier séparé.

### Question 1 : Gestion des arguments de la ligne de commande

**Explication :**
- Le programme commence par vérifier le nombre d'arguments passés à la ligne de commande. S'il n'y pas exactement 3 arguments (nom du programme, hôte, fichier), il affiche un message d'erreur et se termine.
- Sinon, il continue avec le traitement.

### Question 2 : Obtention de l'adresse du serveur

**Explication :**
- Une structure "addrinfo" est initialisée pour spécifier les critères lors de la recherche d'adresses. Cette structure est remplie avec des informations telles que la famille d'adresses (IPv4 qu'on cherche ici) et le type de socket (UDP).
- La fonction "getaddrinfo" est appelée avec les arguments de la ligne de commande (hôte et ficheir) pour obtenir les informations sur l'adresse du serveur. Si cette opération échoue, le programme affiche un message d'erreur et se termine.

### Question 3 : Réservation du socket ###

**Explication :**
- Un socket est créé en utilisant les informations d'adresse obtenue à l'étape précédente. Si la création est échouée, le programme affiche un message d'erreur et se termine.

### Question 4 : Création de la fonction gettftp ###

**Explication :**
1) Construction et envoi de la requête
    - Le programme construit un paquet de requête de lecture (RRQ) formé avec l'opcode approprié et envoie ce paquet au serveur en utilisant la fonction "sendto". Le paquet contient le nom du fichier et le mode de transfert.
2) Réception des données
    - Le programme attend la réponse du serveur en utilisant la fonction "recvfrom" pour recevoir le paquet de données et son acquittement.
    - Ensuite le programme envoie un acquittement ACK au serveur pour confirmer la réception du paquet de données.
    - Enfin les données du fichier reçues du serveur sont écrites sur la sortie standard à l'aide de la fonction "write". Cette sortie standard est représentée par STDOUT_FILENO.
3) Gestion des erreurs et nettoyage
    - En cas d'erreur à n'importe quelle étape du processus, le programme affiche un message d'erreur, ferme le socket, libère la mémoire utilisée pour les informations d'adresse ("freeaddrinfo), et se termine.

### Question 5 : Création de la fonction puttftp ##


