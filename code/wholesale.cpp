#include "wholesale.h"
#include "costs.h"
#include "factory.h"
#include <iostream>
#include <pcosynchro/pcothread.h>

extern bool requestStop;

WindowInterface *Wholesale::interface = nullptr;

Wholesale::Wholesale(int uniqueId, int fund)
    : Seller(fund, uniqueId) {
    NTEST(interface->updateFund(uniqueId, fund));
    NTEST(interface->consoleAppendText(uniqueId, "Wholesaler Created"));
}

void Wholesale::setSellers(std::vector<Seller *> sellers) {
    this->sellers = sellers;

    for (Seller *seller: sellers) {
        NTEST(interface->setLink(uniqueId, seller->getUniqueId()));
    }
}

void Wholesale::buyResources() {
    auto s = Seller::chooseRandomSeller(sellers);
    auto m = s->getItemsForSale();
    auto i = Seller::chooseRandomItem(m);

    if (i == ItemType::Nothing) {
        /* Nothing to buy... */
        return;
    }

    int qty = rand() % 5 + 1;
    int price = qty * getCostPerUnit(i);

    NTEST(interface->consoleAppendText(uniqueId, QString("I would like to buy %1 of ").arg(qty) %
                                                         getItemName(i) % QString(" which would cost me %1").arg(price)));
    /* TODO (OK) */
    mutex.lock();
    if(price <= money && s->trade(i, qty)){
        stocks[i] += qty;
        money -= price;
    }
    mutex.unlock();

}

void Wholesale::run() {

    if (sellers.empty()) {
        std::cerr << "You have to give factories and mines to a wholeseler before launching is routine" << std::endl;
        return;
    }

    NTEST(interface->consoleAppendText(uniqueId, "[START] Wholesaler routine"));
    while (!requestStop) {
        buyResources();
        NTEST(interface->updateFund(uniqueId, money));
        NTEST(interface->updateStock(uniqueId, &stocks));
        //Temps de pause pour espacer les demandes de ressources
        PcoThread::usleep((rand() % 10 + 1) * 100000);
    }
    NTEST(interface->consoleAppendText(uniqueId, "[STOP] Wholesaler routine"));
}

std::map<ItemType, int> Wholesale::getItemsForSale() {
    return stocks;
}

int Wholesale::trade(ItemType it, int qty) {

    // TODO
    mutex.lock();
    if(qty > 0 && stocks[it] >= qty){
        //mettre à jour les stocks & les fonds
        stocks[it] -= qty;
        interface->updateStock(uniqueId, &stocks);

        int tradeProfit = getCostPerUnit(it) * qty;
        money += tradeProfit;
        interface->updateFund(uniqueId, money);

        mutex.unlock();
        return tradeProfit;
    }
    mutex.unlock();

    return 0;
}

void Wholesale::setInterface(WindowInterface *windowInterface) {
    interface = windowInterface;
}
