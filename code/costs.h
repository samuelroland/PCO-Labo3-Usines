/* Modified by: Vitória Oliveira and Samuel Roland */

#ifndef COSTS_H
#define COSTS_H

#define COPPER_COST 5
#define SAND_COST 6
#define PETROL_COST 5
#define CHIP_COST 10
#define PLASTIC_COST 7
#define ROBOT_COST 15

#define EXTRACOTR_COST 4
#define ELECTRICIAN_COST 6
#define PLASTURGIST_COST 5
#define ENGINEER_COST 7

//If GTEST_CONTEXT is enabled (when running Google Test tests), disable the instruction.
// NTEST means "NOT IN TESTING"
#ifdef GTEST_CONTEXT
#define NTEST(INSTRUCTION)
#else
#define NTEST(INSTRUCTION) INSTRUCTION
#endif

#endif// COSTS_H
