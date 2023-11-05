#include "../factory.h"
#include "../extractor.h"
#include "../seller.h"
#include "../utils.h"
#include "costs.h"
#include "wholesale.h"
#include <QApplication>
#include <gtest/gtest.h>
#include <map>
#include <pcosynchro/pcothread.h>
#include <string.h>
#include <unistd.h>

#define GTEST

const int FAKE_SELLER_FUNDS = 100;
class FakeWholeSeller : public Wholesale {
public:
    ItemType lastTradeType = ItemType::Nothing;
    int lastTradeQty = 0;

    FakeWholeSeller(std::map<ItemType, int> stocks = {{ItemType::Plastic, 4}}) : Wholesale(FAKE_SELLER_FUNDS, 1) {
        this->stocks = stocks;
    }

    int trade(ItemType what, int qty) {
        lastTradeType = what;
        lastTradeQty = qty;
        return qty * getCostPerUnit(what);
    }

    std::map<ItemType, int> getItemsForSale() {
        return std::map<ItemType, int>({{ItemType::Petrol, stocks[ItemType::Petrol]}});
    }

    void setStocks(std::map<ItemType, int> stocks) { this->setStocks(stocks); }
};

TEST(FactoryTrade, CanTradeItemsInStock) {
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

TEST(FactoryTrade, CanBuyNeededResourcesToWholesalers) {
    FakeWholeSeller fs({{ItemType::Petrol, 2}});
    PlasticFactory rf(1, FACTORIES_FUND);
    ASSERT_EQ(rf.getStocks().size(), 0);
    rf.setWholesalers({&fs});

    //Let the factory run until it buy some Petrol to the wholeseller
    std::unique_ptr<PcoThread> factoryThread;
    factoryThread = std::make_unique<PcoThread>(&PlasticFactory::run, &rf);
    while (fs.lastTradeQty == 0) {}
    factoryThread->requestStop();
    factoryThread->join();

    //Make sure the factory has bought 1 Petrol unit and has stored it
    EXPECT_EQ(fs.lastTradeQty, 1);
    EXPECT_EQ(fs.lastTradeType, ItemType::Petrol);
    EXPECT_EQ(rf.getStocks().at(ItemType::Petrol), 1);
    EXPECT_EQ(rf.getFund(), FACTORIES_FUND - PETROL_COST);
}

//TODO: can build item
    Utils utils = Utils(NB_EXTRACTOR, NB_FACTORIES, NB_WHOLESALER);
    sleep(2);
    utils.externalEndService();
    // EXPECT_EQ(factory->getFund(), FACTORIES_FUND);
    // Extractor e(0, 0, ItemType::Sand);
    EXPECT_EQ(utils.getFinalReport().toStdString(), "The expected fund is : 2000 and you got at the end : 2000");
}
