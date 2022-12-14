#plugin build file
cmake_minimum_required(VERSION 3.5.0)

#include common rules
include(../PluginRules.cmake)

#add sources, not including OpenEphysLib.cpp
add_sources(${PLUGIN_NAME}
	rhythm-api/okFrontPanelDLL.h
	rhythm-api/rhd2000datablock.cpp
	rhythm-api/rhd2000datablock.h
	rhythm-api/rhd2000evalboard.cpp
	rhythm-api/rhd2000evalboard.h
	rhythm-api/rhd2000registers.cpp
	rhythm-api/rhd2000registers.h
	UI/ChannelComponent.h
	UI/ChannelComponent.cpp
	UI/ChannelList.h
	UI/ChannelList.cpp
	UI/ChannelCanvas.h
	UI/ChannelCanvas.cpp
	DeviceThread.cpp
	DeviceThread.h
	DeviceEditor.cpp
	DeviceEditor.h
	IntanUSBInterface.h
	IntanRecordController.h
	AcquisitionBoard.h
	AcqBoardOutput.h
	AcqBoardOutput.cpp
	AcqBoardOutputEditor.h
	AcqBoardOutputEditor.cpp
	Headstage.h
	Headstage.cpp
	ImpedanceMeter.h
	ImpedanceMeter.cpp
	)

if (MSVC)
	target_link_libraries(${PLUGIN_NAME} "${DLLDIR}/okFrontPanel.lib")
elseif(LINUX)
	target_link_libraries(${PLUGIN_NAME} "${DLLDIR}/libokFrontPanel.so")
elseif(APPLE)
	target_link_libraries(${PLUGIN_NAME} "${DLLDIR}/libokFrontPanel.dylib")
	set_source_files_properties(${DLLDIR}/libokFrontPanel.dylib PROPERTIES MACOSX_PACKAGE_LOCATION Frameworks)
	target_sources(${PLUGIN_NAME} PRIVATE ${DLLDIR}/libokFrontPanel.dylib)
endif()

#optional: create IDE groups
plugin_create_filters()
