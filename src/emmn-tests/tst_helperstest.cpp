#include <QString>
#include <QtTest>

class HelpersTest : public QObject
{
    Q_OBJECT

public:
    HelpersTest();

private Q_SLOTS:
    void testCase1();
};

HelpersTest::HelpersTest()
{
}

void HelpersTest::testCase1()
{
    QVERIFY2(true, "Failure");
}

QTEST_APPLESS_MAIN(HelpersTest)

#include "tst_helperstest.moc"
