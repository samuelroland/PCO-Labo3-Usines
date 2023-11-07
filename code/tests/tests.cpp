/* Modified by: Vitória Oliveira and Samuel Roland */

#include "../extractor.h"
#include "../factory.h"
#include "../seller.h"
#include "../utils.h"
#include "costs.h"
#include "wholesale.h"
#include <QApplication>
#include <gtest/gtest.h>
#include <map>
#include <pcosynchro/pcothread.h>
#include <stdexcept>
#include <string.h>
#include <unistd.h>

#define GTEST

const int FAKE_SELLER_FUNDS = 100;
static volatile bool massiveTradeIsDone;
class FakeWholesaler : public Wholesale {

public:
    ItemType lastTradeType = ItemType::Nothing;
    int lastTradeQty = 0;
    Seller *target = nullptr;
    int totalMoneySpent = 0;
    int totalItemNumbersBought = 0;

    FakeWholesaler(std::map<ItemType, int> stocks = {{}}) : Wholesale(FAKE_SELLER_FUNDS, 1) {
        this->stocks = stocks;
        massiveTradeIsDone = false;
    }

    int trade(ItemType what, int qty) {
        lastTradeType = what;
        lastTradeQty = qty;
        return qty * getCostPerUnit(what);
    }

    std::map<ItemType, int> getItemsForSale() {
        return std::map<ItemType, int>({{ItemType::Petrol, stocks[ItemType::Petrol]}});
    }

    void massiveTrade(ItemType it) {
        if (target == nullptr) throw std::logic_error("target is null");
        int tradeRes;
        while (!PcoThread::thisThread()->stopRequested() && !massiveTradeIsDone) {
            tradeRes = target->trade(it, 1);
            PcoThread::usleep(20);
            if (tradeRes) {
                totalMoneySpent += tradeRes;
                totalItemNumbersBought++;
            } else {
                massiveTradeIsDone = true;
                return;
            }
        }
    }
};

TEST(Factory, CanTradeItemsInStock) {
    RobotFactory rf(1, FACTORIES_FUND);
    rf.setStocks({{ItemType::Robot, 5}});

    //Can trade 1 and 4 robots
    EXPECT_EQ(rf.trade(ItemType::Robot, 1), ROBOT_COST);
    EXPECT_EQ(rf.getStocks().at(ItemType::Robot), 4);
    EXPECT_EQ(rf.trade(ItemType::Robot, 4), 4 * ROBOT_COST);
    EXPECT_EQ(rf.getStocks().at(ItemType::Robot), 0);

    //Cannot trade because stocks is empty
    EXPECT_EQ(rf.trade(ItemType::Robot, 2), 0);
    EXPECT_EQ(rf.trade(ItemType::Robot, 1), 0);

    //Final fund is correct
    EXPECT_EQ(rf.getFund(), ROBOT_COST * 5 + FACTORIES_FUND);
}

TEST(Factory, CanBuyNeededResourcesToWholesalers) {
    FakeWholesaler fs({{ItemType::Petrol, 2}});
    PlasticFactory pf(1, FACTORIES_FUND);
    ASSERT_EQ(pf.getStocks().size(), 0);
    pf.setWholesalers({&fs});

    //Let the factory run until it buy some Petrol to the wholesaler
    std::unique_ptr<PcoThread> factoryThread;
    factoryThread = std::make_unique<PcoThread>(&PlasticFactory::run, &pf);
    while (fs.lastTradeQty == 0) {}
    factoryThread->requestStop();
    factoryThread->join();

    //Make sure the factory has bought 1 Petrol unit and has stored it
    EXPECT_EQ(fs.lastTradeQty, 1);
    EXPECT_EQ(fs.lastTradeType, ItemType::Petrol);
    EXPECT_EQ(pf.getStocks().at(ItemType::Petrol), 1);
    EXPECT_EQ(pf.getFund(), FACTORIES_FUND - PETROL_COST);
}

TEST(Factory, CanBuildItemWhenItHasRessources) {
    RobotFactory rf(1, FACTORIES_FUND);
    ASSERT_EQ(rf.getStocks().size(), 0);
    EXPECT_EQ(rf.trade(ItemType::Robot, 1), 0);
    rf.setStocks({{ItemType::Plastic, 3}, {ItemType::Chip, 2}});
    EXPECT_EQ(rf.trade(ItemType::Robot, 1), 0);

    //Wait on the robot factory to build a robot
    std::unique_ptr<PcoThread> factoryThread;
    FakeWholesaler fs({{ItemType::Petrol, 2}});
    rf.setWholesalers({&fs});
    factoryThread = std::make_unique<PcoThread>(&RobotFactory::run, &rf);
    while (rf.getStocks().at(ItemType::Robot) == 0) {}

    factoryThread->requestStop();
    factoryThread->join();

    //Make sure the factory has the correct money and stocks after the building of one or more robot
    EXPECT_GE(rf.getStocks().at(ItemType::Robot), 1);
    EXPECT_LE(rf.getStocks().at(ItemType::Chip), 1);
    EXPECT_LE(rf.getStocks().at(ItemType::Plastic), 2);
    EXPECT_LE(rf.getFund(), FACTORIES_FUND - ENGINEER_COST);
}

TEST(Factory, InvalidTradeIsImpossible) {
    FakeWholesaler fs;
    PlasticFactory pf(1, FACTORIES_FUND);
    pf.setStocks({{ItemType::Petrol, 3}});
    pf.setWholesalers({&fs});
    EXPECT_EQ(pf.trade(ItemType::Plastic, 0), 0);//zero plastic asked
    EXPECT_EQ(pf.trade(ItemType::Chip, 1), 0);   //chip is not plastic
    EXPECT_EQ(pf.trade(ItemType::Petrol, 1), 0); //used material is not in sale
    EXPECT_EQ(pf.getFund(), FACTORIES_FUND);
}

void runMassiveTrades(Seller &seller, ItemType it, const int originalQuantity, const int cost) {
    const int SELLERS_NUMBER = 8;
    std::vector<std::unique_ptr<PcoThread>> threads;
    std::vector<FakeWholesaler *> wholesalers;

    for (int i = 0; i < SELLERS_NUMBER; ++i) {
        wholesalers.push_back(new FakeWholesaler());
    }

    for (size_t i = 0; i < SELLERS_NUMBER; ++i) {
        wholesalers[i]->target = &seller;
        threads.emplace_back(std::make_unique<PcoThread>(&FakeWholesaler::massiveTrade, wholesalers[i], it));
    }

    //Wait until there is no left stock in the seller
    while (seller.getStocks().at(it) > 0) {}

    for (auto &thread: threads) {
        thread->requestStop();
    }

    for (auto &thread: threads) {
        thread->join();
    }

    //Final checks
    int totalMoneySpent = 0;
    int totalItemNumbersBought = 0;
    for (size_t i = 0; i < SELLERS_NUMBER; ++i) {
        totalMoneySpent += wholesalers[i]->totalMoneySpent;
        totalItemNumbersBought += wholesalers[i]->totalItemNumbersBought;
    }
    EXPECT_EQ(originalQuantity * cost, totalMoneySpent);
    EXPECT_EQ(originalQuantity, totalItemNumbersBought);
}

TEST(Factory, ConcurrentTradesAreManaged) {
    PlasticFactory pf(1, FACTORIES_FUND);
    const int ORIGINAL_STOCK = 20000;
    pf.setStocks({{ItemType::Plastic, ORIGINAL_STOCK}});

    runMassiveTrades(pf, ItemType::Plastic, ORIGINAL_STOCK, PLASTIC_COST);

    EXPECT_EQ(pf.getStocks().at(ItemType::Plastic), 0);
    EXPECT_EQ(pf.getFund(), FACTORIES_FUND + ORIGINAL_STOCK * PLASTIC_COST);
}

TEST(Extractor, ConcurrentTradesAreManaged) {
    CopperExtractor ce(1, FACTORIES_FUND);
    const int ORIGINAL_STOCK = 10000;
    ce.setStocks({{ItemType::Copper, ORIGINAL_STOCK}});

    runMassiveTrades(ce, ItemType::Copper, ORIGINAL_STOCK, COPPER_COST);

    EXPECT_EQ(ce.getStocks().at(ItemType::Copper), 0);
    EXPECT_EQ(ce.getFund(), FACTORIES_FUND + ORIGINAL_STOCK * COPPER_COST);
}

TEST(EndToEnd, ExpectedMoneyEqualsFinalMoney) {
    Utils utils = Utils(NB_EXTRACTOR, NB_FACTORIES, NB_WHOLESALER);
    sleep(5);
    utils.externalEndService();
    EXPECT_EQ(utils.getFinalReport().toStdString(), "The expected fund is : 2000 and you got at the end : 2000");
}