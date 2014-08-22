#ifndef QSTDSTREAM_H
#define QSTDSTREAM_H

#include <ostream>

#include <QByteArray>

namespace {

inline std::ostream& operator << (std::ostream& s, const QByteArray& a) {
    return s.write(a.constData(), a.size());
}

}

#endif // QSTDSTREAM_H
