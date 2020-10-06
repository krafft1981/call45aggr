#pragma once

class BDb;

class BDbTransaction {
protected:
    BDb * db;
    bool transaction;
public:
    BDbTransaction(BDb * db, bool start = true);
    ~BDbTransaction();
    void begin();
    void commit();
    void rollback();
};
