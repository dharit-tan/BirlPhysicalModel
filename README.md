# BirlPhysicalModel

Thesis project by Dharit Tantiviramanond

Princeton University

Advisor: Prof. Jeffrey Snyder

Dependencies:
* Download and install ChucK and MiniAudicle: http://chuck.cs.princeton.edu/release/
* Download the ChuGin source code: https://github.com/spencersalazar/chugins
* Download the STK source code: https://ccrma.stanford.edu/software/stk/download.html

I had to perform these steps in order to successfully set up a new ChuGin:
* Run `chuginate/chuginate [ChuGin name]` from inside of `chugins/`.
* Copy the STK directory to the newly created directory. (If you want, you may remove any .h and .cpp files from `include` and `src` your project does not utilize. `Stk.cpp` and `Stk.h` are usually required.)
* Add this line to the beginning of `makefile`: `FLAGS+=-Istk-[version]/include`. For example, `FLAGS+=-Istk-4.5.0/include`.
* Change the line in `makefile.osx` that reads `FLAGS=-D__MACOSX_CORE__ -I$(CK_SRC_PATH) $(ARCHOPTS)` to be `FLAGS+=-D__MACOSX_CORE__ -I$(CK_SRC_PATH) $(ARCHOPTS)` (add the plus sign).
