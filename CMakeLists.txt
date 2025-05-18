cmake_minimum_required(VERSION 3.10)
project(Thesis)

set(CMAKE_CXX_STANDARD 20)

# Enable Qt's automatic code generation
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6 Widgets
find_package(Qt6 COMPONENTS Widgets REQUIRED)

# Add your executable and all sources/headers
add_executable(Thesis
        main.cpp
        compressor.cpp
        compressor.h
        compare.cpp
        compare.h
        drh.cpp
        drh.h
        compressor_interface.h
        emodnet_extractor.h
        emodnet_extractor.cpp
        MainWindow.cpp
        MainWindow.h
        MainWindow.ui
        CompressorRunner.cpp
        CompressorRunner.h
        delta.cpp
        delta.h
        ResultsTableDialog.cpp
        ResultsTableDialog.h
        compressor_factory.cpp
        compressor_factory.h
        window_optimizer.cpp
        window_optimizer.h
)

# Link against Qt6 Widgets
target_link_libraries(Thesis PRIVATE Qt6::Widgets)
