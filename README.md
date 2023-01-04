# SWFS
cpp codes to simulate the production of wafer lots in Semiconductor Wafer Fabrication Systems

## Simulation Model
The SWFS is implemented in source file `Fab.cpp` with header file `Fab.h`. There are 12 stations each of which may have several identical machines. Meanwhile there is only one type of wafer to produce whose processing procedure consists of 60 steps. You can test the simulation with all default settings by simplily compile and run the project.

If you want to check the details, you first uncomment the following codes in `Fab.h`:

```
//std::string message{};//machine, buffer: old state -> new state;
//void update_state_and_record(const size_t& ei);// ei: event index
//void simulation_and_record();
//void write_to_html(std::string fname = "SWFS-state-update-record.html");
//std::string html_buffers(usint bNo1 = 0, usint bNo2 = 0);
//std::string html_machines_SEL(usint eNo = 0, usint mNo1 = 0, usint mNo2 = 0);
//std::string html_NoIbOb(usint si, usint bNo);
```

then uncomment the corresponding implementations in the source file `Fab.cpp`:

```
//void Fab::update_state_and_record(const size_t& ei) {
// ...
//}
//
//void Fab::simulation_and_record() {
// ...
//}
//
//void Fab::write_to_html(string fname) {
// ...
//}
//
//string Fab::html_buffers(usint bNo1, usint bNo2) {
// ...
//}
//
//string Fab::html_machines_SEL(usint eNo, usint mNo1, usint mNo2) {
// ...
//}
//
//string Fab::html_NoIbOb(usint si, usint bNo) {
// ...
//}
```

After the two steps, we can use the method `simulation_and_record()` to run the simulation and the intermediate state changes will be written in file `SWFS-state-update-record.html` which we can explore with any browser. Note that the `head.html` and `foot.html` are building blocks for the former html file.

- `lessOb_moreIb(const usint& i, const usint& j)` and `maxIb_minOb(const usint& si)`: wafer lots scheduling, select the max incoming buffer with the least outgoing buffer, excluding those outgoing buffers with 0 incoming buffer.
- `reset()`, `add_machine(const usint& sNo)` and `del_machine(const usint& sNo)`: reset SWFSs.
- `rexp(const float& mean)`: generating exponential random variables with parameter `mean`.

## Marginal Machine Allocation Algorithms

In order to maximize the throughput of SWFSs by allocating proper number of machines into different stations, I implemented a series of *Marginal Machine Allocation Algorithms* in `margin_alloc.cpp` with head file `margin_alloc.h`. The two algorithms are *Marginal One Machine Allocation Algorithm* and *Marginal Two Machines Allocation Algorithm*. 

- `mass_run_margin_one(Fab& fab, usint N_more, usint Rep = 200, usint rep = 1)`
- `mass_run_margin_two(Fab& fab, usint N_more, usint Rep = 200, usint rep = 1)`
- 
- `std::vector<usint_float> greedy_search(const usint& Nmore, Fab& fab, const usint& rep = 1)`: greedy using `add_one()`
- `std::vector<iandj_float> greedy_search2(const usint& Nmore, Fab& fab, const usint& rep = 1)`: greedy using `add_two()`
- 
- `usint_float add_one(Fab& fab, const usint& rep = 1)`: each time allocate one machine
- `iandj_float add_two(Fab& fab, const usint& rep = 1)`: each time allocate two machines
