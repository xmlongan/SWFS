#pragma once
#include "Fab.h"

using iandj = std::pair<usint, usint>;
using usint_float = std::pair<usint, float>;
using iandj_float = std::pair<iandj, float>;

std::vector<iandj> comb_ij();

usint_float add_one(Fab& fab, const usint& rep = 1);
iandj_float add_two(Fab& fab, const usint& rep = 1);

std::vector<usint_float> greedy_search(const usint& Nmore, Fab& fab, const usint& rep = 1);
std::vector<iandj_float> greedy_search2(const usint& Nmore, Fab& fab, const usint& rep = 1);


void mass_run_margin_one(Fab& fab, usint N_more, usint Rep = 200, usint rep = 1);
void mass_run_margin_two(Fab& fab, usint N_more, usint Rep = 200, usint rep = 1);