cmake_minimum_required(VERSION 3.10)
project(Thesis)
set(CMAKE_CXX_STANDARD 20)

# Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
find_package(Qt6 COMPONENTS Widgets REQUIRED)

add_executable(
        Thesis
        main.cpp
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
        huffman.cpp
        huffman.h
        lzw.cpp
        lzw.h
        rle.cpp
        rle.h
        delta_rle.cpp
        delta_rle.h
        delta_huffman.cpp
        delta_huffman.h
        delta_lzw.cpp
        delta_lzw.h
)
target_link_libraries(Thesis PRIVATE Qt6::Widgets)
