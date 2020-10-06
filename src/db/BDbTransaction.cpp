#include "BDbTransaction.h"
#include "BDb.h"

BDbTransaction::BDbTransaction(BDb * db, bool start) {
    this->db = db;
    transaction = false;
    if (start) {
        begin();
    }
}

BDbTransaction::~BDbTransaction() {
    if (transaction) {
        try {
            rollback();
        } catch (...) { }
    }
}

void BDbTransaction::begin() {
    db->exec("BEGIN");
    transaction = true;
}

void BDbTransaction::commit() {
    db->exec("COMMIT");
    transaction = false;
}

void BDbTransaction::rollback() {
    db->exec("ROLLBACK");
    transaction = false;
}
