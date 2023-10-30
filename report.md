# Labo 3 report - Managing concurrent access - PCO

Authors: Vitória Oliveira et Samuel Roland

## Introduction
This project consists in a sales simulation application with extractors, wholesalers and factories. They are constantly selling and/or buying resources from/to other entities. The program has a UI that shows in real time the ressources and funds of each entity.

The primary goal of this project is to effectively address problems associated to concurrent access such as the protection of shared resources. 

This report documents our strategies in implementing solutions to tackle these issues. 

## Description of software features

## Implementation Choices

### Extractors
    - Competition Management in extractor::run

    It's necessary to acquire the instance's mutex in two steps to ensure that the current thread is the only one modifying the amount of money during the miner's payment and, later on, the number of extractions and the modification of the stock. These instructions need protection because other threads might attempt to modify these variables simultaneously (for example, a call to trade() by another thread that could try to access money and stocks).
    
    - Extractor::trade

    First, it's essential to check the arguments to proceed with the transaction. A check to certify that the requested quantity is greater than 0 is made; otherwise, it would not be logical neither necessary to carry out the transaction. It is also essential to verify that the requested item matches the item sold by the extractor and that the extractor has the requested quantity.

    Once this is done, we can proceed with the transaction. Therefore, we need to update the seller's stock and funds.
    Since multiple threads can attempt to perform transactions simultaneously, we need to manage concurrency. Since a stock check is performed before starting the transaction, it is vital to lock a mutex before this check is made to ensure that the updated stock corresponds to the stock previously returned and that no other threads have been able to change it between these two instructions.

 ### Termination 
    In order to ensure a proper termination of the simulation, we've declared a boolean variable called *stopRequest* in the file *Utils.cpp* that serves as signal to indicate if a request to end the simulation has been made. 
    This variable is initially set to false and is set to true if a call to *Utils::endService()* is made (when the window is closed).

    To take this in account elsewhere in the project, we've added a while loop that checks if *stopRequest* is true in all run functions for each Seller subclass.    
    
## Tests

## Conclusion

