# Labo 3 report - Managing concurrent access - PCO

Authors: Vitória Oliveira et Samuel Roland

## Introduction
This project consists in a sales simulation application with extractors, wholesalers and factories. They are constantly selling and/or buying resources from/to other entities. The program has a UI that shows in real time the ressources and funds of each entity.

The primary goal of this project is to effectively address problems associated to concurrent access such as the protection of resources that can be manipulated by multiple functions. 

This report documents our strategies in implementing solutions to tackle these issues. 

## Description of software features

## Implementation Choices

### Mutex
- (TODO à effacer) QUESTION: MULTIPLE THREADS? 

The main problem of this project is to safeguard resources of a instance that might be accessed by multiple threads/functions simutaneaously. More precisely, we've focused on protecting the stocks and funds of each Seller object as these variables are modify everytime Sellers build, buy and sell items to each other. 

- (TODO à effacer)  POURQUOI MUTEX EST PROTECTED ET DECLARE DANS SELLER

In order to do so, we've implemented a protected access mutex in Seller's class. This allows each subclass to have it's own exclusive mutex, ensuring that no other object can access it. This prevents data race conditions, where multiple functions within an object might attempt to access these resources concurrently.

- (TODO à effacer) INTERET DE CHAQUE OBJET D'AVOIR UN SEUL MUTEX PAR RESOURCE A PROTEGER 

As multiple functions in each object might try to access these ressources at the same time, we've chosen to implement a single mutex for each resource to be secured as a way to ensure data integrity. This mutex was acquired  whenever we read or write to the money or stocks and is released once the operation is completed.

- (TODO à effacer) POURQUOI UN MUTEX POUR MONEY/STOCK ET PAS UN POUR CHAQUE RESOURCE

However, for our specific project requirements, we observed that money and stocks were consistently read and written to within the same block of instructions. To simplify our implementation and enhance performance, we opted for a single mutex to protect both variables.

- (TODO à effacer) NECESSAIRE? (texte ci-dessous)

It's essential to note that whenever a resource is read and modified right after, this should be done without releasing the mutex inbetween these instructions. Doing so would introduce the possibility of another thread or function modifying the resource, potentially leading to incoherent results. Our chosen design ensures that resource access and modification occur atomically within the same critical section of code, preserving data integrity.


- (TODO à effacer) EN FAIT, JSP SI C'EST JUSTE CE QUI EST CI-DESSOUS, A VOIR SI ON LE LE JETTE OU ON LE GARDE (ET VOIR OU LE METTRE) 

There is one important point to understand here: stocks and funds can only be modified by its owner as they are private variables. But, it is important to notice that, an object can ask for another to "change" its resources; this is done by the trade function. This function allows a thread to "communicate" with another. 

### Extractors
**Competition Management in *extractor::run***
It's necessary to acquire the instance's mutex in two steps to ensure that the current thread is the only one modifying the amount of money during the miner's payment and, later on, the modification of the stock. These instructions need protection because other threads might attempt to modify these variables simultaneously (for example, a call to trade() by another thread that could try to access money and stocks).

***Extractor::trade***  
First, it's essential to check the arguments to proceed with the transaction. A check to certify that the requested quantity is greater than 0 is made; otherwise, it would not be logical neither necessary to carry out the transaction. It is also essential to verify that the requested item matches the item sold by the extractor and that the extractor has the requested quantity.

Once this is done, we can proceed with the transaction. Therefore, we need to update the seller's stock and funds.
Since multiple threads can attempt to perform transactions simultaneously, we need to manage concurrency. Since a stock check is performed before starting the transaction, it is vital to lock a mutex before this check is made to ensure that the updated stock corresponds to the stock previously returned and that no other threads have been able to change it between these two instructions.

### Wholesales
***Wholesale::buyResources***  
To start with, it is fundamental to acquire a mutex prior to verifying the availability of the wholesale's financial resources and confirming the feasibility of the trade through a call to the seller's trade function. This is necessary to prevent a potential interference by another thread that may alter the wholesale's finances or the seller's inventory. Once this is done, we can proceed and update the wholesale's funds and inventory,  after which the mutex can be released. 

***Wholesale::trade()***  
Similar to Extractor::trade, except for the conditions to proceed with the trade. It is necessary to check if the requested quantity is strictly positive or if this quantity for the wanted item is available in stock.

### Factories
***Factory::buildItem***
- /* TODO */

****Factory::orderResources***
- /* TODO */

****Factory::trade***
- /* TODO */

### Termination 
In order to ensure a proper termination of the simulation, we've declared a boolean variable called *stopRequest* in the file *Utils.cpp* that serves as signal to indicate if a request to end the simulation has been made. 
This variable is initially set to false and is set to true if a call to *Utils::endService()* is made (when the window is closed).

To take this in account elsewhere in the project, we've added a while loop that checks if *stopRequest* is true in all run functions for each Seller subclass.    

- /* TODO appeler requestStop de pcothread */

## Tests
To avoid needing to setup a Qt UI interface just to do unit tests, we decided to disable any usage of the `interface` attribute so we don't call `setInterface`. (Therefore the attribute `interface` is `NULLPTR` in tests). We created a macro `NTEST` used like `NTEST(interface...)` that doesn't run the given instruction in case the `GTEST` variable has been defined. This is kind of a "headless" mode.

To run unit test, we setup a each entity with fakes entities requesting trades and accepted trades against it so we can validate the concurrency protections are effective.

To run end to end test, we create a `Utils` object and call its `run` method to start and `externalEndService` method to end.

## Conclusion
/* TODO */

- TODO expliquer pourquoi on a deux mutex dans extractor::run (à cause du sleep) et TODO pourquoi on en un slmt 1 dans factory

- TODO pourquoi verifyRessources dans buildItem alors que deja dans run (risque de preemption entre les deux?)

- TODO expliquer pourquoi on ne fait rien concernant le commentaire "acheter en priorite celui que l'usine n'a pas en stock" car de toute facon on ne va acheter que si stock[item] == 0. donc il n'y a rien à vérifier. 

- TODO add message d'affichage

- TODO dans factory::orderResources(). 1 de quantité as variable quantity for code evolution (or add CONST STATIC for recette d'ingredients) + vérifier stock[it] < qtyNeeded

- TODO voir où on check la quantité d'un stock car on peut l'obtenir à partir de getItemsForSale.first == item voulu et on check .second qui nous dira la quantité disponible.

- Factory::orderResources
on itere sur les ressources necessaires:
    on regarde si on a besoin du produit et si on a assez d'argent, sinon ca sert à rien d'itérer sur les wholesellers alors qu'on a pas besoin/peut pas acheter le produit
        on itère sur les sellers 
            on regarde qui peut nous fournir le produit
                on l'achete
mutex dès qu'on vérifie les fonds car on veut pas que les fonds diminue avant d'acheter le produit (par exemple dans buildItem) (pas besoin de mettre mutex pour le stock car ici on vérifie si stock[0] et si stock[0] il ne peut pas diminuer ailleurs et s'il augmente c'est pas grave MAIS pas forcément besoin de l'acheter MAIS par question d'évolutivité ce ne serait pas grave de le sécuriser)

- TODO expliquer stratégie générale des mutex. Un mutex par chaque instance d'objet car chaque objet possède ses propres fonds & stock & est le seul à y avoir accès. Aussi car lorsqu'une transaction est faite, elle est implementé dans le vendeur. Ainsi, le vendeur a accès à ses ressources. Et le stock & fonds de l'acheter est mis à jour lorsqu'il appelle la fonction trade de l'objet à qui il veut acheter (notamment dans la fonction run).

- TODO appeler requestStop de pcothread 
            