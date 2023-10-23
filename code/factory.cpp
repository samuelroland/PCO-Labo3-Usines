#include "factory.h"
#include "costs.h"
#include "extractor.h"
#include "wholesale.h"
#include <iostream>
#include <pcosynchro/pcothread.h>

extern bool requestStop;

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

    for (Seller *seller: wholesalers) {
        NTEST(interface->setLink(uniqueId, seller->getUniqueId()));
    }
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

    //Temps simulant l'assemblage d'un objet.
    PcoThread::usleep((rand() % 100) * 100000);

    // TODO

    NTEST(interface->consoleAppendText(uniqueId, "Factory have build a new object"));
}

void Factory::orderResources() {

    // TODO - Itérer sur les resourcesNeeded et les wholesalers disponibles

    //Temps de pause pour éviter trop de demande
    PcoThread::usleep(10 * 100000);
}

void Factory::run() {
    if (wholesalers.empty()) {
        std::cerr << "You have to give to factories wholesalers to sales their resources" << std::endl;
        return;
    }
    NTEST(interface->consoleAppendText(uniqueId, "[START] Factory routine"));

    while (!requestStop) {
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
    // TODO
    return 0;
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
