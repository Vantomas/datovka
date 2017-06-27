
QT += core network
QT += gui svg widgets

TEMPLATE = app
APP_NAME = ds_test_app
VERSION = 0.0.1

REQUIRED_MAJOR = 5
REQUIRED_MINOR = 2

lessThan(QT_MAJOR_VERSION, $${REQUIRED_MAJOR}) {
	error(Qt version $${REQUIRED_MAJOR}.$${REQUIRED_MINOR} is required.)
}

top_srcdir = ../..

DEFINES += \
	DEBUG=1 \
	VERSION=\\\"$${VERSION}\\\"

QMAKE_CXXFLAGS = \
	-g -O0 -std=c++11 \
	-Wall -Wextra -pedantic \
	-Wdate-time -Wformat -Werror=format-security

INCLUDEPATH += \
	$${top_srcdir}

SOURCES += \
	$${top_srcdir}/src/document_service/conversion.cpp \
	$${top_srcdir}/src/document_service/io/document_service_connection.cpp \
	$${top_srcdir}/src/document_service/json/entry_error.cpp \
	$${top_srcdir}/src/document_service/json/helper.cpp \
	$${top_srcdir}/src/document_service/json/service_info.cpp \
	$${top_srcdir}/src/document_service/json/stored_files.cpp \
	$${top_srcdir}/src/document_service/json/upload_file.cpp \
	$${top_srcdir}/src/document_service/json/upload_hierarchy.cpp \
	$${top_srcdir}/src/document_service/models/upload_hierarchy_model.cpp \
	$${top_srcdir}/src/document_service/models/upload_hierarchy_proxy_model.cpp \
	$${top_srcdir}/tests/document_service_app/gui/app.cpp \
	$${top_srcdir}/tests/document_service_app/gui/dialogue_service_info.cpp \
	$${top_srcdir}/tests/document_service_app/gui/dialogue_stored_files.cpp \
	main.cpp

HEADERS += \
	$${top_srcdir}/src/document_service/conversion.h \
	$${top_srcdir}/src/document_service/io/document_service_connection.h \
	$${top_srcdir}/src/document_service/json/entry_error.h \
	$${top_srcdir}/src/document_service/json/helper.h \
	$${top_srcdir}/src/document_service/json/service_info.h \
	$${top_srcdir}/src/document_service/json/stored_files.h \
	$${top_srcdir}/src/document_service/json/upload_file.h \
	$${top_srcdir}/src/document_service/json/upload_hierarchy.h \
	$${top_srcdir}/src/document_service/models/upload_hierarchy_model.h \
	$${top_srcdir}/src/document_service/models/upload_hierarchy_proxy_model.h \
	$${top_srcdir}/tests/document_service_app/gui/app.h \
	$${top_srcdir}/tests/document_service_app/gui/dialogue_service_info.h \
	$${top_srcdir}/tests/document_service_app/gui/dialogue_stored_files.h

FORMS += \
	$${top_srcdir}/tests/document_service_app/ui/app.ui \
	$${top_srcdir}/tests/document_service_app/ui/dialogue_service_info.ui \
	$${top_srcdir}/tests/document_service_app/ui/dialogue_stored_files.ui

RESOURCES +=
TRANSLATIONS +=
OTHER_FILES +=
