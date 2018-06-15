#include <QString>
#include <QtTest>
#include <cmath>
#include <QDebug>
#include "helpers.h"

const double EPSILON = 0.001;

class HelpersTest : public QObject
{
    Q_OBJECT

public:
    HelpersTest();

private Q_SLOTS:
    void testRadToDeg();
    void testClip();
};

HelpersTest::HelpersTest()
{
}

/* Always respect QCOMPARE parameter semantics
   The first parameter to QCOMPARE should always be the actual value produced by the code-under-test, while the second parameter should always be the expected value. When the values don't match, QCOMPARE prints them with the labels "Actual" and "Expected". If the parameter order is swapped, debugging a failing test can be confusing.
*/
void HelpersTest::testRadToDeg()
{
    QVERIFY(fabs(Helpers::radToDeg(1) - 57.2958) <= EPSILON);
    QVERIFY(fabs(Helpers::radToDeg(0) - 0) <= EPSILON);
    QVERIFY(fabs(Helpers::radToDeg(-1) - (-57.2958)) <= EPSILON);
}

void HelpersTest::testClip()
{
    QCOMPARE(Helpers::clip(25.0, 20.0), 20.0);
    QCOMPARE(Helpers::clip(-25.0, 20.0), -20.0);
}

QTEST_APPLESS_MAIN(HelpersTest)

#include "tst_helperstest.moc"
