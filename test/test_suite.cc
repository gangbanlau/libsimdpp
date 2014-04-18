/*  Copyright (C) 2012  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#include "test_suite.h"
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <type_traits>
#include <typeinfo>
#include <cstdlib>

TestSuite::TestSuite(const char* name, const char* file) :
    name_(name),
    file_(file),
    curr_precision_ulp_(0)
{
    reset_seq();
}

TestSuite::Result& TestSuite::push(Type type, unsigned length, unsigned line)
{
    results_.emplace_back(type, length, size_for_type(type), line, seq_++,
                          curr_precision_ulp_);
    return results_.back();
}

std::size_t TestSuite::size_for_type(Type t)
{
    switch (t) {
    case TYPE_INT8:
    case TYPE_UINT8: return 1;
    case TYPE_INT16:
    case TYPE_UINT16: return 2;
    case TYPE_UINT32:
    case TYPE_INT32: return 4;
    case TYPE_UINT64:
    case TYPE_INT64: return 8;
    case TYPE_FLOAT32: return 4;
    case TYPE_FLOAT64: return 8;
    default: std::abort();
    }
}

unsigned TestSuite::precision_for_result(const Result& res)
{
    switch (res.type) {
    case TestSuite::TYPE_FLOAT32:
    case TestSuite::TYPE_FLOAT64:
        return res.prec_ulp;
    default:
        return 0;
    }
}

template<class T> struct fix_char_type { using type = T; };
template<> struct fix_char_type<uint8_t> { using type = int; };
template<> struct fix_char_type<int8_t> { using type = int; };

template<class T>
void fmt_hex(std::ostream& err, unsigned num_elems, unsigned width,
             const char* prefix, const T* p)
{
    static_assert(std::is_unsigned<T>::value, "T must be unsigned");
    err << prefix << "[ " << std::hex << std::setfill('0');
    err.precision(width);
    for (unsigned i = 0; i < num_elems; i++, p++) {
        err << std::setw(width*2) << uint64_t(*p);
        if (i != num_elems - 1) {
            err << " ; ";
        }
    }
    err << " ]\n";
    err << std::dec << std::setfill(' ');
}

template<class T>
void fmt_num(std::ostream& err, unsigned num_elems, unsigned precision,
             const char* prefix, const T* p)
{
    err << prefix << "[ ";
    err.precision(precision);
    for (unsigned i = 0; i < num_elems; i++, p++) {
        err << typename fix_char_type<T>::type(*p);
        if (i != num_elems - 1) {
            err << " ; ";
        }
    }
    err << " ]\n";
    err << std::dec;
}

template<class T>
void fmt_bin(std::ostream& err, unsigned num_elems, const char* prefix, const T* p)
{
    err << prefix << "[ ";
    for (unsigned i = 0; i < num_elems; i++, p++) {
        T pl = *p;
        uint64_t pi;
        std::memcpy(&pi, &pl, sizeof(T));
        unsigned bits = sizeof(T)*8;
        for (unsigned j = 0; j < bits; j++) {
            err << ((pi & (1 << j)) ? '1' : '0');
        }
        if (i != num_elems - 1) {
            err << " ; ";
        }
    }
    err << " ]\n";
    err << std::dec;
}

template<class T>
bool cmpeq_arrays(const T* a, const T* b, unsigned num_elems, unsigned prec)
{
    for (unsigned i = 0; i < num_elems; i++) {
        // we need to be extra-precise here. nextafter works won't introduce
        // any rounding errors
        T ia = *a++;
        T ib = *b++;
        for (unsigned i = 0; i < prec; i++) {
            ia = std::nextafter(ia, ib);
        }
        if (ia != ib) {
            return false;
        }
    }
    return true;
}

bool test_equal(const TestSuite& a, const char* a_arch,
                const TestSuite& b, const char* b_arch,
                std::ostream& err)

{
    auto fmt_separator = [&]()
    {
        err << "--------------------------------------------------------------\n";
    };
    auto fmt_arch = [&]()
    {
        err << "  For architectures: " << a_arch << " and " << b_arch << " :\n";
    };
    auto fmt_file = [&]()
    {
        fmt_arch();
        err << "  In file \"" << a.file_ << "\" :\n";
    };
    auto fmt_file_line = [&](unsigned line)
    {
        fmt_arch();
        err << "  In file \"" << a.file_ << "\" at line " << line << " : \n";
    };
    auto fmt_test_case = [&]()
    {
        err << "  In test case \"" << a.name_ << "\" :\n";
    };
    auto fmt_seq = [&](unsigned num)
    {
        err << "  Sequence number: " << num << "\n"; // start from one
    };
    auto fmt_prec = [&](unsigned prec)
    {
        if (prec > 0) {
            err << "  Precision: " << prec << "ULP\n";
        }
    };

    auto type_str = [&](unsigned type) -> const char*
    {
        switch (type) {
        case TestSuite::TYPE_UINT8: return "uint86";
        case TestSuite::TYPE_INT8: return "int86";
        case TestSuite::TYPE_UINT16: return "uint16";
        case TestSuite::TYPE_INT16: return "int16";
        case TestSuite::TYPE_UINT32: return "uint32";
        case TestSuite::TYPE_INT32: return "int32";
        case TestSuite::TYPE_UINT64: return "uint64";
        case TestSuite::TYPE_INT64: return "int64";
        case TestSuite::TYPE_FLOAT32: return "float32";
        case TestSuite::TYPE_FLOAT64: return "float64";
        default: return "UNDEFINED";
        }
    };

    auto fmt_vector = [&](const TestSuite::Result& r, const char* prefix) -> void
    {
        if (r.length == 1) {
            switch (r.type) {
            case TestSuite::TYPE_INT8:   fmt_bin(err, r.length, prefix, (const int8_t*)r.d());
            case TestSuite::TYPE_UINT8:  fmt_bin(err, r.length, prefix, (const uint8_t*)r.d());
            case TestSuite::TYPE_INT16:  fmt_bin(err, r.length, prefix, (const int16_t*)r.d());
            case TestSuite::TYPE_UINT16: fmt_bin(err, r.length, prefix, (const uint16_t*)r.d());
            case TestSuite::TYPE_INT32:  fmt_bin(err, r.length, prefix, (const int32_t*)r.d());
            case TestSuite::TYPE_UINT32: fmt_bin(err, r.length, prefix, (const uint32_t*)r.d());
            case TestSuite::TYPE_INT64:  fmt_bin(err, r.length, prefix, (const int64_t*)r.d());
            case TestSuite::TYPE_UINT64: fmt_bin(err, r.length, prefix, (const uint64_t*)r.d());
            case TestSuite::TYPE_FLOAT32: fmt_bin(err, r.length, prefix, (const float*)r.d());
            case TestSuite::TYPE_FLOAT64:  fmt_bin(err, r.length, prefix, (const double*)r.d());
            }
        } else {
            switch (r.type) {
            case TestSuite::TYPE_UINT8:
                fmt_hex(err, r.length, 1, prefix, (const uint8_t*)r.d());
                fmt_num(err, r.length, 4, prefix, (const int8_t*)r.d());
                break;
            case TestSuite::TYPE_INT8:
                fmt_hex(err, r.length, 1, prefix, (const uint8_t*)r.d());
                fmt_num(err, r.length, 4, prefix, (const uint8_t*)r.d());
                break;
            case TestSuite::TYPE_UINT16:
                fmt_hex(err, r.length, 2, prefix, (const uint16_t*)r.d());
                fmt_num(err, r.length, 6, prefix, (const int16_t*)r.d());
                break;
            case TestSuite::TYPE_INT16:
                fmt_hex(err, r.length, 2, prefix, (const uint16_t*)r.d());
                fmt_num(err, r.length, 6, prefix, (const uint16_t*)r.d());
                break;
            case TestSuite::TYPE_UINT32:
                fmt_hex(err, r.length, 4, prefix, (const uint32_t*)r.d());
                fmt_num(err, r.length, 11, prefix, (const int32_t*)r.d());
                break;
            case TestSuite::TYPE_INT32:
                fmt_hex(err, r.length, 4, prefix, (const uint32_t*)r.d());
                fmt_num(err, r.length, 11, prefix, (const uint32_t*)r.d());
                break;
            case TestSuite::TYPE_UINT64:
                fmt_hex(err, r.length, 8, prefix, (const uint64_t*)r.d());
                fmt_num(err, r.length, 20, prefix, (const int64_t*)r.d());
                break;
            case TestSuite::TYPE_INT64:
                fmt_hex(err, r.length, 8, prefix, (const uint64_t*)r.d());
                fmt_num(err, r.length, 20, prefix, (const uint64_t*)r.d());
                break;
            case TestSuite::TYPE_FLOAT32:
                fmt_hex(err, r.length, 4, prefix, (const uint32_t*)r.d());
                fmt_num(err, r.length, 7, prefix, (const float*)r.d());
                break;
            case TestSuite::TYPE_FLOAT64:
                fmt_hex(err, r.length, 8, prefix, (const uint64_t*)r.d());
                fmt_num(err, r.length, 17, prefix, (const double*)r.d());
                break;
            }
        }
    };

    auto cmpeq_result = [](const TestSuite::Result& ia, const TestSuite::Result& ib,
                           unsigned prec) -> bool
    {
        if (std::memcmp(ia.d(), ib.d(), TestSuite::size_for_type(ia.type)) == 0) {
            return true;
        }

        if (prec == 0) {
            return false;
        }

        switch (ia.type) {
        case TestSuite::TYPE_FLOAT32:
            return cmpeq_arrays((const float*)ia.d(), (const float*)ib.d(), ia.length, prec);
        case TestSuite::TYPE_FLOAT64:
            return cmpeq_arrays((const double*)ia.d(), (const double*)ib.d(), ia.length, prec);
        default:
            return false;
        }
    };

    // Handle fatal errors first
    if (std::strcmp(a.name_, b.name_) != 0) {
        fmt_separator();
        fmt_file();
        err << "FATAL: Test case names do not match: \""
            << a.name_ << "\" and \""  << b.name_ << "\"\n";
        fmt_separator();
        return false;
    }

    if (a.results_.size() != b.results_.size()) {
        if (a.results_.size() == 0 || b.results_.size() == 0) {
            return true; // Ignore empty sections
        }
        fmt_separator();
        fmt_file();
        fmt_test_case();
        err << "FATAL: The lengths of the result vectors does not match: "
            << a.results_.size() << "/" << b.results_.size() << "\n";
        fmt_separator();
        return false;
    }

    bool ok = true;
    // Compare results
    for (unsigned i = 0; i < a.results_.size(); i++) {
        const auto& ia = a.results_[i];
        const auto& ib = b.results_[i];

        if (ia.line != ib.line) {
            fmt_separator();
            fmt_file();
            fmt_test_case();
            err << "FATAL: Line numbers do not match for items with the same "
                << "sequence number: id: " << i
                << " line_A: " << ia.line << " line_B: " << ib.line << "\n";
            fmt_separator();
            return false;
        }

        if (ia.type != ib.type) {
            fmt_separator();
            fmt_file_line(ia.line);
            fmt_test_case();
            err << "FATAL: Types do not match for items with the same "
                << "sequence number: id: " << i
                << " type_A: " << type_str(ia.type)
                << " line_B: " << type_str(ib.type) << "\n";
            fmt_separator();
            return false;
        }

        unsigned prec = std::max(TestSuite::precision_for_result(ia),
                                 TestSuite::precision_for_result(ib));

        if (!cmpeq_result(ia, ib, prec)) {
            fmt_separator();
            fmt_file_line(ia.line);
            fmt_test_case();
            fmt_seq(ia.seq);
            err << "ERROR: Vectors not equal: \n";
            fmt_vector(ia, "A : ");
            fmt_vector(ib, "B : ");
            fmt_prec(prec);
            fmt_separator();
            ok = false;
        }
    }
    return ok;
}
