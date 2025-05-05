# CMakeList.txt: progetto CMake per HCI. Includere l'origine e definire
# qui la logica specifica del progetto.
#
cmake_minimum_required (VERSION 3.8)

project ("HCI")

# Aggiungere l'origine all'eseguibile di questo progetto.
add_executable (HCI "HCI.cpp" "HCI.h")

# TODO: Aggiungere i test e, se necessario, installare le destinazioni.
set(OpenCV_DIR "${CMAKE_SOURCE_DIR}/librerie/lib/install/opencv/include")
set(OpenCV_DIR "${CMAKE_SOURCE_DIR}/librerie/lib/install/opencv/x64/vc16/lib")
find_package(OpenCV REQUIRED)
target_link_libraries(HCI ${OpenCV_LIBS})



