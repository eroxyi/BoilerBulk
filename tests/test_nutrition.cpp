#include <cassert>
#include <cmath>
#include <iostream>
#include "optimize.hpp"

int main() {
  Profile p;
  p.weightKg = 75.0;
  p.goal = Goal::Bulk;
  assert(p.resolve());
  assert(p.calorieTarget == 2850);
  assert(std::abs(p.proteinTarget - 150.0) < 1e-6);
  assert(p.remainingCalorieTarget == 2850);
  assert(std::abs(p.remainingProteinTarget - 150.0) < 1e-6);

  ExternalFood shake;
  shake.name = "Protein Shake";
  shake.calories = 160;
  shake.protein = 30.0;
  shake.quantity = 2;
  p.addExternalFood(shake);

  assert(p.externalCalories == 320);
  assert(std::abs(p.externalProtein - 60.0) < 1e-6);
  assert(p.remainingCalorieTarget == 2530);
  assert(std::abs(p.remainingProteinTarget - 90.0) < 1e-6);

  ExternalFood bar;
  bar.name = "Protein Bar";
  bar.calories = 200;
  bar.protein = 20.0;
  bar.quantity = 1;
  p.addExternalFood(bar);

  assert(p.externalCalories == 520);
  assert(std::abs(p.externalProtein - 80.0) < 1e-6);
  assert(p.remainingCalorieTarget == 2330);
  assert(std::abs(p.remainingProteinTarget - 70.0) < 1e-6);

  ExternalFood feast;
  feast.name = "Feast";
  feast.calories = 5000;
  feast.protein = 200.0;
  feast.quantity = 1;
  p.addExternalFood(feast);

  assert(p.remainingCalorieTarget == 0);
  assert(p.remainingProteinTarget == 0.0);

  std::cout << "All nutrition tests passed successfully.\n";
  return 0;
}
