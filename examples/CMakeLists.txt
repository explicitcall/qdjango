set(QT_DONT_USE_QTGUI 1)
set(QT_USE_QTNETWORK 1)
set(QT_USE_QTSCRIPT 1)
set(QT_USE_QTSQL 1)
include(${QT_USE_FILE})

if(NOT QDJANGO_STATIC)
  add_executable(qdjango-console console.cpp)
  target_link_libraries(qdjango-console qdjango-script)
#  add_subdirectory(script)
endif()

add_executable(qdjango-redirect redirect.cpp)
target_link_libraries(qdjango-redirect qdjango-http)
