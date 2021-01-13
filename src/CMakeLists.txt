cmake_minimum_required (VERSION 3.1)

file(GLOB BROTLI_SOURCE
  "../ext/brotli/enc/*.c"  
  "../ext/brotli/dec/*.c"
  "../ext/brotli/common/*.c"
)
 
file(GLOB ZSTD_SOURCE
  "../ext/zstd/common/*.c"
  "../ext/zstd/compress/*.c"
  "../ext/zstd/decompress/*.c"
  "../ext/zstd/dictBuilder/*.c"
)

set (IMGUI_SOURCE 
  "../ext/imgui/imgui.cpp"
  "../ext/imgui/imgui_demo.cpp"
  "../ext/imgui/imgui_draw.cpp"
  "../ext/imgui/imgui_impl_dx11.cpp"
  "../ext/imgui/imgui_impl_win32.cpp"
  "../ext/imgui/imgui_widgets.cpp"
)

set (DX11_LINKS
  d3d11.lib
  d3dcompiler.lib
  dxgi.lib
  imm32.lib  
  Shcore.lib
)

foreach (subproject downloader)
  add_executable(${subproject} ${subproject}/${subproject}.cpp ${ZSTD_SOURCE})
  target_compile_features(${subproject} INTERFACE cxx_std_11)
endforeach() 

foreach (subproject default)
  add_executable(${subproject} ${subproject}/${subproject}.cpp ${IMGUI_SOURCE} ${ZSTD_SOURCE})
  target_compile_features(${subproject} INTERFACE cxx_std_17)
endforeach() 
target_link_libraries(default ${DX11_LINKS})

configure_file("../extra/Exo2-Medium.ttf" "${CMAKE_CURRENT_BINARY_DIR}/extra/Exo2-Medium.ttf" COPYONLY)

