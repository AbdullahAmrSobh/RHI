if(PROJECT_IS_TOP_LEVEL)
  set(
      CMAKE_INSTALL_INCLUDEDIR "include/RHI-${PROJECT_VERSION}"
      CACHE STRING ""
  )
  set_property(CACHE CMAKE_INSTALL_INCLUDEDIR PROPERTY TYPE PATH)
endif()

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# find_package(<package>) call for consumers to find this project
set(package RHI)

install(
    DIRECTORY
    include/
    "${PROJECT_BINARY_DIR}/export/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT RHI_Development
)

install(
    TARGETS RHI::RHI
    EXPORT RHITargets
    RUNTIME #
    COMPONENT RHI_Runtime
    LIBRARY #
    COMPONENT RHI_Runtime
    NAMELINK_COMPONENT RHI_Development
    ARCHIVE #
    COMPONENT RHI_Development
    INCLUDES #
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

write_basic_package_version_file(
    "${package}ConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
)

# Allow package maintainers to freely override the path for the configs
set(
    RHI_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/${package}"
    CACHE STRING "CMake package config location relative to the install prefix"
)
set_property(CACHE RHI_INSTALL_CMAKEDIR PROPERTY TYPE PATH)
mark_as_advanced(RHI_INSTALL_CMAKEDIR)

install(
    FILES cmake/install-config.cmake
    DESTINATION "${RHI_INSTALL_CMAKEDIR}"
    RENAME "${package}Config.cmake"
    COMPONENT RHI_Development
)

install(
    FILES "${PROJECT_BINARY_DIR}/${package}ConfigVersion.cmake"
    DESTINATION "${RHI_INSTALL_CMAKEDIR}"
    COMPONENT RHI_Development
)

install(
    EXPORT RHITargets
    NAMESPACE RHI::
    DESTINATION "${RHI_INSTALL_CMAKEDIR}"
    COMPONENT RHI_Development
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
