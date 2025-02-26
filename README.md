#### Supporting data for the paper:
## "Everything everywhere all at once, a probability-based enhanced sampling approach to rare events"
Enrico Trizio, Peilin Kang and Michele Parrinello

[![ArXiv](https://img.shields.io/badge/arXiv-2410.17029-lightblue)](https://arxiv.org/abs/2410.17029)

The training of the models was based on the [mlcolvar library](https://github.com/luigibonati/mlcolvar), where the updated relevant code and updated example notebooks are available:
- [didactical example: Muller-brown](https://github.com/luigibonati/mlcolvar/blob/main/docs/notebooks/tutorials/cvs_committor.ipynb)
- [practical example: Alanine with distances](https://github.com/luigibonati/mlcolvar/blob/main/docs/notebooks/examples/ex_committor.ipynb)

The results of the paper were obtained using the [1.2.2 version of the library](https://github.com/luigibonati/mlcolvar/tree/v1.2.2).

---

### Repo structure
The contents of the repository are organized as follows:

#### muller : files for Muller-Brown 2D potential using `PLUMED ves_md_linearexpansion`
In the main text, data from `iter_2_long`, `iter_2_long_opes_only` are reported
  - **unbiased_sims**: input files (`md_input`, `md_potential` and `plumed.dat`) and results (`COLVAR`) of the unbiased simulations used in the boundary conditions labeled dataset (`A` and `B` folders, respectively)
  - **biased_sims**: biased simulations inputs and results from different iterations
    - **generate_and_run_sims.sh**: bash script to set up and run simulations for a given iteration with plumed `ves_md_linearexpansion` starting from a template folder (see below)
    - **template**: template folder for biased simulations, including the input files for starting simulations from A and B basins (`input_md_A.dat`, `input_md_B.dat`, `input_md-potential.dat`, `plumed.dat`)
    - **iter_***: input files (`input_md_A.dat`, `input_md_B.dat`, `input_md-potential.dat`, `plumed.dat`) and results (`A/COLVAR` and `B/COLVAR`) obtained from simulations at different iterations of the procedure. `COLVAR` files have been used to incrementally build the variational loss dataset.
    - **iter_2_long***: input files (`input_md_A.dat`, `input_md_B.dat`, `input_md-potential.dat`, `plumed.dat`) and results (`A/COLVAR` and `B/COLVAR`) obtained from longer production simulations at the end of the iterative procedure. `COLVAR` files have been used to obtain the results reported in the main text.
  - **models**: frozen torchscript models for committor (`model_*_q.pt` files) and for z (`model_*_z.pt` files) for the different iterations


#### alanine : files for Alanine dipeptide in vacuum using `Gromacs`
In the main text, data from `iter_4`, `iter_4_opes_only` and `ref_phi_psi` are reported.
  - **unbiased_sims**: unbiased simulations data used in the boundary conditions labeled dataset (`COLVAR_A` and `COLVAR_B` files)
  - **biased_sims**: biased simulations inputs and results from different iterations
    - **generate_and_run_sims.sh**: bash script to set up and run simulations for a given iteration with `Gromacs` starting from a template folder (see below)
    - **template**: template folder for biased simulations, including the input files for starting simulations from A and B basins (`input_A.tpr`, `input_B.tpr`, `plumed.dat`)
    - **iter_***: input files (`input_A.tpr`, `input_B.tpr`, `plumed.dat`) and results (`A/COLVAR` and `B/COLVAR`) obtained from simulations at different iterations of the procedure. `COLVAR` files have been used to incrementally build the variational loss dataset.
    - **ref_phi_psi**: files for reference OPES simulation using Phi and Psi as CVs 
  - **models**: frozen torchscript models for committor (`model_*_q.pt` files) and for z (`model_*_z.pt` files) for the different iterations


#### two_paths_potential : files for double path 2D potential using `PLUMED ves_md_linearexpansion`
In the main text, data from `iter_2` are reported
  - **unbiased_sims**: input files (`input_md.dat`, `input_md-potential.dat` and `plumed.dat`) and results (`COLVAR`) of the unbiased simulations used in the boundary conditions labeled dataset (`A` and `B` folders, respectively)
  - **biased_sims**: biased simulations inputs and results from different iterations
    - **generate_and_run_sims.sh**: bash script to set up and run simulations for a given iteration with plumed `ves_md_linearexpansion` starting from a template folder (see below)
    - **template**: template folder for biased simulations, including the input files for starting simulations from A and B basins (`input_md_A.dat`, `input_md_B.dat`, `input_md-potential.dat`, `plumed.dat`)
    - **iter_***: input files (`input_md_A.dat`, `input_md_B.dat`, `input_md-potential.dat`, `plumed.dat`) and results (`A/COLVAR` and `B/COLVAR`) obtained from simulations at different iterations of the procedure. `COLVAR` files have been used to incrementally build the variational loss dataset.
  - **models**: frozen torchscript models for committor (`model_*_q.pt` files) and for z (`model_*_z.pt` files) for the different iterations


#### chignolin : files for chignolin folding in explicit water using `Gromacs`
Training data are too large for Github, they will be available in the Zenodo repo associated with the published paper.
  - **models**: frozen torchscript models for committor (`model_*.pt`, `model_*.para` files) and for z (`model_*_zeta.pt` files) for the different iterations
  - **sims/template**: template folder for biased and unbiased simulations, including files for starting simulations from folded and unfolded states (`folded.gro`, `unfolded.gro`, `mdT.mdp`, `topol.top`), PLUMED input files (`plumed.dat`, `plumed-descriptors.dat`, `plumed_position.dat`) and the additional PLUMED files (`chignolin-ref.pdb` for molinfo and `chignolin-ca.pdb` for RMSD calculation)
  - **models**: frozen torchscript models for committor (`model_*.pt`, `model_*.para` files) and for z (`model_*_zeta.pt` files) for the different iterations
  - **sims/template**: template folder for biased and unbiased simulations, including files for starting simulations from bound and unbound states (`b.gro`, `u.gro`, `NVT.mdp`, `topol.top`), PLUMED input files for unbiased and biased simulations (`plumed.dat`, `plumed_biased.dat`) and the additional PLUMED files (`conf_template.pdb` for alignment and `TEST_positions.cpp` for printing only atoms within the water coordination points neighborlist)



#### calixarene : files for OAMe-G2 binding in explicit water using `Gromacs`

  - **models**: frozen torchscript models for committor (`model_*.pt`, `model_*.para` files) and for z (`model_*_zeta.pt` files) for the different iterations
  - **sims/template**: template folder for biased and unbiased simulations, including files for starting simulations from bound and unbound states (`b.gro`, `u.gro`, `NVT.mdp`, `topol.top`), PLUMED input files for unbiased and biased simulations (`plumed.dat`, `plumed_biased.dat`) and the additional PLUMED files (`conf_template.pdb` for alignment and `TEST_positions.cpp` for printing only atoms within the water coordination points neighborlist)
