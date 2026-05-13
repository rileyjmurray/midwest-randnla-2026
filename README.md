# Easy* Python bindings for RandBLAS and RandLAPACK

This repo holds materials for a tutorial I gave at the inaugural Midwest Randomized Linear Algebra Workshop (2026,
UW Madison). 
The point of the tutorial is to show people how coding agents can dramatically simplify the process of making
bespoke Python bindings for C++ functions in RandBLAS or RandLAPACK.
The lessons are portable to making bindings for other C++ libraries.

Because the tutorial was 45 minutes (short!) and interactive (slow!) my goal was to get people to expose a single RandLAPACK function to Python.
I chose "BQRRP" as that function.
This is a method for computing a full column-pivoted Householder QR decomposition described in
https://arxiv.org/abs/2507.00976v1. 
BQRRP is a good tutorial candidate because LAPACK's column-pivoted Householder QR
method (GEQP3) is agonizingly slow.
What's more, it's not practical to implement BQRRP in Python using the standard
NumPy/SciPy stack.
So a Python wrapper of RandLAPACK's C++ implementation significantly opens up the 
usefulness of column-pivoted QR in applications.

This repo has a few branches.
 * `main`. Use this if you want to work through the tutorial like an exercise.
   It has the least content of all the branches.
 * `scratch` (the current branch). Use this to understand the repo's purpose
   and have access to useful Python bindings, with a pinned version of RandLAPACK.
   This branch has a weird name to discourage agents from investigating its contents.
 * `mid`. This is between `main` and `scratch`. It has a working Python wrapper
   of BQRRP. That wrapper isn't as full-featured as what's on `scratch`.


## The flow of the tutorial

The tutorial structure is in the final slide of `participants/presentation.pdf`.
In my live demo the coding agent was configured to request permissions before running any 
shell commands. 
I deliberately used a not-fully-frontier language model, and
I set my agent to use high-but-not-maximal reasoning effort.
(If I was whipping together bindings for myself I'd have run the latest-available
model with maximum reasoning effort.)


### Step 1: follow written directions

Step 1 is to follow setup instructions at `participants/project-instructions.md`.
To some peoples' dismay the instructions include using Docker.
You're welcome to try the tutorial without Docker, but there are some things
to be aware of.
 * You need CMake, a C++20-compatible compiler, Google Test (aka "gtest"),
   OpenMP, and BLAS/LAPACK libraries.
 * You'll need CMake builds of BLAS++ and LAPACK++, which provide C++ APIs for 
   BLAS and LAPACK. You'll also need to download Random123.
 * RandLAPACK's CMake configuration line in `project-instructions.md` references
   environment variables for locations of BLAS++, LAPACK++, and Random123
   dependencies. These environment variables are only appropriate for the 
   Docker container used in the tutorial. You'll have to set them yourself if
   you don't use Docker.

This step is complete when (1) RandLAPACK is installed and (2) you can run
the following shell commands successfully.
```bash
cd bindings
pip install -e .
pytest tests
```

### Step 2: interacting with the agent

To get experience interacting with coding agents I asked people to run the following prompt.
```
Please read AGENTS.md, then create a function suparla.add(a, b) that takes two float64 scalars and returns their sum.
```
My machine's agent made a mistake in the live demo.
It appropriately updated `bindings/src/suparla_module.cc`, but it didn't update `bindings/python/suparla/__init__.py`.
We noticed this after trying to run tests it wrote for `add`;
all we had to do was update the `__init__.py` file so its first line was as follows.
```python
from ._suparla import hello, add  # "add" was missing
```
After that change the tests passed and the `add` function worked as expected.

### Step 3: doing something interesting (what happened live)

The prompt I used is very much in the spirit of vibe coding.
```
Help me expose RandLAPACK’s BQRRP algorithm to Python so it can
operate on NumPy arrays.
```
This sort of prompt is appropriate for something interactive;
it would have been a terrible prompt if doing "real" work.

*Note*. In the tutorial I should have put the coding agent in planning mode
before issuing this prompt. That was a real oversight on my part.

The agent did a so-so job.
It updated `bindings/CMakeLists.txt`, 
`bindings/suparla_module.cc `, and `bindings/python/suparla/__init__.py`.
It didn't provide tests in its initial response; I had to ask for tests.
There were also two issues with the CMakeLists.txt edits.
These issues were very mild and could have easily been detected and
fixed if we were running an autonomous agent.

The first error was in this line.
```CMake
target_link_libraries(_suparla PRIVATE RandLAPACK)  # wrong
```
That line should have been as follows.
```CMake
target_link_libraries(_suparla PRIVATE RandLAPACK::RandLAPACK)  # correct! 
```
The agent figured out the solution to its mistake when I handed it
the error output when I tried to run `pip install -e .`.

The second CMake error only appeared when you tried to import `suparla` in a Python session.
The error happened because our Docker environment didn't set the
LD_LIBRARY_PATH environment variable to include the locations
of BLAS++ and LAPACK++ (RandLAPACK's key dependencies).
This error could have been fixed in a few ways.
The agent's chosen fix was to edit CMakeLists.txt so that it
didn't rely on us setting any environment variables.

*Note*. I didn't have time to run the tests that the agent
ended up writing. After ceding the floor I ran the tests and saw
that one failed. The failing test was based on an "understandable
misunderstanding" of BQRRP's semantics.
The misunderstanding concerned an instance variable called `rank`
that gets computed during BQRRP's execution.
That variable really should be named `rank_bound`, since we only
promise that it's an upper bound on the numerical rank of the matrix.
In essence, the agent failed to respect the following source code comment.
```
        // CRITICAL: `rank` is only an **upper bound** on the numerical rank of the matrix.
        // It's perfectly valid to set rank=n for an n-by-n zero-matrix.
        int64_t rank;
```
I consider this mistake of modest significance.
The function we're having the agent wrap is a few hundred lines,
and the comment above is the only place where we explain the nature
of the `rank` variable.

