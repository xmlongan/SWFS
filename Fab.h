#pragma once
#include <iostream>
#include <vector>
#include <utility>
#include <random>
#include <chrono>
#include <limits>
#include <iterator>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <iomanip>
#include <sstream>
#include <fstream>


using usint = unsigned short;
using vv_usint = std::vector<std::vector<usint>>;
using v_int = std::vector<int>;
using v_usint = std::vector<usint>;
using v_float = std::vector<float>;
const float inf = std::numeric_limits<float>::infinity();

class Fab
{
public:
	const usint step_sNo[60] = { 1,2,3,8,10,1,2,6,1,2,3,8,9,1,2,3,8,10,1,6,11,1,2,5,1,2,3,8,9,1,
								2,3,8,10,1,6,11,1,2,5,1,2,3,8,9,10,11,1,2,4,8,3,1,2,7,8,10,1,2,12 };
	usint buffers[60] = { 0 };
	const float mpts[12] = {0.5f, 0.375f, 2.5f, 1.8f, 0.9f, 1.2f, 1.8f, 0.8f, 0.58f, 3.0f, 1.1f, 2.5f};
	const float mtbfs[12] = {150.0f, 200.0f, 200.0f, 200.0f, 200.0f, 200.0f, 200.0f, 150.0f, 200.0f, 130.0f, 200.0f, 200.0f};
	const float mttrs[12] = {5.0f, 9.0f, 5.0f, 1.0f, 1.0f, 1.0f, 1.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f};
	const vv_usint ibNos{ {1,6,9,14,19,22,25,30,35,38,41,48,53,58},
						  {2,7,10,15,23,26,31,39,42,49,54,59},
						  {3,11,16,27,32,43,52},
						  {50},
						  {24,40},
						  {8,20,36},
						  {55},
						  {4,12,17,28,33,44,51,56},
						  {13,29,45},
						  {5,18,34,46,57},
						  {21,37,47},
						  {60} };// incoming buffer Nos for each station
	float t = 0.0f;
	usint target_lots = 0;
	usint produced_lots = 0;
	usint mcumsums[12] = {0};// machine count cumsums = {n1, n1+n2,n1+n2+n3,...,n1+...+n12}
	v_int machines{};// machine: -1:failed; 0:idle; j:working on j step of some lot
	v_usint sis{};// station index for each machine
	v_float SEL{};//[time-to-occur]

	Fab() {};
	// usint machineCounts[12] = {4,3,10,1,1,2,1,4,1,9,2,2};
	Fab(usint* machineCounts);
	void initialize_state(const usint& targetLots, const float& start_time);

	void update_state(const size_t& ei);
	
	//schedule policy implementation, starve-avoiding
	bool lessOb_moreIb(const usint& i, const usint& j);// i: bNo, j: bNo
	usint maxIb_minOb(const usint& si);
	//

	void simulation();

	void reset();
	void add_machine(const usint& sNo);
	void del_machine(const usint& sNo);

	float rexp(const float& mean);

	//// record state change for debugging
	//std::string message{};//machine, buffer: old state -> new state;
	//void update_state_and_record(const size_t& ei);// ei: event index
	//void simulation_and_record();
	//void write_to_html(std::string fname = "SWFS-state-update-record.html");
	//std::string html_buffers(usint bNo1 = 0, usint bNo2 = 0);
	//std::string html_machines_SEL(usint eNo = 0, usint mNo1 = 0, usint mNo2 = 0);
	//std::string html_NoIbOb(usint si, usint bNo);
};
