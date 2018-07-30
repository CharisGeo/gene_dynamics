// -----------------------------------------------------------------------------
//
// Copyright (C) The BioDynaMo Project.
// All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#ifndef DEMO_GENE_REGULATION_H_
#define DEMO_GENE_REGULATION_H_

#include <string>
#include <vector>

#include "biodynamo.h"
#include "my_cell.h"

namespace bdm {

using std::array;
using std::vector;
using std::string;

// 1. Define compile time parameter
template <typename Backend>
struct CompileTimeParam : public DefaultCompileTimeParam<Backend> {
  using BiologyModules = Variant<RegulateGenes, GrowDivide>;
   using AtomicTypes = VariadicTypedef<MyCell>;
};


inline int Simulate(int argc, const char** argv) {
  // 2. Initialize BioDynaMo
  Simulation<> simulation(argc, argv);
  auto* param = simulation.GetParam();

  param->bound_space_ = true;
  param->min_bound_ = 0;
  param->max_bound_ = 500;

  // 3. Initialize RegulateGenes module.
  // To add functions to the module use RegulateGenes::AddGene() function.
  // You should pass to the function two variables.
  // The first is of type  std::function<double(double, double)>.
  // This is the function by which concentration of the protein will be
  // calculated.
  // The second is double. This is the initial value for the protein.
  RegulateGenes regulate_example;
  regulate_example.AddGene(
      [](double curr_time, double last_concentration) {
        return curr_time * last_concentration + 0.2f;
      },
      1);

  RegulateGenes second_example;
  second_example.AddGene(
      [](double curr_time, double last_concentration) {
      return last_concentration * last_concentration * curr_time;
      },
      5);

  // 4. Define initial model -- in this example just one cell.
  auto construct = [&](const std::array<double, 3>& position) {
    MyCell cell(position);
    cell.SetDiameter(40);
    cell.SetMass(1.0);
    cell.SetAdherence(0.4);
    cell.SetCellType(1);
    cell.AddBiologyModule(regulate_example);
    return cell;
  };
 ModelInitializer::CreateCellsRandom(param->min_bound_, param->max_bound_, 1, construct);

   auto create = [&](const std::array<double, 3>& place){
   MyCell cell(place);
   cell.SetDiameter(30);
   cell.SetAdherence(0.4);
   cell.SetMass(1.0);
   cell.SetCellType(-1);
   cell.AddBiologyModule(second_example);
   return cell;
   };
 ModelInitializer::CreateCellsRandom(param->min_bound_, param->max_bound_, 1, create);

// 5. Run simulation
  auto* scheduler = simulation.GetScheduler();
  scheduler->Simulate(1000);

  // 6. Output concentration values for each gene
  auto* rm = simulation.GetResourceManager();
  auto&& cell = (*(rm->Get<MyCell>()))[0];
  const auto* regulate_genes = cell.GetBiologyModules<RegulateGenes>()[0];
  const auto& concentrations = regulate_genes->GetConcentrations();
 
  if (cell->GetCellType() == 1) {
      std::cout << "Gene concentrations after " << scheduler->GetSimulatedSteps()
            << " time steps for Type 1 Cells" << std::endl;
  for (double concentration : concentrations) {
    std::cout << concentration << std::endl;
  }
    } else {
      std::cout << "Gene concentrations after " << scheduler->GetSimulatedSteps()
            << " time steps for Type -1 Cells" << std::endl;
  for (double concentration : concentrations) {
    std::cout << concentration << std::endl;
  }
 }


  std::cout << "Simulation completed successfully!\n";
  return 0;
}

}  // namespace bdm

#endif // DEMO_GENE_REGULATION_H_
