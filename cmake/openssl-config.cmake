
OpenSSL_INCLUDE_DIRS

if(APPLE)
  find_library(Security Security REQUIRED)
  find_library(CoreFoundation CoreFoundation REQUIRED)
  target_link_libraries(${app} ${Security} ${CoreFoundation})
else()
endif()
