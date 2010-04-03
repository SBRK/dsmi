echo "Copying frameworks"

mkdir DSMidiWifi.app/Contents/Frameworks
cp -R /Library/Frameworks/QtCore.framework DSMidiWifi.app/Contents/Frameworks
cp -R /Library/Frameworks/QtGui.framework DSMidiWifi.app/Contents/Frameworks
cp -R /Library/Frameworks/QtNetwork.framework DSMidiWifi.app/Contents/Frameworks

find DSMidiWifi.app -name "*_debug" | xargs rm
find DSMidiWifi.app -type d -name "Headers" | xargs rm -rf

echo "bending links"

install_name_tool -id @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore DSMidiWifi.app/Contents/Frameworks/QtCore.framework/Versions/4/QtCore
install_name_tool -id @executable_path/../Frameworks/QtGui.framework/Versions/4/QtGui DSMidiWifi.app/Contents/Frameworks/QtGui.framework/Versions/4/QtGui
install_name_tool -id @executable_path/../Frameworks/QtNetwork.framework/Versions/4/QtNetwork DSMidiWifi.app/Contents/Frameworks/QtNetwork.framework/Versions/4/QtNetwork

install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore DSMidiWifi.app/Contents/MacOs/DSMidiWifi
install_name_tool -change QtGui.framework/Versions/4/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/4/QtGui DSMidiWifi.app/Contents/MacOs/DSMidiWifi
install_name_tool -change QtNetwork.framework/Versions/4/QtNetwork @executable_path/../Frameworks/QtNetwork.framework/Versions/4/QtNetwork DSMidiWifi.app/Contents/MacOs/DSMidiWifi

install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore DSMidiWifi.app/Contents/Frameworks/QtGui.framework/Versions/4/QtGui
install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore DSMidiWifi.app/Contents/Frameworks/QtNetwork.framework/Versions/4/QtNetwork
