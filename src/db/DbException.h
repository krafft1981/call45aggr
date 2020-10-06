#pragma once

#include <stdexcept>
#include <string.h>
#include "BDb.h"
#include <systems/Exception.h>

using namespace std;

class DbException : public Exception {
public:

    DbException() {
    }

    DbException(PGconn * conn) {
        char error_message[1024];
        strncpy(error_message, PQerrorMessage(conn), sizeof (error_message) - 1);
        message = error_message;
    }

    DbException(PGconn * conn, const string &_trace) {
        char error_message[1024];
        strncpy(error_message, PQerrorMessage(conn), sizeof (error_message) - 1);
        message = error_message;
        addTrace(_trace);
    }

    DbException(const string &_what) : Exception(_what) {
    }

    DbException(const char * _what) : Exception(_what) {
    }

    DbException(const string &_what, const string &_trace) : Exception(_what, _trace) {
    }

    DbException(const char * _what, const char * _trace) : Exception(_what, _trace) {
    }
};
