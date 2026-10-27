# BoilerBulk

A personalized meal planning application for Purdue University students that combines dining court menus, nutritional goals, and class schedules to generate practical daily meal plans.

## Overview

Finding meals that fit nutritional goals while balancing classes, workouts, and travel time can be challenging for college students. BoilerBulk aims to simplify this process by analyzing available dining court options and generating meal recommendations tailored to each user's needs.

The application retrieves daily menu information from Purdue's dining API, evaluates available food options against user-defined nutritional targets, and considers scheduling constraints to recommend meals throughout the day.

The project is designed to run locally on Linux, with a C++ core responsible for menu processing, nutritional calculations, and meal selection. Bash scripts handle execution and automation.

## Features

### Personalized Nutrition

- Configure daily calorie and protein targets.
- Specify a fitness goal, such as bulking, cutting, or maintaining weight.
- Account for food consumed outside dining courts, including protein shakes and protein bars.
- Calculate remaining daily nutritional targets before recommending meals.

### Dining Court API 

- Retrieve daily dining court menus through Purdue's dining API.
- Process available menu items and nutritional information.
- Support meal recommendations based on the food available that day.

### Schedule-Aware Planning

- Use a manually entered class schedule to identify potential meal windows.
- Consider dining court operating hours and estimated travel time.
- Recommend meals that fit the user's daily schedule.

### Optimization Modes

The planner is designed to support two meal selection strategies:

**Balanced Mode**

Balances protein intake with practical considerations, including travel time, schedule compatibility, and calorie targets.

**Protein Maximization Mode**

Prioritizes protein intake while respecting calorie limits, scheduling constraints, and dietary restrictions.

### Local Execution and Automation

- Run the planner locally from the command line.
- Store user settings and schedules in JSON configuration files.
- Generate a daily meal plan as a text file.
- Support future automated daily execution through Bash and `cron`.

## Technology Stack

| Technology | Purpose |
|---|---|
| C++17 | Core application and meal selection algorithms |
| Bash | Build, execution, and automation scripts |
| libcurl | HTTP requests to Purdue's dining API |
| nlohmann/json | JSON parsing and configuration management |
| CMake | Build configuration and project compilation |
| Git | Version control |
| cron | Planned daily execution |
| Linux | Primary development and execution environment |

## Prerequisites

Before building the application, install the following tools and libraries:

- A C++17-compatible compiler, such as GCC.
- CMake.
- Git.
- libcurl development libraries.
- The nlohmann/json library.
- Bash.

On Ubuntu or a similar Debian-based Linux distribution, the required packages can generally be installed with:

```bash
sudo apt update
sudo apt install build-essential cmake git libcurl4-openssl-dev nlohmann-json3-dev
```

Package availability may vary by Linux distribution.

## Installation

### 1. Clone the Repository

```bash
git clone git@github.com:eroxyi/BoilerBulk.git 
cd BoilerBulk 
```

### 2. Configure the Project

Generate the build files with CMake:

```bash
cmake -S . -B build
```

### 3. Build the Application

```bash
cmake --build build
```

### 4. Run the Application

```bash
./build/meal_planner
```

## Configuration

The application uses JSON files to separate user-specific settings from the program logic.

### User Profile

The user profile stores nutritional targets and meal planning preferences.

Example:

```json
{
  "weight_kg": 75,
  "goal": "bulk",
  "daily_calorie_target": 2800,
  "daily_protein_target_g": 150,
  "optimization_mode": "balanced",
  "dietary_restrictions": []
}
```

These values are illustrative and should be adjusted to the user's needs.

### Class Schedule

The class schedule records class times and locations so the planner can identify potential meal windows.

Example:

```json
{
  "classes": [
    {
      "day": "Monday",
      "start": "09:30",
      "end": "10:20",
      "location": "WALC"
    },
    {
      "day": "Monday",
      "start": "13:30",
      "end": "14:20",
      "location": "POTR"
    }
  ]
}
```

### External Food Log

The external food log records nutritional intake from sources outside the dining courts.

Example:

```json
{
  "foods": [
    {
      "name": "Protein Shake",
      "calories": 160,
      "protein_g": 30,
      "quantity": 1
    }
  ]
}
```

The planner can use these records to subtract consumed calories and protein from the user's daily targets.

## How It Works

The intended planning pipeline consists of the following steps:

1. **Load configuration:** Read the user's nutritional targets, class schedule, dietary restrictions, and external food intake.
2. **Retrieve menus:** Request daily dining court menu data from Purdue's dining API.
3. **Parse menu data:** Extract available food items, dining locations, and nutritional information.
4. **Calculate remaining targets:** Account for externally consumed food to determine remaining daily calorie and protein goals.
5. **Identify meal windows:** Use the class schedule, dining hours, and estimated travel times to identify feasible meal opportunities.
6. **Optimize meal selections:** Apply the selected strategy to evaluate available meals.
7. **Generate the plan:** Organize meal recommendations into a daily schedule.
8. **Save the results:** Write the generated plan to a text file for convenient reference.

## Output

The application is intended to generate a daily meal plan in:

```text
output/daily_plan.txt
```

The plan may include:

- Recommended dining court.
- Suggested meal time.
- Selected food items.
- Estimated calories and protein per meal.
- Daily nutritional totals.
- Remaining calorie and protein targets.

## Privacy

The application is designed to run locally and store configuration and output files on the user's computer.

User profiles and schedules may contain personal information. These files should not be committed to a shared Git repository. Example configuration files should contain only fictional or non-personal values.

## License

This project is licensed under the Apache License, Version 2.0. See the [LICENSE](LICENSE) file for the full license text.

Copyright 2026 Purdue Meal Planner contributors.
