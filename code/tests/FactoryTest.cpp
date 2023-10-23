#include "../factory.h"
#include "../extractor.h"
#include "../seller.h"
#include "../utils.h"
#include <QApplication>
#include <gtest/gtest.h>
#include <pcosynchro/pcothread.h>
#include <string.h>
#include <unistd.h>

#define GTEST

TEST(FactoryTrade, CannotTradeWithoutResources) {
    PlasticFactory se(12, EXTRACTOR_FUND);
    ASSERT_EQ(se.getFund(), EXTRACTOR_FUND);
    ASSERT_EQ(se.trade(ItemType::Plastic, 10), 0);
    ASSERT_EQ(se.trade(ItemType::Chip, 10), 0);
}

TEST(EndToEnd, ExpectedMoneyEqualsFinalMoney) {

    Utils utils = Utils(NB_EXTRACTOR, NB_FACTORIES, NB_WHOLESALER);
    utils.externalEndService();
    // ASSERT_EQ(factory->getFund(), FACTORIES_FUND);
    // Extractor e(0, 0, ItemType::Sand);
    ASSERT_EQ(utils.getFinalReport().toStdString(), "The expected fund is : 2000 and you got at the end : 2000");
}