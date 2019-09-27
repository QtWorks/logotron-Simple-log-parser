#include "Appearance.h"

namespace Appearance {

QColor colorError() {
    static QColor c(255, 0, 0, 55);
    return  c;
}

QColor colorWarning() {
    static QColor c(255, 153, 0, 55);
    return c;
}

QColor colorDebug() {
    static QColor c(0, 0, 0, 35);
    return c;
}

} // namespace Appearance
