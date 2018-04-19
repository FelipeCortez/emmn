#include <QtTest>
#include <QCoreApplication>

class TestFirst : public QObject
{
    Q_OBJECT

public:
    TestFirst();
    ~TestFirst();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void test_case1();

};

TestFirst::TestFirst()
{

}

TestFirst::~TestFirst()
{

}

void TestFirst::initTestCase()
{

}

void TestFirst::cleanupTestCase()
{

}

void TestFirst::test_case1()
{

}

QTEST_MAIN(TestFirst)

#include "tst_testfirst.moc"
