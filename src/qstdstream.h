#ifndef QSTDSTREAM_H
#define QSTDSTREAM_H

#include <ostream>

#include <QByteArray>

namespace {

inline std::ostream& operator << (std::ostream& s, const QByteArray& a) {
    return s.write(a.constData(), a.size());
}

inline QByteArray qt(const std::string &s) {
    return QByteArray(s.c_str(), s.size());
}

inline std::string str(const QByteArray &s) {
    return std::string(s.constData(), s.size());
}

}

#endif // QSTDSTREAM_H
