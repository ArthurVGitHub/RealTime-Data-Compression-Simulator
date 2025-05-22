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
#        delta_lzw.h
#        Sprintz/sprintz.cpp
#        Sprintz/sprintz.h
#        sprintz_compressor.cpp
#        sprintz_compressor.h
#        Sprintz/sprintz_format.cpp
#        Sprintz/sprintz_format.h
#        Sprintz/sprintz_delta.cpp
#        Sprintz/sprintz_delta.h
#        Sprintz/sprintz_xff.cpp
#        Sprintz/sprintz_xff.h
#        Sprintz/sprintz_util.h
#        Sprintz/sprintz_macros.h
#        Sprintz/sprintz_bitpack.h
#        Sprintz/sprintz_query.hpp
#        Sprintz/sprintz_traits.hpp
#        Sprintz/sprintz_delta_lowdim.cpp
#        Sprintz/sprintz_delta_rle.cpp
#        Sprintz/sprintz_delta_rle_query.cpp
#        Sprintz/sprintz_delta_rle_query.hpp
#        Sprintz/sprintz_xff_rle.cpp
#        Sprintz/sprintz_xff_rle_query.cpp
#        Sprintz/sprintz_xff_rle_query.hpp
#        Sprintz/sprintz_transpose.h
#        Sprintz/univariate_8b.cpp
#        Sprintz/univariate_8b.h
        sprintz.cpp
        sprintz.h
        lz77.cpp
        lz77.h
        lz4/lz4.c
        lz4/lz4.h
        lz4_compressor.cpp
        lz4_compresor.h
)
target_link_libraries(Thesis PRIVATE Qt6::Widgets)
#target_compile_options(Thesis PRIVATE -mavx2 -mbmi -mbmi2 -mlzcnt)
target_compile_options(Thesis PRIVATE -mavx2)
