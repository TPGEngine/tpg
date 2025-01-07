# Tangled Program Graphs (TPG)

Developer Names: 
- Cyruss Amante
- Calvyn Siong
- Mark Cruz
- Edward Gao
- Richard Li

Date of project start: September 12, 2024

This project is developing an interface to test the evolutionary machine learning framework Tangle Programming Graphs (TPG) in a robotic simulation engine called MuJoCo created by Google Deepmind.


### **Key Features**

1. **Integration with new complex environments like Mujoco**  

TPG framework will have numerous integrations between basic agents and several environments provide by Mujoco

2. **CI/CD integration to improve development practises**

The TPG framework will have an integrated CI/CD pipeline that will automatically run linting processes, build project to ensure it's compatible on different platforms with new changes and run automated unit tests

3. **Experiments to measure reinforncement learning performance**

Series of experiments to evaluate the behavior and performance of agents controlled by the TPG algorithm within the MuJoCo environment.



The folders and files for this project are as follows:

`docs` - Documentation for the project
`refs` - Reference material used for the project, including papers
`src` - TPG source code cloned using Git Subtree
`src/src/cpp` - Contains different experiments and models (Classic Control and MuJoCo)
`src/scripts` - Contains environment commands containing useful scripts for running and plotting experiments 
`test` - Test cases

## Background

The code in `src` reproduces results from the paper:

Stephen Kelly, Tatiana Voegerl, Wolfgang Banzhaf, and Cedric Gondro. Evolving Hierarchical Memory-Prediction Machines in Multi-Task Reinforcement Learning. Genetic Programming and Evolvable Machines, 2021.

To learn more, read this [PDF](https://link.springer.com/epdf/10.1007/s10710-021-09418-4?sharing_token=JXpw69MCJpHudtVwbwbjzPe4RwlQNchNByi7wbcMAY6UliAwn5GntUdmTAY_mnFzVDzBjsFnj4emNyqwnsRvyvXV3pLgfSINPIbIY7CthuAHi9ud7gHQbNpqk5zSEhF9e).
