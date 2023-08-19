if(PROJECT_IS_TOP_LEVEL)
  set(
      CMAKE_INSTALL_INCLUDEDIR "include/RHI-${PROJECT_VERSION}"
      CACHE PATH ""
  )
endif()

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# find_package(<package>) call for consumers to find this project
set(package RHI)

install(
    DIRECTORY
    include/
    "${PROJECT_BINARY_DIR}/RHI/Export/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT RHI_Development
)

install(
    TARGETS RHI-RHI RHI-Vulkan
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
