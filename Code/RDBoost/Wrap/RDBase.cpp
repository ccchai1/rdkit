
// Copyright (c) 2004-2019 greg Landrum and Rational Discovery LLC
//
//   @@ All Rights Reserved @@
//  This file is part of the RDKit.
//  The contents are covered by the terms of the BSD license
//  which is included in the file license.txt, found at the root
//  of the RDKit source tree.
//
#include <RDBoost/python.h>
#include <iostream>
#include <fstream>
#include <RDBoost/Wrap.h>
#include <RDBoost/python_streambuf.h>
#include <RDGeneral/versions.h>
#include <RDGeneral/Invariant.h>
#include <cstdlib>

#include <RDGeneral/RDLog.h>
#if 0
#include <boost/log/functions.hpp>
#if defined(BOOST_HAS_THREADS)
#include <boost/log/extra/functions_ts.hpp>
#endif
#endif

namespace python = boost::python;
namespace logging = boost::logging;

std::string _version() { return "$Id$"; }

void EnableLog(std::string spec) { logging::enable_logs(spec); }
void DisableLog(std::string spec) { logging::disable_logs(spec); }
std::string LogStatus() { return logging::log_status(); }
void AttachFileToLog(std::string spec, std::string filename, int delay = 100) {
  (void)spec;
  (void)filename;
  (void)delay;
#if 0
#if defined(BOOST_HAS_THREADS)
  logging::manipulate_logs(spec)
    .add_appender(logging::ts_appender(logging::write_to_file(filename),
				       delay));
#else
  logging::manipulate_logs(spec)
    .add_appender(logging::write_to_file(filename));

#endif
#endif
}
void LogMessage(std::string spec, std::string msg) {
#if 0
  logging::logger theLog(spec);
  if(theLog.is_enabled(logging::level::default_)){
    *(theLog.stream().stream()) << msg;
  }
#else
  //  FIX: get this more general
  std::shared_ptr<boost::logging::rdLogger> dest = nullptr;
  if (spec == "rdApp.error") {
    dest = rdErrorLog;
  } else if (spec == "rdApp.warning") {
    dest = rdWarningLog;
  } else if (spec == "rdApp.info") {
    dest = rdInfoLog;
  } else if (spec == "rdApp.debug") {
    dest = rdDebugLog;
  } else {
    dest = nullptr;
  }

  if (dest) {
    BOOST_LOG(dest) << msg;
  }
#endif
}

namespace {
struct python_streambuf_wrapper {
  typedef boost_adaptbx::python::streambuf wt;

  static void wrap() {
    using namespace boost::python;
    class_<wt, boost::noncopyable>("streambuf", no_init)
        .def(init<object&, std::size_t>(
            (arg("python_file_obj"), arg("buffer_size") = 0),
            "documentation")[with_custodian_and_ward_postcall<0, 2>()]);
  }
};

struct python_ostream_wrapper {
  typedef boost_adaptbx::python::ostream wt;

  static void wrap() {
    using namespace boost::python;
    class_<std::ostream, boost::noncopyable>("std_ostream", no_init);
    class_<wt, boost::noncopyable, bases<std::ostream>>("ostream", no_init)
        .def(init<object&, std::size_t>(
            (arg("python_file_obj"), arg("buffer_size") = 0)));
  }
};

void seedRNG(unsigned int seed) { std::srand(seed); }
}  // namespace

BOOST_PYTHON_MODULE(rdBase) {
  python::scope().attr("__doc__") =
      "Module containing basic definitions for wrapped C++ code\n"
      "\n";
  RDLog::InitLogs();
  RegisterVectorConverter<int>();
  RegisterVectorConverter<unsigned>();
  RegisterVectorConverter<double>();
  RegisterVectorConverter<std::string>(1);
  RegisterVectorConverter<std::vector<int>>();
  RegisterVectorConverter<std::vector<unsigned>>();
  RegisterVectorConverter<std::vector<double>>();

  RegisterListConverter<int>();
  RegisterListConverter<std::vector<int>>();
  RegisterListConverter<std::vector<unsigned int>>();

  python::register_exception_translator<IndexErrorException>(
      &translate_index_error);
  python::register_exception_translator<ValueErrorException>(
      &translate_value_error);
  python::register_exception_translator<KeyErrorException>(
      &translate_key_error);

#if INVARIANT_EXCEPTION_METHOD
  python::register_exception_translator<Invar::Invariant>(
      &translate_invariant_error);
#endif

  python::def("_version", _version,
              "Deprecated, use the constant rdkitVersion instead");

  python::scope().attr("rdkitVersion") = RDKit::rdkitVersion;
  python::scope().attr("boostVersion") = RDKit::boostVersion;
  python::scope().attr("rdkitBuild") = RDKit::rdkitBuild;

  python::def("EnableLog", EnableLog);
  python::def("DisableLog", DisableLog);
  python::def("LogStatus", LogStatus);

  python::def("AttachFileToLog", AttachFileToLog,
              "Causes the log to write to a file",
              (python::arg("spec"), python::arg("filename"),
               python::arg("delay") = 100));
  python::def("LogMessage", LogMessage);

  python::def("SeedRandomNumberGenerator", seedRNG,
              "Provides a seed to the standard C random number generator\n"
              "This does not affect pure Python code, but is relevant to some "
              "of the RDKit C++ components.",
              (python::arg("seed")));

  python_streambuf_wrapper::wrap();
  python_ostream_wrapper::wrap();

  python::class_<RDLog::BlockLogs, boost::noncopyable>(
      "BlockLogs",
      "Temporarily block logs from outputting while this instance is in "
      "scope.");
}
