# Top-level overview of [[scuff-em]]

[[scuff-em]] is a free, open-source software 
implementation of the boundary-element method (BEM)
(or the "method of moments") of electromagnetic
scattering. (More specifically, [[scuff-em]]
implements the EFIE and PMCHWT formulations
of the BEM using RWG basis functions.)

[[scuff-em]] originated as a specialized tool
for using BEM techniques to model
fluctuation-induced electromagnetic phenomena---such as
[Casimir forces][FSCCasimirPaper]
and
[radiative heat transfer][FSCNEQPaper]---and 
subsequently expanded into a general-purpose BEM solver targeting a
variety of applications in nanophotonics.

[[scuff-em]] consists of a [core library](../API/libscuff.md),
implementing the basic BEM functionality, plus a large number 
of specialized [application modules](#AvailableApplications) 
designed for specific problems in science and engineering.

[[scuff-em]] stands for **S**urface **CU**rrent/**F**ield **F**ormulation of 
**E**lectro**M**agnetism. This is a reference to the underlying solution 
methodology used by [[scuff-em]] and other BEM solvers, in which we solve 
first for surface currents [including both electric (***K***) and 
magnetic (***N***) currents, as cartooned in the [[scuff-em]] logo], 
then use these currents to compute the scattered fields or other 
quantities of interest.

The entire [[scuff-em]] suite is free software distributed 
under the [GNU GPL][GNUGPL]. The source code for [[scuff-em]] 
may be downloaded from the 
[<span class="SC">scuff-em</span> GitHub page][GitHub]. 
**The GitHub page is also the right place for questions, 
bug reports, feature requests, and other discussion of [[scuff-em]].**

For commercial support and consulting services
related to [[scuff-em]], please contact 
[SIMPETUS Inc.](http://www.simpetuscloud.com)

## Interfaces to [[scuff-em]]

Access to the [[scuff-em]] computational engine is available
via multiple interfaces.

The *command-line interface* consists of a large number
of [command-line applications](#AvailableApplications) for
running various types of standard calculations in computational
physics. Using [[scuff-em]] in this way requires only
that you learn the basic command-line options

The *application programming interface* consists of 
[C++ and python APIs](../API/libscuff.md)
that allow access to internal [[scuff-em]] data structures
and methods for maximal flexibility in implementing your
own custom-designed physics codes.

## Inputs to [[scuff-em]] calculations

Typical inputs to [[scuff-em]] calculations include

+ A [geometry file](Geometries.md) describing the scattering geometry

+ An optional [list of geometric transformations](Transformations.md) 
  to be applied to the geometry, with calculations generally repeated
  at each transformation

+ Specification of the frequencies (and, for extended geometries,
  the Bloch vectors) at which you want to perform calculations

+ For scattering codes: a specification of the 
  [incident fields][IncidentFields.md].

+ Specifications of the output quantities you wish to get back: 
  field components at individual points in space, power/force/torque
  information, Casimir quantities, heat-transfer rates, impedance 
  parameters, capacitances, polarizabilities, etc.

## Outputs from [[scuff-em]] calculations

Typical outputs from [[scuff-em]] calculations include

+ text-based data files reporting output quantities

+ Visualization files written in 
  [<span class="SC">GMSH</span>][GMSH] post-processing
  format.

<a name="AvailableApplications"></a>
## Command-line Applications

### Nanophotonics / electromagnetic scattering 

 + [<span class="SC">scuff-scatter</span>][scuff-scatter]
> A general-purpose solver forproblems involving
> Available outputs include: scattered and total fields
> at arbitrary points in space; visualization of fields 
> and surface currents; absorbed and scattered power;
> force and torque (radiation pressure); induced dipole
> or spherical multipole moments; and more.
> 

 + [<span class="SC">scuff-transmission</span>][scuff-transmission]
> A specialized solver for computing plane-wave transmission
> in 2D extended geometries: thin films, perforated screens,
> nanoparticle arrays, etc. 

 + [<span class="SC">scuff-tmatrix</span>][scuff-tmatrix]
> A specialized code for computing the
> [T-matrices](http://en.wikipedia.org/wiki/T-matrix_method)
> of arbitrary compact scatterers.

### Fluctuation-induced interactions

 + [<span class="SC">scuff-cas3D</span>][scuff-cas3D]
> An implementation of the 
> [fluctuating-surface-current approach to equilibrium Casimir forces][FSCPaper] 
> in compact or extended geometries.

 + [<span class="SC">scuff-caspol</span>][scuff-caspol]
> A tool for computing Casimir-Polder potentials for
> polarizable molecules in the vicinity of compact or 
> extended material bodies.

 + [<span class="SC">scuff-neq</span>][scuff-neq]
> An implementation of the fluctuating-surface-current
> approach to non-equilibrium fluctuation-induced
> interactions among compact objects.
> Available outputs include: frequency-resolved or 
> frequency-integrated rates of heat radiation or 
> radiative heat transfer; non-equilibrium Casimir 
> forces; self-propulsion and self-rotation of 
> isolated bodies.

### RF / microwave engineering

 + [<span class="SC">scuff-RF</span>][scuff-RF]
> A tool for modeling the electromagnetic properties of 
> passive RF devices such as antennas and inductors.
> Available outputs include: frequency-dependent
> S-parameters for arbitrarily-shaped objects;
> radiated field patterns for antennas or other objects
> driven by user-specified currents.

### Electrostatics

 + [<span class="SC">scuff-static</span>][scuff-static]
> An electrostatic solver. 
> Available outputs include: self- and mutual-capacitances
> of arbitrarily-shaped conductors ; DC polarizabilities of
> conducting and dielectric bodies; electrostatic fields
> (at user-specified evaluation points) in the presence 
> of conducting surfaces held at user-specified potentials.

##Citing [[scuff-em]]

If you find [[scuff-em]] useful for generating
results included in publications, please consider citing both 
**(a)** one of the papers discussing the implementation of
[[scuff-em]], and 
**(b)** the URL for the code. For example, if you are writing
in LaTeX, you might write something like this:

````tex
Numerical computations were performed using {\sc scuff-em}, a free,
open-source software implementation of the boundary-element 
method~\cite{SCUFF1, SCUFF2}.
````

Here the ``SCUFF1`` and ``SCUFF2``
references refer to the following ``.bibtex`` entries:

````tex
@ARTICLE{SCUFF1,
author = {{Homer Reid}, M.~T. and {Johnson}, S.~G.},
title = "{Efficient Computation of Power, Force, and Torque in 
BEM Scattering Calculations}",
journal = {ArXiv e-prints},
archivePrefix = "arXiv",
eprint = {1307.2966},
primaryClass = "physics.comp-ph",
keywords = {Physics - Computational Physics, Physics - Classical Physics},
year = 2013,
month = jul,
}

@ARTICLE{SCUFF2,
note="\texttt{http://github.com/homerreid/scuff-EM}"
}
````

[GMSH]: http://www.geuz.org/gmsh
[scuff-scatter]:            ../applications/scuff-scatter/scuff-scatter.md
[scuff-transmission]:       ../applications/scuff-transmission/scuff-transmission.md
[scuff-tmatrix]:            ../applications/scuff-tmatrix/scuff-tmatrix.md
[scuff-cas3D]:              ../applications/scuff-cas3D/scuff-cas3D.md
[scuff-caspol]:             ../applications/scuff-caspol/scuff-caspol.md
[scuff-neq]:                ../applications/scuff-neq/scuff-neq.md
[scuff-RF]:                 ../applications/scuff-RF/scuff-RF.md
[scuff-static]              ../applications/scuff-static/scuff-static.md
[GNUGPL]:                   http://en.wikipedia.org/wiki/GNU_General_Public_License
[GitHub]:                   https://github.com/HomerReid/scuff-em/
[FSCCasimirPaper]:          http://dx.doi.org/10.1103/PhysRevA.88.022514
[FSCNEQPaper]:              http://dx.doi.org/10.1103/PhysRevB.88.054305
