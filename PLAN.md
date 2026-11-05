## setup the "tests" it needs to pass für Problem1: DONE

* tested on Lid-driven cavity + 4 more scenarios: <https://numsim-exercises.readthedocs.io/en/latest/exercise1/hints.html#reference-solution>

  * [input: parameters.txt, output: “.vti” files (.txt files are not required, just useful for debugging)](https://numsim-exercises.readthedocs.io/en/latest/exercise1/submission.html)

## suche in nextcloud, etc nach passenden "vorlagen": DONE

## how to eval "vorlagen"?: DONE

`python3 Exercise1/Submission/test/tools/compare_vti_outputs.py JULIEN_localonly/test/0_lid_driven_cavity/out JULIEN_localonly/TEMPLATES/Abschlussprojekt_official_WS21/build/out_`

## eval these "vorlagen": DONE

###### Abgabe1\_student\_WS18: runs, DONT USE (large errors, outdated apis, ..)

###### NumSim\_official\_WS21: runs, DONT USE(contains paralellizations, decent 1e-2/1e-3 errors)

###### Abschlussprojekt\_official\_WS21: runs. USE (decent 1e-2/1e-3 errors,)

## implement stuff (with **Abschlussprojekt\_official\_WS21 as reference**): TODO

* **refactor AbschProj: YES**

  *
  * renaming in-file stuff (functions, variables...). removing comments?
  * but try to keep the API the same (function names, arguments, etc) so i can easilybolt on stuff for Problem2, Problem3, etc later
  * source code hiding: e.g. renaming variables, restructuring
  * output hiding: e.g. change orders of summations, maybe even subtle changes as long as they dont mess up the 1e-2/1e-3 accuracy

`rm -rf build`\
`mkdir -p build && cd build`\
`cmake -DCMAKE_BUILD_TYPE=Release ..`\
`make install`

`./numsim ../test/0_driven_cavity/input_parameters.txt`

`python3 ../test/tools/compare_vti_outputs.py ../test/0_driven_cavity/reference_output/ out/`

`Reference: ../test/0_driven_cavity/reference_output/`

`Generated: out/`

`Frames: reference=384, generated=400; mean dt: reference=0.026044386, generated=0.025 (relative difference 4.01%)`

`PASS: pressure maximum absolute difference 4.795936e-05 (tolerance 1.000000e-02)`

`PASS: velocity maximum absolute difference 1.572287e-04 (tolerance 1.000000e-02)`

`SUCCESS: Reference comparison passed.`

* **then port needed parts to NumSimUE: YES**

* keep the "full" AbschlussProject in my julien-dev branch (for later submissions)

- only parts that are strictly required by Problem1 (e.g. no voluntary stuff). is there a way to cleanly turn off/remove features that would only be implemented in Task2, Task3,...?)

* push to main, instruct e.g. Nico to work on it
  * e.g. improving accuracy to 1e-4 (is it isnt yet). doing the whole github actions test thingy?(or finding another way) writing more robust tests.&#x20;

*

* fix GitHub actions test

  * **TODO**

    * more tests (hard asserts?c - e.g. google test (gtest) for unit tests?)

      * all 5 scenarios (driven cavity, varying Re, etc.)

        * scenario1: compare against given ground truth
        * scenario 2-5: compare against values computed by **Abschlussprojekt\_official\_WS21**
    * test what happens when uploading to the cluster. then decide for testing do i use cluster or github actions?

##
