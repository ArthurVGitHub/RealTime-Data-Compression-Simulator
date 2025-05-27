cmake_minimum_required(VERSION 3.10)
project(Thesis)
set(CMAKE_CXX_STANDARD 20)

# Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include_directories(${CMAKE_SOURCE_DIR}/lz4)
include_directories(${CMAKE_SOURCE_DIR}/zstandard)
link_directories(${CMAKE_SOURCE_DIR}/zstandard)
include_directories(${CMAKE_SOURCE_DIR}/lzma)
include_directories(${CMAKE_SOURCE_DIR}/lzo/lzo-2.10/include)
include_directories(${CMAKE_SOURCE_DIR}/snappy/build)
include_directories(${CMAKE_SOURCE_DIR}/snappy)
include_directories(${CMAKE_SOURCE_DIR}/lz77/yalz77-master)

#find_package(Qt6 COMPONENTS Widgets REQUIRED)
find_package(Qt6 COMPONENTS Widgets PrintSupport REQUIRED)
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
        sprintz.cpp
        sprintz.h
        lz77_compressor.cpp
        lz77_compressor.h
        lz4/lz4.c
        lz4/lz4.h
        lz4_compressor.cpp
        lz4_compresor.h
        graphLib/qcustomplot.cpp
        graphLib/qcustomplot.h
        SensorPlotDialog.cpp
        SensorPlotDialog.h
        zstd_compressor.cpp
        zstd_compressor.h
        lzma/C/LzmaEnc.c
        lzma/C/LzmaDec.c
        lzma/C/LzFind.c
        lzma/C/LzFindMt.c
        lzma/C/LzFindOpt.c
        lzma/C/CpuArch.c
        lzma/C/Threads.c
        lzma_compressor.cpp
        lzma_compressor.h
        lzo/lzo-2.10/src/lzo1x_1.c
        lzo/lzo-2.10/src/lzo_util.c
        lzo/lzo-2.10/src/lzo_init.c
        lzo/lzo-2.10/src/lzo1x_d1.c
        lzo_compressor.cpp
        lzo_compressor.h
        snappy_compressor.cpp
        snappy_compressor.h
)
#target_link_libraries(Thesis PRIVATE Qt6::Widgets)
#target_link_libraries(Thesis PRIVATE Qt6::Widgets Qt6::PrintSupport)
target_link_libraries(Thesis PRIVATE Qt6::Widgets Qt6::PrintSupport zstd)
#target_link_libraries(Thesis PRIVATE snappy)
target_link_libraries(Thesis PRIVATE ${CMAKE_SOURCE_DIR}/snappy/build/libsnappy.a)
#target_compile_options(Thesis PRIVATE -mavx2 -mbmi -mbmi2 -mlzcnt)
target_compile_options(Thesis PRIVATE -mavx2)


