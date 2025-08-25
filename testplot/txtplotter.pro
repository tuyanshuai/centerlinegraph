QT += core widgets network 
CONFIG += c++17 qt 
TARGET = txtplotter 
TEMPLATE = app 

SOURCES += main.cpp deepseek_dialog.cpp mainwindow.cpp plotwidget_new.cpp
HEADERS += deepseek_dialog.h mainwindow.h plotwidget_new.h

# win32:RC_ICONS = app.ico 

# Chinese support 
CODECFORSRC = UTF-8 
CODECFORTR = UTF-8