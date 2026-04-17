# NFL Draft Simulator

A full-stack NFL mock draft simulator that models real draft behavior using weighted decision logic, team needs, and controlled randomness. Built with a C++ backend and a browser-based frontend, the system simulates realistic draft outcomes across multiple rounds.

---

## Overview

This project simulates the NFL Draft by combining player rankings, team needs, and probabilistic decision-making to generate realistic draft results.

Users can:

* Configure draft settings (teams, rounds, speed)
* Run fully automated simulations
* Observe how teams prioritize players based on dynamic factors

The goal of this project is to model how NFL teams make draft decisions and provide a realistic, replayable draft experience.

---

## Features

### Intelligent Draft Logic

* Teams evaluate players using weighted priorities
* Factors include:

  * Consensus ranking
  * Positional need
  * Player traits (ceiling, floor, athleticism, etc.)
* Each team has a unique priority order

### Controlled Randomness

* Each team includes a configurable noise factor
* Ensures realistic variation between simulations
* Prevents deterministic or repetitive outcomes

### Dynamic Team Needs

* Team positional needs update after each pick
* High-value selections reduce future positional urgency
* Simulates real-world roster building strategy

### Simulation Modes

* Adjustable simulation speeds:

  * Slow
  * Base
  * Fast
* Supports full CPU simulations or partial user control

### Large Player Pool

* Hundreds of draft prospects
* Structured data loaded via JSON
* Easily extendable for future classes

---

## Architecture

### Backend

* Language: C++
* Framework: Crow
* Responsibilities:

  * Draft simulation engine
  * Player/team data processing
  * Scoring and selection logic

### Frontend

* HTML / CSS / JavaScript
* Responsibilities:

  * User configuration UI
  * Draft visualization
  * Interaction with backend API

### Data Layer

* JSON-based configuration:

  * Players
  * Teams
  * Draft order
* Designed for easy modification and scalability

---

## Core Algorithm

Each team evaluates available players using a weighted scoring system:

* Assign weights based on team priorities
* Score each player across multiple attributes
* Apply randomness within a defined range
* Select the highest-scoring player

After each selection:

* Team needs are updated
* Player pool is reduced
* Draft continues to next pick

---

## Scoring Algorithm Details

The draft decision logic is based on a weighted scoring system that evaluates players across multiple factors such as consensus ranking, positional need, and player traits.

A detailed breakdown of the scoring methodology, weight distribution, and decision process can be found here:

(To be added later)

This document outlines:

* Weight assignment based on team priorities
* Player evaluation formulas
* Randomness injection and variability control
* Dynamic updates to team needs after each pick

---

## Design Goals

* Model realistic draft decision-making
* Balance determinism with randomness
* Create a reusable simulation engine
* Build a scalable foundation for future sports analytics projects

---

## Future Improvements

* Save/load draft states
* Player profile pages
* Advanced analytics (ADP, simulation averages)
* Integration with database systems
* Expanded simulation controls
* Multi-sport support (NBA draft)

---

## Made by Ethan Howard

I am a junior Computer Science student at the University of North Texas with a strong interest in backend engineering. This project reflects my focus on building systems that combine data modeling, algorithmic logic, and real-world simulation.

My long-term goal is to transition into machine learning and data engineering, where I can work on large-scale data systems and predictive models.
