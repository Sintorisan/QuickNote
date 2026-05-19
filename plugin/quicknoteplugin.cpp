#include "quicknotestore.h"

#include <QQmlExtensionPlugin>
#include <qqml.h>

class QuickNotePlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    void registerTypes(const char *uri) override
    {
        qmlRegisterType<QuickNoteStore>(uri, 1, 0, "QuickNoteStore");
    }
};

#include "quicknoteplugin.moc"
