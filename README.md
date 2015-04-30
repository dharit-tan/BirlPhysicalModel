# BirlPhysicalModel

Thesis project by Dharit Tantiviramanond

Princeton University

Advisor: Prof. Jeffrey Snyder

Short description:
This project implements a physical model of an acoustic tube with
* Continuous tonehole control - Users can "half-close" toneholes instead of being limited to fully closed or fully open.
* User-defined physical parameters - Physical specifications of the model (length of bore, size of toneholes) can be controlled by the user.
* Tuning system - A tuning model that helps users choose suitable physical parameters based on a desired tuning system and fundamental frequency.

You can read the report I wrote for my thesis (included in this repo) for more information. To get up and running quickly,
install the ChuGin and check out some of the example ChucK patches (`AxiomBirl.ck` is a good place to start).


Dependencies:
* Download and install ChucK and MiniAudicle: http://chuck.cs.princeton.edu/release/
* Download the ChuGin source code: https://github.com/spencersalazar/chugins

Installation:
* Move this repo to inside of the `chugins/` directory.
* Run `make osx && sudo make install` to install the ChuGin.
* You should now be able to use the `BirlPhysicalModel` object in ChucK patches.

Some example ChuCk patches included in this repo:
* AxiomBirl.ck: A setup I used during testing that connects a model to my M-Audio Axiom 49 controller.
* BirlFinal.ck: The default setup used to connect with the Birl controller.
* Birl2Octaves.ck: A test setup that adds a 2nd octave by running two instances of `BirlPhysicalModel` at two different octaves and adding an interpolating mix control between the two.

Helpful notes:
I had to perform these steps in order to successfully set up a new ChuGin:
* Run `chuginate/chuginate [ChuGin name]` from inside of `chugins/`.
* Copy the STK directory to the newly created directory. (If you want, you may remove any .h and .cpp files from `include` and `src` your project does not utilize if you need to save space. `Stk.cpp` and `Stk.h` are usually required. If you're using filters you'll also need `Filters.h`.)
* Add this line to the beginning of `makefile`: `FLAGS+=-Istk-[version]/include`. For example, `FLAGS+=-Istk-4.5.0/include`.
* Change the line in `makefile.osx` that reads `FLAGS=-D__MACOSX_CORE__ -I$(CK_SRC_PATH) $(ARCHOPTS)` to be `FLAGS+=-D__MACOSX_CORE__ -I$(CK_SRC_PATH) $(ARCHOPTS)` (add the plus sign).
* Add the names of all `.cpp` files that your project needs to the variable `CXX_MODULES=Test.cpp`. For example, `CXX_MODULES=Test.cpp stk-4.5.0/src/Stk.cpp stk-4.5.0/src/BiQuad.cpp`. Notice that there is no `Filters.cpp`.
* There is a compile-time error caused by a conflict of definitions of `TWO_PI`. An easy way to fix this is to change the name of all occurrences of the `TWO_PI` variables to something like `TWOPI` instead. If you have XCode, you can open up `iOS/STK.xcodeproj` and use the replace functionality of XCode.
* You should now be able to include STK `.h` files in your ChuGin and run `make osx && sudo make install` to build and install your ChuGin.
