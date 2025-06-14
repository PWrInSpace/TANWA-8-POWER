# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/Matteo/esp/v5.2.1/esp-idf/components/bootloader/subproject"
  "C:/Users/Matteo/Desktop/PWR_IN_SPACE/TANWA+8/TANWA-8-POWER/bootloader"
  "C:/Users/Matteo/Desktop/PWR_IN_SPACE/TANWA+8/TANWA-8-POWER/bootloader-prefix"
  "C:/Users/Matteo/Desktop/PWR_IN_SPACE/TANWA+8/TANWA-8-POWER/bootloader-prefix/tmp"
  "C:/Users/Matteo/Desktop/PWR_IN_SPACE/TANWA+8/TANWA-8-POWER/bootloader-prefix/src/bootloader-stamp"
  "C:/Users/Matteo/Desktop/PWR_IN_SPACE/TANWA+8/TANWA-8-POWER/bootloader-prefix/src"
  "C:/Users/Matteo/Desktop/PWR_IN_SPACE/TANWA+8/TANWA-8-POWER/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/Matteo/Desktop/PWR_IN_SPACE/TANWA+8/TANWA-8-POWER/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/Matteo/Desktop/PWR_IN_SPACE/TANWA+8/TANWA-8-POWER/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
