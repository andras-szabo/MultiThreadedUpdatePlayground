:: Compiler flags:
:: /EHsc: enable standard C++ exception handling behaviour
:: /O2 "creates fast code"
:: /Ox "A subset of /O2 that doesn't include /GF or /Gy"
::		/GF: string pooling
::		/Gy: enables function-level linking
::
:: /Wall "All warnings, including warnings that are disabled by default"
:: /W0 - suppress warnings
:: /W1 - only severe warnings; default with cl compiler
:: /W2 - level 1 and 2 (significant)
:: /W3 - level 1 and 2 and 3 (production quality)
:: /W4 - level 1, 2, 3 and 4 (informational), that are not off by default
:: /std:c++17	
:: /std:c++20

cl /EHsc /W4 /std:c++20 /O2 ..\main.cpp
