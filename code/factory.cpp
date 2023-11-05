#include "factory.h"
#include "costs.h"
#include "extractor.h"
#include "wholesale.h"
#include <iostream>
#include <pcosynchro/pcothread.h>

WindowInterface *Factory::interface = nullptr;


Factory::Factory(int uniqueId, int fund, ItemType builtItem, std::vector<ItemType> resourcesNeeded)
    : Seller(fund, uniqueId), resourcesNeeded(resourcesNeeded), itemBuilt(builtItem), nbBuild(0) {
    assert(builtItem == ItemType::Chip ||
           builtItem == ItemType::Plastic ||
           builtItem == ItemType::Robot);

    NTEST(interface->updateFund(uniqueId, fund));
    NTEST(interface->consoleAppendText(uniqueId, "Factory created"));
}

void Factory::setWholesalers(std::vector<Wholesale *> wholesalers) {
    Factory::wholesalers = wholesalers;

#ifndef GTEST_CONTEXT
    for (Seller *seller: wholesalers) {
        interface->setLink(uniqueId, seller->getUniqueId());
    }
#endif
}

ItemType Factory::getItemBuilt() {
    return itemBuilt;
}

int Factory::getMaterialCost() {
    return getCostPerUnit(itemBuilt);
}

bool Factory::verifyResources() {
    for (auto item: resourcesNeeded) {
        if (stocks[item] == 0) {
            return false;
        }
    }

    return true;
}

void Factory::buildItem() {

    // TODO

    /* Vérification  des fonds et du stock nécessaire pour produire l'objet */
    int itemCost = getEmployeeSalary(getEmployeeThatProduces(itemBuilt));
    if (itemCost > money || !verifyResources()) {
        return;
    }

    //Temps simulant l'assemblage d'un objet.
    PcoThread::usleep((rand() % 100) * 100000);

    // TODO
    mutex.lock();
    money -= itemCost;

    /* Incrément des stocks de l'article produit */
    nbBuild++;
    stocks[itemBuilt] += 1;
    /* Decrémenter le stock des matériaux utilisés */
    for (ItemType item: resourcesNeeded) {
        stocks[item] -= 1;
    }
    mutex.unlock();

    NTEST(interface->updateFund(uniqueId, money));
    NTEST(interface->updateStock(uniqueId, &stocks));

    NTEST(interface->consoleAppendText(uniqueId, QString("1 ") % getItemName(itemBuilt) % " has been built"));
}

void Factory::orderResources() {

    // TODO - Itérer sur les resourcesNeeded et les wholesalers disponibles
    for (ItemType it: resourcesNeeded) {
        int price = getCostPerUnit(it);

        /*
        NTEST(interface->consoleAppendText(uniqueId, QString("I would like to buy 1 of ") % getItemName(it)
                                           % QString(" which would cost me %1").arg(price)));
        */

        /* TODO (OK) */
        /* Si stocks[item] = 0 et on a assez d'argent*/
        mutex.lock();
        if (!stocks[it] && money >= price) {

            /* On regarde lequel des wholesaler peut nous fournir le produit */
            for (Wholesale *w: wholesalers) {
                if (w->trade(it, 1)) {
                    money -= price;
                    NTEST(interface->updateFund(uniqueId, money));
                    stocks[it] += 1;
                    NTEST(interface->updateStock(uniqueId, &stocks));

                    NTEST(interface->consoleAppendText(uniqueId, QString("I have bought 1 of ") % getItemName(it) % QString(" at wholesaler %1").arg(w->getUniqueId()) % QString(" which would costed me %1").arg(price)));

                    break;
                }
            }
        }
        mutex.unlock();
    }

    //Temps de pause pour éviter trop de demande
    PcoThread::usleep(10 * 100000);
}

void Factory::run() {
    if (wholesalers.empty()) {
        std::cerr << "You have to give to factories wholesalers to sales their resources" << std::endl;
        return;
    }
    NTEST(interface->consoleAppendText(uniqueId, "[START] Factory routine"));

    while (!PcoThread::thisThread()->stopRequested()) {
        if (verifyResources()) {
            buildItem();
        } else {
            orderResources();
        }
        NTEST(interface->updateFund(uniqueId, money));
        NTEST(interface->updateStock(uniqueId, &stocks));
    }
    NTEST(interface->consoleAppendText(uniqueId, "[STOP] Factory routine"));
}

std::map<ItemType, int> Factory::getItemsForSale() {
    return std::map<ItemType, int>({{itemBuilt, stocks[itemBuilt]}});
}

int Factory::trade(ItemType it, int qty) {
    if (it != getItemBuilt()) return 0;

    mutex.lock();
    if (qty <= 0 || stocks[it] < qty) {
        mutex.unlock();
        return 0;
    }

    stocks[it] -= qty;
    NTEST(interface->updateStock(uniqueId, &stocks));

    int tradeProfit = getMaterialCost() * qty;
    money += tradeProfit;
    NTEST(interface->updateFund(uniqueId, money));

    mutex.unlock();
    return tradeProfit;
}

int Factory::getAmountPaidToWorkers() {
    return Factory::nbBuild * getEmployeeSalary(getEmployeeThatProduces(itemBuilt));
}

void Factory::setInterface(WindowInterface *windowInterface) {
    interface = windowInterface;
}

PlasticFactory::PlasticFactory(int uniqueId, int fund) : Factory::Factory(uniqueId, fund, ItemType::Plastic, {ItemType::Petrol}) {}

ChipFactory::ChipFactory(int uniqueId, int fund) : Factory::Factory(uniqueId, fund, ItemType::Chip, {ItemType::Sand, ItemType::Copper}) {}

RobotFactory::RobotFactory(int uniqueId, int fund) : Factory::Factory(uniqueId, fund, ItemType::Robot, {ItemType::Chip, ItemType::Plastic}) {}
