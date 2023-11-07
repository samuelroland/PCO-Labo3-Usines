/* Modified by: Vitória Oliveira and Samuel Roland */

#include "extractor.h"
#include "costs.h"
#include <pcosynchro/pcothread.h>

WindowInterface *Extractor::interface = nullptr;

Extractor::Extractor(int uniqueId, int fund, ItemType resourceExtracted)
    : Seller(fund, uniqueId), resourceExtracted(resourceExtracted), nbExtracted(0) {
    assert(resourceExtracted == ItemType::Copper ||
           resourceExtracted == ItemType::Sand ||
           resourceExtracted == ItemType::Petrol);
    NTEST(interface->consoleAppendText(uniqueId, QString("Mine Created")));
    NTEST(interface->updateFund(uniqueId, fund));
}

std::map<ItemType, int> Extractor::getItemsForSale() {
    return stocks;
}

int Extractor::trade(ItemType it, int qty) {
    mutex.lock();
    if (qty > 0 && it == getResourceMined() && stocks[it] >= qty) {
        //mettre à jour les stocks & les fonds
        stocks[it] -= qty;
        NTEST(interface->updateStock(uniqueId, &stocks));

        int tradeProfit = getMaterialCost() * qty;
        money += tradeProfit;
        NTEST(interface->updateFund(uniqueId, money));

        mutex.unlock();
        return tradeProfit;
    }

    mutex.unlock();
    return 0;
}

void Extractor::run() {
    NTEST(interface->consoleAppendText(uniqueId, "[START] Mine routine"));

    while (!PcoThread::thisThread()->stopRequested()) {
        /* (TODO) concurrence OK */

        int minerCost = getEmployeeSalary(getEmployeeThatProduces(resourceExtracted));
        if (money < minerCost) {
            /* Pas assez d'argent */
            /* Attend des jours meilleurs */
            PcoThread::usleep(1000U);
            continue;
        }

        /* On peut payer un mineur */
        mutex.lock();
        money -= minerCost;
        mutex.unlock();

        /* Temps aléatoire borné qui simule le mineur qui mine */
        PcoThread::usleep((rand() % 100 + 1) * 10000);

        /* Statistiques */
        nbExtracted++;
        mutex.lock();
        /* Incrément des stocks */
        stocks[resourceExtracted] += 1;
        mutex.unlock();

        /* Message dans l'interface graphique */
        NTEST(interface->consoleAppendText(uniqueId, QString("1 ") % getItemName(resourceExtracted) %
                                                             " has been mined"));
        /* Update de l'interface graphique */
        NTEST(interface->updateFund(uniqueId, money));
        NTEST(interface->updateStock(uniqueId, &stocks));
    }
    NTEST(interface->consoleAppendText(uniqueId, "[STOP] Mine routine"));
}

int Extractor::getMaterialCost() {
    return getCostPerUnit(resourceExtracted);
}

ItemType Extractor::getResourceMined() {
    return resourceExtracted;
}

int Extractor::getAmountPaidToMiners() {
    return nbExtracted * getEmployeeSalary(getEmployeeThatProduces(resourceExtracted));
}

void Extractor::setInterface(WindowInterface *windowInterface) {
    interface = windowInterface;
}

SandExtractor::SandExtractor(int uniqueId, int fund) : Extractor::Extractor(uniqueId, fund, ItemType::Sand) {}

CopperExtractor::CopperExtractor(int uniqueId, int fund) : Extractor::Extractor(uniqueId, fund, ItemType::Copper) {}

PetrolExtractor::PetrolExtractor(int uniqueId, int fund) : Extractor::Extractor(uniqueId, fund, ItemType::Petrol) {}
