/****************************************************************************
** Meta object code from reading C++ file 'udp2midithread.h'
**
** Created: Tue Sep 5 23:22:59 2006
**      by: The Qt Meta Object Compiler version 59 (Qt 4.1.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "udp2midithread.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'udp2midithread.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.1.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_Udp2MidiThread[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets

       0        // eod
};

static const char qt_meta_stringdata_Udp2MidiThread[] = {
    "Udp2MidiThread\0"
};

const QMetaObject Udp2MidiThread::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_Udp2MidiThread,
      qt_meta_data_Udp2MidiThread, 0 }
};

const QMetaObject *Udp2MidiThread::metaObject() const
{
    return &staticMetaObject;
}

void *Udp2MidiThread::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Udp2MidiThread))
	return static_cast<void*>(const_cast<Udp2MidiThread*>(this));
    return QThread::qt_metacast(_clname);
}

int Udp2MidiThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
